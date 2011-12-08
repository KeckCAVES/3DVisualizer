/***********************************************************************
MagaliSubductionFile - Class to encapsulate operations on Magali
Billen's subduction simulation data sets.
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
#include <Math/Math.h>
#include <Geometry/Endianness.h>

#include <Concrete/MagaliSubductionFile.h>

namespace Visualization {

namespace Concrete {

/*************************************
Methods of class MagaliSubductionFile:
*************************************/

MagaliSubductionFile::MagaliSubductionFile(void)
	:BaseModule("MagaliSubductionFile")
	{
	}

Visualization::Abstract::DataSet* MagaliSubductionFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the data file: */
	Misc::File dataFile(args[0].c_str(),"rb",Misc::File::LittleEndian);
	
	/* Create result data set: */
	DS::Index numVertices;
	dataFile.read(numVertices.getComponents(),3);
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices);
	
	/* Read all vertex positions and values: */
	DS::Array& vertices=result->getDs().getVertices();
	for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		{
		dataFile.read(vIt->pos);
		dataFile.read(vIt->value);
		vIt->value.viscosity=Math::log(vIt->value.viscosity);
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
	Visualization::Concrete::MagaliSubductionFile* module=new Visualization::Concrete::MagaliSubductionFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
