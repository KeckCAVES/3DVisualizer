/***********************************************************************
ReifSeismicTomography - Class to visualize results of seismic
tomographic analyses in Mercator grid format.
Copyright (c) 2007 Oliver Kreylos

This file is part of the 3D Data Visualizer (Visualizer).

The 3D Data Visualizer is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The 3D Data Visualizer is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the 3D Data Visualizer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>

#include <Concrete/ReifSeismicTomography.h>

namespace Visualization {

namespace Concrete {

/**************************************
Methods of class ReifSeismicTomography:
**************************************/

ReifSeismicTomography::ReifSeismicTomography(void)
	:BaseModule("ReifSeismicTomography")
	{
	}

Visualization::Abstract::DataSet* ReifSeismicTomography::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Parse the module command line: */
	bool haveNumVertices=false;
	DS::Index numVertices;
	bool haveFileNames=false;
	const char* dataFileNames[2]={0,0};
	for(unsigned int i=0;i<args.size();++i)
		{
		if(args[i][0]=='-')
			{
			if(strcasecmp(args[i].c_str()+1,"size")==0) // Size is given in lat, long, depth order
				{
				unsigned int j;
				for(j=0;j<3&&i<args.size()-1;++j)
					{
					++i;
					numVertices[2-j]=atoi(args[i].c_str());
					}
				haveNumVertices=j==3;
				}
			}
		else if(dataFileNames[0]==0)
			dataFileNames[0]=args[i].c_str();
		else
			{
			dataFileNames[1]=args[i].c_str();
			haveFileNames=true;
			}
		}
	if(!haveNumVertices)
		Misc::throwStdErr("ReifSeismicTomography::load: Missing data set size");
	if(!haveFileNames)
		Misc::throwStdErr("ReifSeismicTomography::load: Missing data set file name");
	
	/* Open the P and S wave velocity files: */
	Misc::File pFile(dataFileNames[0],"rt");
	Misc::File sFile(dataFileNames[1],"rt");
	
	/* Data size is depth, longitude, latitude in C memory order (latitude varies fastest) */
	
	/* Create the data set: */
	EarthDataSet<DataSet>* result=new EarthDataSet<DataSet>(args);
	result->getSphericalCoordinateTransformer()->setDepth(true);
	result->getDs().setGrids(1);
	result->getDs().setGridData(0,numVertices+DS::Index(0,1,0)); // Make extra room to stitch at 0 meridian
	
	/* Set the data value's name: */
	result->getDataValue().setScalarVariableName(0,"P Velocity");
	result->getDataValue().setScalarVariableName(1,"S Velocity");
	
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	const double scaleFactor=1.0e-3;
	
	/* Read all grid points from both files in parallel: */
	DS::Array& vertices=result->getDs().getGrid(0).getVertices();
	DS::Index index;
	for(index[0]=0;index[0]<numVertices[0];++index[0])
		{
		for(index[1]=1;index[1]<=numVertices[1];++index[1])
			{
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				{
				/* Read the next grid point from both files: */
				char line[80];
				pFile.gets(line,sizeof(line));
				double plat,plng,pdepth;
				float pvalue;
				sscanf(line,"%lf %lf %lf %f",&plat,&plng,&pdepth,&pvalue);
				sFile.gets(line,sizeof(line));
				double slat,slng,sdepth;
				float svalue;
				sscanf(line,"%lf %lf %lf %f",&slat,&slng,&sdepth,&svalue);
				
				/* Check for grid consistency: */
				if(plat!=slat||plng!=slng||pdepth!=sdepth)
					Misc::throwStdErr("ReifSeismicTomography::load: Mismatching grid vertices in input files %s and %s",dataFileNames[0],dataFileNames[1]);
				
				/* Convert geoid coordinates to Cartesian coordinates: */
				DS::GridVertex& vertex=vertices(index);
				plat=Math::rad(plat);
				plng=Math::rad(plng);
				double s0=Math::sin(plat);
				double c0=Math::cos(plat);
				double r=(a*(1.0-f*Math::sqr(s0))-pdepth*1000.0)*scaleFactor;
				double xy=r*c0;
				double s1=Math::sin(plng);
				double c1=Math::cos(plng);
				vertex.pos[0]=float(xy*c1);
				vertex.pos[1]=float(xy*s1);
				vertex.pos[2]=float(r*s0);
				
				/* Store the velocity values: */
				vertex.value.components[0]=pvalue;
				vertex.value.components[1]=svalue;
				}
			}
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			vertices(index[0],0,index[2])=vertices(index[0],numVertices[1],index[2]);
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

Visualization::Abstract::DataSetRenderer* ReifSeismicTomography::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new EarthDataSetRenderer<DataSet,DataSetRenderer>(dataSet);
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::ReifSeismicTomography* module=new Visualization::Concrete::ReifSeismicTomography();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
