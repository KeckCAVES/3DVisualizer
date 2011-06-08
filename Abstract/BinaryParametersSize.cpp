/***********************************************************************
BinaryParametersSize - Class to calculate sizes of parameter
serializations using the parameter sink interface.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010 Oliver Kreylos

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

#include <Abstract/BinaryParametersSize.h>

#include <string>
#include <Misc/StandardMarshallers.h>

namespace Visualization {

namespace Abstract {

/*************************************
Methods of class BinaryParametersSize:
*************************************/

BinaryParametersSize::BinaryParametersSize(const VariableManager* sVariableManager,bool sRaw)
	:ParametersSink(sVariableManager),
	 size(0),raw(sRaw)
	{
	}

void BinaryParametersSize::write(const char* name,const WriterBase& value)
	{
	size+=value.getBinarySize();
	}

void BinaryParametersSize::writeScalarVariable(const char* name,int scalarVariableIndex)
	{
	if(raw)
		size+=sizeof(int);
	else
		{
		std::string scalarVariableName=variableManager->getScalarVariableName(scalarVariableIndex);
		size+=Misc::Marshaller<std::string>::getSize(scalarVariableName);
		}
	}

void BinaryParametersSize::writeVectorVariable(const char* name,int vectorVariableIndex)
	{
	if(raw)
		size+=sizeof(int);
	else
		{
		std::string vectorVariableName=variableManager->getVectorVariableName(vectorVariableIndex);
		size+=Misc::Marshaller<std::string>::getSize(vectorVariableName);
		}
	}

}

}
