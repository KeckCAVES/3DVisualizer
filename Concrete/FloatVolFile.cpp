/***********************************************************************
FloatVolFile - Class to encapsulate operations on scalar-valued data
sets stored in float-valued .vol files.
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

#include <Misc/File.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/FloatVolFile.h>

namespace Visualization {

namespace Concrete {

/*****************************
Methods of class FloatVolFile:
*****************************/

FloatVolFile::FloatVolFile(void)
	:BaseModule("FloatVolFile")
	{
	}

Visualization::Abstract::DataSet* FloatVolFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the volume file: */
	Misc::File file(args[0].c_str(),"rb",Misc::File::BigEndian);
	
	/* Read the volume file header: */
	int volSize[3];
	file.read(volSize,3);
	int borderSize=file.read<int>();
	float domainSize[3];
	file.read(domainSize,3);
	
	/* Create the data set: */
	DS::Index numVertices;
	DS::Size cellSize;
	for(int i=0;i<3;++i)
		{
		numVertices[i]=volSize[i]+2*borderSize;
		cellSize[i]=float(domainSize[i])/float(numVertices[i]-1);
		}
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Read the vertex values from file: */
	file.read(result->getDs().getVertices().getArray(),result->getDs().getVertices().getNumElements());
	
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
	Visualization::Concrete::FloatVolFile* module=new Visualization::Concrete::FloatVolFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
