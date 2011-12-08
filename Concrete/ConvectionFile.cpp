/***********************************************************************
ConvectionFile - Class to encapsulate operations on convection
simulation data sets.
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

#include <Concrete/ConvectionFile.h>

namespace Visualization {

namespace Concrete {

/*******************************
Methods of class ConvectionFile:
*******************************/

ConvectionFile::ConvectionFile(void)
	:BaseModule("ConvectionFile")
	{
	}

Visualization::Abstract::DataSet* ConvectionFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
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
	
	/* Create result data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices);
	
	/* Set the data value's name: */
	result->getDataValue().setVectorVariableName("Velocity");
	
	/* Read all vertex positions and values: */
	DS::Array& vertices=result->getDs().getVertices();
	for(DS::Index index(0);index[0]<vertices.getSize(0);vertices.preInc(index))
		{
		DS::GridVertex& vertex=vertices(index);
		dataFile.gets(line,sizeof(line));
		float dummy;
		if(sscanf(line,"%f %f %f %f %f %f %f",&vertex.pos[0],&vertex.pos[1],&vertex.pos[2],&dummy,&vertex.value[0],&vertex.value[1],&vertex.value[2])!=7)
			Misc::throwStdErr("ConvectionFile::load: Error while reading data file %s",args[0].c_str());
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
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
	Visualization::Concrete::ConvectionFile* module=new Visualization::Concrete::ConvectionFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
