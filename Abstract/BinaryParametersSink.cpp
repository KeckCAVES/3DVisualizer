/***********************************************************************
BinaryParametersSink - Generic class for parameter sinks utilizing the
pipe I/O abstraction.
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

#include <Abstract/BinaryParametersSink.h>

#include <string>
#include <Misc/StandardMarshallers.h>
#include <IO/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>

namespace Visualization {

namespace Abstract {

/*************************************
Methods of class BinaryParametersSink:
*************************************/

template <class DataSinkParam>
inline
BinaryParametersSink<DataSinkParam>::BinaryParametersSink(
	const VariableManager* sVariableManager,
	DataSinkParam& sSink,
	bool sRaw)
	:ParametersSink(sVariableManager),
	 sink(sSink),raw(sRaw)
	{
	}

template <class DataSinkParam>
inline
void
BinaryParametersSink<DataSinkParam>::write(
	const char* name,
	const WriterBase& value)
	{
	value.write(sink);
	}

template <class DataSinkParam>
inline
void
BinaryParametersSink<DataSinkParam>::writeScalarVariable(
	const char* name,
	int scalarVariableIndex)
	{
	if(raw)
		{
		/* Write the variable index directly: */
		sink.template write<int>(scalarVariableIndex);
		}
	else
		{
		/* Write the name of the variable: */
		std::string scalarVariableName=variableManager->getScalarVariableName(scalarVariableIndex);
		Misc::Marshaller<std::string>::write(scalarVariableName,sink);
		}
	}

template <class DataSinkParam>
inline
void
BinaryParametersSink<DataSinkParam>::writeVectorVariable(
	const char* name,
	int vectorVariableIndex)
	{
	if(raw)
		{
		/* Write the variable index directly: */
		sink.template write<int>(vectorVariableIndex);
		}
	else
		{
		/* Write the name of the variable: */
		std::string vectorVariableName=variableManager->getVectorVariableName(vectorVariableIndex);
		Misc::Marshaller<std::string>::write(vectorVariableName,sink);
		}
	}

/****************************************************************
Force instantiation of all standard BinaryParametersSink classes:
****************************************************************/

template class BinaryParametersSink<IO::File>;
template class BinaryParametersSink<Comm::MulticastPipe>;
template class BinaryParametersSink<Comm::ClusterPipe>;

}

}
