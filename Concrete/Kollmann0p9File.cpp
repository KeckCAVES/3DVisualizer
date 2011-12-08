/***********************************************************************
Kollmann0p9File - Class to encapsulate operations on Wolfgang Kollmann's
fluid dynamics simulation data sets.
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

#include <stdio.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

#include <Concrete/Kollmann0p9File.h>

namespace Visualization {

namespace Concrete {

namespace {

/***************
Helper function:
***************/

template <class VectorParam,class ScalarParam>
static void cylindricalToCartesian(VectorParam& vector,const ScalarParam cylindrical[3])
	{
	vector[0]=Math::cos(cylindrical[1])*cylindrical[0];
	vector[1]=Math::sin(cylindrical[1])*cylindrical[0];
	vector[2]=cylindrical[2];
	}

}

/********************************
Methods of class Kollmann0p9File:
********************************/

Kollmann0p9File::Kollmann0p9File(void)
	:BaseModule("Kollmann0p9File")
	{
	}

Visualization::Abstract::DataSet* Kollmann0p9File::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the data file: */
	Misc::File dataFile(args[0].c_str(),"rt");
	
	/* Create result data set: */
	DS::Index numVertices(195,71,129); // Hard-coded; there is no dimension field in the data file
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices);
	
	/* Set the data value's name: */
	result->getDataValue().setVectorVariableName("Velocity");
	
	/* Read all vertex positions and values: */
	DS::Array& vertices=result->getDs().getVertices();
	char line[256];
	for(DS::Index index(0);index[0]<vertices.getSize(0);vertices.preInc(index))
		{
		DS::GridVertex& vertex=vertices(index);
		if(index[2]==128)
			vertex=vertices(index[0],index[1],0);
		else
			{
			dataFile.gets(line,sizeof(line));
			float pos[3],value[3];
			if(sscanf(line,"%f %f %f %f %f %f",&pos[0],&pos[1],&pos[2],&value[0],&value[1],&value[2])!=6)
				Misc::throwStdErr("Kollmann0p9File::load: Error while reading data file %s",args[0].c_str());
			cylindricalToCartesian(vertex.pos,pos);
			cylindricalToCartesian(vertex.value,value);
			}
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
	Visualization::Concrete::Kollmann0p9File* module=new Visualization::Concrete::Kollmann0p9File();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
