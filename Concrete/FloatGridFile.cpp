/***********************************************************************
FloatGridFile - Class to encapsulate operations on curvilinear data sets
storing a single floating-point scalar value.
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

#include <Misc/LargeFile.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/FloatGridFile.h>

namespace Visualization {

namespace Concrete {

/******************************
Methods of class FloatGridFile:
******************************/

FloatGridFile::FloatGridFile(void)
	:BaseModule("FloatGridFile")
	{
	}

Visualization::Abstract::DataSet* FloatGridFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Construct the grid file name: */
	char gridFilename[2048];
	snprintf(gridFilename,sizeof(gridFilename),"%s.grid",args[0].c_str());
	
	/* Open the grid file: */
	Misc::LargeFile gridFile(gridFilename,"rb",Misc::LargeFile::LittleEndian);
	
	/* Read the grid file header: */
	DS::Index numVertices;
	gridFile.read<int>(numVertices.getComponents(),3);
	
	/* Create the data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices);
	
	/* Read the vertex positions from file: */
	DS::Array& vertices=result->getDs().getVertices();
	for(DS::Index index(0);index[0]<numVertices[0];index.preInc(numVertices))
		{
		DS::Point p;
		gridFile.read<DS::Scalar>(p.getComponents(),3);
		vertices(index).pos=p;
		}
	
	/* Construct data file name: */
	char dataFilename[2048];
	snprintf(dataFilename,sizeof(dataFilename),"%s.dat",args[0].c_str());
	
	/* Open the data file: */
	Misc::LargeFile dataFile(dataFilename,"rb",Misc::LargeFile::LittleEndian);
	
	/* Read the data file header: */
	DS::Index numDataVertices;
	dataFile.read<int>(numDataVertices.getComponents(),3);
	if(numVertices[0]!=numDataVertices[0]||numVertices[1]!=numDataVertices[1]||numVertices[2]!=numDataVertices[2])
		Misc::throwStdErr("FloatGridFile::load: Size of data file %s does not match grid file %s",dataFilename,gridFilename);
	
	/* Read the vertex values from file: */
	for(DS::Index index(0);index[0]<numVertices[0];index.preInc(numVertices))
		vertices(index).value=dataFile.read<float>();
	
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
	Visualization::Concrete::FloatGridFile* module=new Visualization::Concrete::FloatGridFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
