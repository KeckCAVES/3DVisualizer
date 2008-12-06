/***********************************************************************
MultiScalarValue - Class for data values of multiple-value scalar data
sets.
Copyright (c) 2007 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_MULTISCALARVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_MULTISCALARVALUE_INCLUDED

#include <string.h>

#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Wrappers {

template <class VScalarParam,int numComponentsParam>
class MultiValue
	{
	/* Embedded classes: */
	public:
	typedef VScalarParam VScalar;
	static const int numComponents=numComponentsParam;
	
	/* Elements: */
	public:
	VScalar components[numComponents]; // Array of value components
	};

}

namespace Templatized {

template <class VScalarParam,int numComponentsParam>
class ScalarExtractor<VScalarParam,Wrappers::MultiValue<VScalarParam,numComponentsParam> >
	{
	/* Embedded classes: */
	public:
	typedef VScalarParam Scalar; // Returned scalar type
	typedef VScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Wrappers::MultiValue<VScalarParam,numComponentsParam> SourceValue; // Source value type
	
	/* Elements: */
	private:
	int componentIndex; // Index of extracted value component
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(int sComponentIndex) // Creates a scalar extractor for the given value component
		:componentIndex(sComponentIndex)
		{
		}
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return source.components[componentIndex];
		}
	};

}

namespace Wrappers {

template <class DSParam,class VScalarParam>
class MultiScalarValue:public DataValue<DSParam,VScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef DataValue<DSParam,VScalarParam> Base;
	static const int numComponents=Base::Value::numComponents;
	typedef typename Base::SE SE;
	
	/* Elements: */
	private:
	char* scalarVariableNames[numComponents]; // Array of names of the individual scalar variables
	
	/* Constructors and destructors: */
	public:
	MultiScalarValue(void) // Dummy constructor
		{
		for(int i=0;i<numComponents;++i)
			scalarVariableNames[i]=0;
		}
	private:
	MultiScalarValue(const MultiScalarValue& source); // Prohibit copy constructor
	MultiScalarValue& operator=(const MultiScalarValue& source); // Prohibit assignment operator
	public:
	~MultiScalarValue(void)
		{
		for(int i=0;i<numComponents;++i)
			delete[] scalarVariableNames[i];
		}
	
	/* Methods: */
	void setScalarVariableName(int index,const char* newScalarVariableName) // Sets the given scalar variable's name
		{
		delete[] scalarVariableNames[index];
		scalarVariableNames[index]=new char[strlen(newScalarVariableName)+1];
		strcpy(scalarVariableNames[index],newScalarVariableName);
		}
	int getNumScalarVariables(void) const
		{
		return numComponents;
		}
	const char* getScalarVariableName(int index) const
		{
		return scalarVariableNames[index];
		}
	SE getScalarExtractor(int index) const
		{
		return SE(index);
		}
	};

}

}

#endif
