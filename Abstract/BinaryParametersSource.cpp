/***********************************************************************
BinaryParametersSource - Generic class for parameter sources utilizing
the pipe I/O abstraction.
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

#include <Abstract/BinaryParametersSource.h>

#include <string>
#include <Misc/StandardMarshallers.h>
#include <IO/File.h>

namespace Visualization {

namespace Abstract {

/***************************************
Methods of class BinaryParametersSource:
***************************************/

BinaryParametersSource::BinaryParametersSource(VariableManager* sVariableManager,IO::File& sSource,bool sRaw)
	:ParametersSource(sVariableManager),
	 source(sSource),raw(sRaw)
	{
	}

void BinaryParametersSource::read(const char* name,const ReaderBase& value)
	{
	value.read(source);
	}

void BinaryParametersSource::readScalarVariable(const char* name,int& scalarVariableIndex)
	{
	if(raw)
		{
		/* Read the variable index directly: */
		source.read<int>(scalarVariableIndex);
		}
	else
		{
		/* Read the name of the variable: */
		std::string scalarVariableName=Misc::Marshaller<std::string>::read(source);
		scalarVariableIndex=variableManager->getScalarVariable(scalarVariableName.c_str());
		}
	}

void BinaryParametersSource::readVectorVariable(const char* name,int& vectorVariableIndex)
	{
	if(raw)
		{
		/* Read the variable index directly: */
		source.read<int>(vectorVariableIndex);
		}
	else
		{
		/* Read the name of the variable: */
		std::string vectorVariableName=Misc::Marshaller<std::string>::read(source);
		vectorVariableIndex=variableManager->getVectorVariable(vectorVariableName.c_str());
		}
	}

}

}
