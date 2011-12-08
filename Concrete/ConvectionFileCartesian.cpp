/***********************************************************************
ConvectionFileCartesian - Class to encapsulate operations on convection
simulation data sets treated as Cartesian grids.
Copyright (c) 2006-2007 Oliver Kreylos

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

#include <stdio.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

#include <Concrete/ConvectionFileCartesian.h>

namespace Visualization {

namespace Concrete {

/****************************************
Methods of class ConvectionFileCartesian:
****************************************/

ConvectionFileCartesian::ConvectionFileCartesian(void)
	:BaseModule("ConvectionFileCartesian")
	{
	}

Visualization::Abstract::DataSet* ConvectionFileCartesian::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the data file: */
	Misc::File dataFile(args[0].c_str(),"rt");
	
	char line[256];
	
	/* Skip the title line: */
	dataFile.gets(line,sizeof(line));
	
	/* Skip the variables line: */
	dataFile.gets(line,sizeof(line));
	
	/* Parse the size line: */
	DS::Index numVertices;
	dataFile.gets(line,sizeof(line));
	char dummy[20];
	sscanf(line,"ZONE T= %20s i=%d j=%d k=%d",dummy,&numVertices[2],&numVertices[1],&numVertices[0]);
	
	/* Read the data set vertex positions and values: */
	Misc::Array<DS::Point,3> points(numVertices);
	Misc::Array<DS::Value,3> values(numVertices);
	for(DS::Index index(0);index[0]<numVertices[0];index.preInc(numVertices))
		{
		DS::Point& pos=points(index);
		DS::Value& value=values(index);
		dataFile.gets(line,sizeof(line));
		float dummy;
		if(sscanf(line,"%f %f %f %f %f %f %f",&pos[0],&pos[1],&pos[2],&dummy,&value[0],&value[1],&value[2])!=7)
			Misc::throwStdErr("ConvectionFileCartesian::load: Error while reading data file %s",args[0].c_str());
		}
	
	/* Calculate the average cell size: */
	DS::Index numCells;
	double accumCellSize[3];
	for(int i=0;i<3;++i)
		{
		numCells[i]=numVertices[i]-1;
		accumCellSize[i]=0.0;
		}
	for(DS::Index index(0);index[0]<numCells[0];index.preInc(numCells))
		{
		for(int i=0;i<3;++i)
			{
			DS::Index index2=index;
			++index2[i];
			accumCellSize[i]+=points(index2)[i]-points(index)[i];
			}
		}
	DS::Size cellSize;
	for(int i=0;i<3;++i)
		cellSize[i]=DS::Scalar(accumCellSize[i]/double(numCells.calcIncrement(-1)));
	
	/* Create result data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Set the data value's name: */
	result->getDataValue().setVectorVariableName("Velocity");
	
	/* Copy all vertex values: */
	Misc::Array<DS::Value,3>::const_iterator sIt;
	DS::Array::iterator dIt=result->getDs().getVertices().begin();
	for(sIt=values.begin();sIt!=values.end();++sIt,++dIt)
		*dIt=*sIt;
	
	return result;
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::ConvectionFileCartesian* module=new Visualization::Concrete::ConvectionFileCartesian();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
