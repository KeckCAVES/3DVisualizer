/***********************************************************************
EarthTomographyGrid - Class to visualize results of seismic tomographic
analyses in Mercator grid format.
Copyright (c) 2005-2007 Oliver Kreylos

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
#include <vector>
#include <algorithm>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>

#include <Concrete/EarthTomographyGrid.h>

namespace Visualization {

namespace Concrete {

namespace {

/*****************
Helper structures:
*****************/

struct GridFile
	{
	/* Elements: */
	public:
	std::string fileName; // Name of grid file
	int depth; // Depth of file's grid values
	
	/* Constructors and destructors: */
	GridFile(const char* sFileName,int sDepth)
		:fileName(sFileName),depth(sDepth)
		{
		}
	
	/* Methods: */
	friend bool operator==(const GridFile& gf1,const GridFile& gf2)
		{
		return gf1.depth==gf2.depth;
		}
	friend bool operator<(const GridFile& gf1,const GridFile& gf2)
		{
		return gf1.depth<gf2.depth;
		}
	};

}

/************************************
Methods of class EarthTomographyGrid:
************************************/

EarthTomographyGrid::EarthTomographyGrid(void)
	:BaseModule("EarthTomographyGrid")
	{
	}

Visualization::Abstract::DataSet* EarthTomographyGrid::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Read all grid file names from the given directory: */
	std::vector<GridFile> gridFiles;
	DIR* gridFileDir=opendir(args[0].c_str());
	if(gridFileDir==0)
		Misc::throwStdErr("EarthTomographyGrid::load: Could not open grid file directory %s",args[0].c_str());
	struct dirent* dirEntry;
	while((dirEntry=readdir(gridFileDir))!=0)
		{
		/* Check if the directory entry is a grid file: */
		const char* extPtr=0;
		int depthState=0;
		int depth=-1;
		for(const char* dnPtr=dirEntry->d_name;*dnPtr!='\0';++dnPtr)
			{
			/* Parse the depth value encoded in the file name: */
			switch(depthState)
				{
				case 0: // Nothing read yet
					if(*dnPtr=='.')
						depthState=1;
					break;
				
				case 1: // Read "."
					if(isdigit(*dnPtr))
						{
						depth=int(*dnPtr-'0');
						depthState=2;
						}
					else if(*dnPtr=='.')
						depthState=1;
					else
						depthState=0;
					break;
				
				case 2: // Read ".[0-9]+"
					if(tolower(*dnPtr)=='k')
						depthState=3;
					else if(isdigit(*dnPtr))
						{
						depth=depth*10+int(*dnPtr-'0');
						depthState=2;
						}
					else if(*dnPtr=='.')
						depthState=1;
					else
						depthState=0;
					break;
				
				case 3: // Read ".[0-9]+k"
					if(tolower(*dnPtr)=='m')
						depthState=4;
					else if(*dnPtr=='.')
						depthState=1;
					else
						depthState=0;
					break;
				
				case 4: // Read ".[0-9]+km"
					if(*dnPtr=='.')
						depthState=5;
					else
						depthState=0;
					break;
				
				case 5: // Read ".[0-9]+km."
					depthState=5;
					break;
				}
			
			if(*dnPtr=='.')
				extPtr=dnPtr;
			}
		
		if(extPtr!=0&&strcasecmp(extPtr,".dat")==0&&depthState==5)
			{
			/* Store the file name: */
			gridFiles.push_back(GridFile(dirEntry->d_name,depth));
			}
		}
	closedir(gridFileDir);
	
	/* Sort the list of grid files by depth: */
	std::sort(gridFiles.begin(),gridFiles.end());
	
	/* Create the data set: */
	DS::Index numVertices(gridFiles.size(),181,91);
	EarthDataSet<DataSet>* result=new EarthDataSet<DataSet>(args);
	result->getSphericalCoordinateTransformer()->setDepth(true);
	result->getDs().setGrids(1);
	result->getDs().setGridData(0,numVertices);
	
	/* Set the data value's name: */
	result->getDataValue().setScalarVariableName("Differential Wave Velocity");
	
	/* Read all grid files: */
	DS::Array& vertices=result->getDs().getGrid(0).getVertices();
	DS::Index index;
	for(index[0]=0;index[0]<vertices.getSize(0);++index[0])
		{
		/* Open the grid file: */
		std::string gridFileName=args[0];
		gridFileName.push_back('/');
		gridFileName.append(gridFiles[vertices.getSize(0)-1-index[0]].fileName);
		Misc::File gridFile(gridFileName.c_str(),"rt");
		double depth=double(gridFiles[vertices.getSize(0)-1-index[0]].depth)*1000.0;
		
		/* Constant parameters for geoid formula: */
		const double a=6378.14e3; // Equatorial radius in m
		const double f=1.0/298.247; // Geoid flattening factor
		
		/* Read all vertices from the grid file: */
		unsigned int lineNumber=1;
		for(index[2]=0;index[2]<vertices.getSize(2);++index[2])
			{
			for(index[1]=0;index[1]<vertices.getSize(1)-1;++index[1],++lineNumber)
				{
				/* Read latitude, longitude, and density from grid file: */
				char line[80];
				gridFile.gets(line,sizeof(line));
				double lat,lng;
				float density;
				if(sscanf(line,"%lf %lf %f",&lng,&lat,&density)!=3)
					Misc::throwStdErr("EarthTomographyGrid::load: invalid data in line %u in grid file %s",lineNumber,gridFileName.c_str());
				lat=Math::rad(-lat);
				lng=Math::rad(lng);
				
				/* Convert geoid coordinates to Cartesian coordinates: */
				DS::GridVertex& vertex=vertices(index);
				double s0=Math::sin(lat);
				double c0=Math::cos(lat);
				double r=(a*(1.0-f*Math::sqr(s0))-depth)*0.001;
				double xy=r*c0;
				double s1=Math::sin(lng);
				double c1=Math::cos(lng);
				vertex.pos[0]=float(xy*c1);
				vertex.pos[1]=float(xy*s1);
				vertex.pos[2]=float(r*s0);
				
				/* Store the density value: */
				vertex.value=density;
				}
			vertices(index[0],vertices.getSize(1)-1,index[2])=vertices(index[0],0,index[2]);
			}
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

Visualization::Abstract::DataSetRenderer* EarthTomographyGrid::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
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
	Visualization::Concrete::EarthTomographyGrid* module=new Visualization::Concrete::EarthTomographyGrid();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
