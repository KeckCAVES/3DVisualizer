/***********************************************************************
BinaryParametersSink - Generic class for parameter sinks utilizing the
pipe I/O abstraction.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010-2011 Oliver Kreylos

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

#include <Abstract/BinaryParametersSink.h>

#include <string>
#include <Misc/StandardMarshallers.h>
#include <IO/File.h>

namespace Visualization {

namespace Abstract {

/*************************************
Methods of class BinaryParametersSink:
*************************************/

BinaryParametersSink::BinaryParametersSink(const VariableManager* sVariableManager,IO::File& sSink,bool sRaw)
	:ParametersSink(sVariableManager),
	 sink(sSink),raw(sRaw)
	{
	}

void BinaryParametersSink::write(const char* name,const WriterBase& value)
	{
	value.write(sink);
	}

void BinaryParametersSink::writeScalarVariable(const char* name,int scalarVariableIndex)
	{
	if(raw)
		{
		/* Write the variable index directly: */
		sink.write<int>(scalarVariableIndex);
		}
	else
		{
		/* Write the name of the variable: */
		std::string scalarVariableName=variableManager->getScalarVariableName(scalarVariableIndex);
		Misc::Marshaller<std::string>::write(scalarVariableName,sink);
		}
	}

void BinaryParametersSink::writeVectorVariable(const char* name,int vectorVariableIndex)
	{
	if(raw)
		{
		/* Write the variable index directly: */
		sink.write<int>(vectorVariableIndex);
		}
	else
		{
		/* Write the name of the variable: */
		std::string vectorVariableName=variableManager->getVectorVariableName(vectorVariableIndex);
		Misc::Marshaller<std::string>::write(vectorVariableName,sink);
		}
	}

}

}
