/***********************************************************************
ParametersSink - Abstract base class for sinks to which visualization
algorithm parameters can be written.
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

#ifndef VISUALIZATION_ABSTRACT_PARAMETERSSINK_INCLUDED
#define VISUALIZATION_ABSTRACT_PARAMETERSSINK_INCLUDED

#include <string>
#include <Misc/Marshaller.h>
#include <Misc/ArrayMarshallers.h>
#include <Misc/ValueCoder.h>
#include <Misc/ArrayValueCoders.h>
#include <IO/File.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class VariableManager;
}
}

namespace Visualization {

namespace Abstract {

class WriterBase // Base class for atomic or compound values that can be written to sinks
	{
	/* Constructors and destructors: */
	public:
	virtual ~WriterBase(void)
		{
		}
	
	/* Methods: */
	virtual size_t getBinarySize(void) const =0; // Returns the size of the value when written to a binary sink
	virtual void write(std::string& string) const =0; // Writes the value to a string
	virtual void write(IO::File& file) const =0; // Writes the value to a binary file
	};

template <class DataParam>
class Writer:public WriterBase // Generic class for values that can be written to sinks
	{
	/* Embedded classes: */
	public:
	typedef DataParam Data; // Type of data to be written
	
	/* Elements: */
	private:
	const Data& data; // Constant reference to the datum to be written
	
	/* Constructors and destructors: */
	public:
	Writer(const Data& sData)
		:data(sData)
		{
		}
	
	/* Methods from WriterBase: */
	virtual size_t getBinarySize(void) const
		{
		return Misc::Marshaller<Data>::getSize(data);
		}
	virtual void write(std::string& string) const
		{
		string=Misc::ValueCoder<Data>::encode(data);
		}
	virtual void write(IO::File& file) const
		{
		Misc::Marshaller<Data>::write(data,file);
		}
	};

template <class DataParam>
class ArrayWriter:public WriterBase // Generic class for array values that can be written to sinks
	{
	/* Embedded classes: */
	public:
	typedef DataParam Data; // Type of data to be written
	
	/* Elements: */
	private:
	const Data* elements; // Pointer to the data to be written
	size_t numElements; // Number of elements in the array
	
	/* Constructors and destructors: */
	public:
	ArrayWriter(const Data* sElements,size_t sNumElements)
		:elements(sElements),numElements(sNumElements)
		{
		}
	
	/* Methods from WriterBase: */
	virtual size_t getBinarySize(void) const
		{
		return Misc::FixedArrayMarshaller<Data>::getSize(elements,numElements);
		}
	virtual void write(std::string& string) const
		{
		string=Misc::FixedArrayValueCoder<Data>(numElements).encode(elements);
		}
	virtual void write(IO::File& file) const
		{
		Misc::FixedArrayMarshaller<Data>::write(elements,numElements,file);
		}
	};

class ParametersSink
	{
	/* Elements: */
	protected:
	const VariableManager* variableManager; // Pointer to variable manager
	
	/* Constructors and destructors: */
	public:
	ParametersSink(const VariableManager* sVariableManager)
		:variableManager(sVariableManager)
		{
		}
	virtual ~ParametersSink(void)
		{
		}
	
	/* Methods: */
	const VariableManager* getVariableManager(void) const // Returns the variable manager
		{
		return variableManager;
		}
	virtual void write(const char* name,const WriterBase& value) =0; // Writes the value to the sink
	virtual void writeScalarVariable(const char* name,int scalarVariableIndex) =0; // Writes a scalar variable to the sink
	virtual void writeVectorVariable(const char* name,int vectorVariableIndex) =0; // Writes a vector variable to the sink
	};

}

}

#endif
