/***********************************************************************
ParametersSource - Abstract base class for sources from which
visualization algorithm parameters can be read.
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

#ifndef VISUALIZATION_ABSTRACT_PARAMETERSSOURCE_INCLUDED
#define VISUALIZATION_ABSTRACT_PARAMETERSSOURCE_INCLUDED

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

class ReaderBase // Base class for atomic or compound values that can be read from sources
	{
	/* Constructors and destructors: */
	public:
	virtual ~ReaderBase(void)
		{
		}
	
	/* Methods: */
	virtual void read(const std::string& string) const =0; // Reads the value from a string
	virtual void read(IO::File& file) const =0; // Reads the value from a binary file
	};

template <class DataParam>
class Reader:public ReaderBase // Generic class for values that can be read from sources
	{
	/* Embedded classes: */
	public:
	typedef DataParam Data; // Type of data to be read
	
	/* Elements: */
	private:
	Data& data; // Reference to the datum to be read
	
	/* Constructors and destructors: */
	public:
	Reader(Data& sData)
		:data(sData)
		{
		}
	
	/* Methods from ReaderBase: */
	virtual void read(const std::string& string) const
		{
		data=Misc::ValueCoder<Data>::decode(string.data(),string.data()+string.length());
		}
	virtual void read(IO::File& file) const
		{
		data=Misc::Marshaller<Data>::read(file);
		}
	};


template <class DataParam>
class ArrayReader:public ReaderBase // Generic class for array values that can be read from sources
	{
	/* Embedded classes: */
	public:
	typedef DataParam Data; // Type of data to be read
	
	/* Elements: */
	private:
	Data* elements; // Pointer to the data to be read
	size_t numElements; // Number of elements in the array
	
	/* Constructors and destructors: */
	public:
	ArrayReader(Data* sElements,size_t sNumElements)
		:elements(sElements),numElements(sNumElements)
		{
		}
	
	/* Methods from ReaderBase: */
	virtual void read(const std::string& string) const
		{
		Misc::FixedArrayValueCoder<Data>(elements,numElements).decode(string.data(),string.data()+string.length());
		}
	virtual void read(IO::File& file) const
		{
		Misc::FixedArrayMarshaller<Data>::read(elements,numElements,file);
		}
	};

class ParametersSource
	{
	/* Elements: */
	protected:
	VariableManager* variableManager; // Pointer to variable manager
	
	/* Constructors and destructors: */
	public:
	ParametersSource(VariableManager* sVariableManager)
		:variableManager(sVariableManager)
		{
		}
	virtual ~ParametersSource(void)
		{
		}
	
	/* Methods: */
	VariableManager* getVariableManager(void) // Returns the variable manager
		{
		return variableManager;
		}
	virtual void read(const char* name,const ReaderBase& value) =0; // Reads the value from the source
	virtual void readScalarVariable(const char* name,int& scalarVariableIndex) =0; // Reads a scalar variable from the source
	virtual void readVectorVariable(const char* name,int& vectorVariableIndex) =0; // Reads a vector variable from the source
	};

}

}

#endif
