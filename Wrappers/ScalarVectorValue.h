/***********************************************************************
ScalarVectorValue - Class for data values that contain one scalar value
and one vector value.
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

#ifndef VISUALIZATION_WRAPPERS_SCALARVECTORVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_SCALARVECTORVALUE_INCLUDED

#include <string.h>
#include <Geometry/Vector.h>

#include <Templatized/VectorExtractor.h>
#include <Templatized/VectorScalarExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Wrappers {

/**************************************************
Memory representation of scalar-vector data values:
**************************************************/

template <class VScalarParam,int vectorDimensionParam>
class ScalarVectorValue
	{
	/* Embedded classes: */
	public:
	typedef VScalarParam VScalar; // Data type for scalar value
	static const int vectorDimension=vectorDimensionParam; // Dimension of vector data value
	typedef Geometry::Vector<VScalar,vectorDimensionParam> VVector; // Data type for vector value
	
	/* Elements: */
	VScalar scalar; // Scalar value
	VVector vector; // Vector value
	};

}

}

namespace Misc {

/****************
Helper functions:
****************/

template <class VScalarParam,int vectorDimensionParam>
inline
void
swapEndianness(Visualization::Wrappers::ScalarVectorValue<VScalarParam,vectorDimensionParam>& value)
	{
	swapEndianness(value.scalar);
	swapEndianness(value.vector.getComponents(),vectorDimensionParam);
	}

}

namespace Visualization {

namespace Templatized {

/*****************************************************************
Specialized value extractor classes for ScalarVectorValue objects:
*****************************************************************/

template <class ScalarParam,class VScalarParam,int vectorDimensionParam>
class ScalarExtractor<ScalarParam,Visualization::Wrappers::ScalarVectorValue<VScalarParam,vectorDimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Visualization::Wrappers::ScalarVectorValue<VScalarParam,vectorDimensionParam> SourceValue; // Source value type
	
	enum ScalarType
		{
		SCALAR=0,
		VECTOR_X,VECTOR_Y,VECTOR_Z,VECTOR_MAG
		};
	
	/* Elements: */
	private:
	int scalarType; // Which scalar part is extracted by this object
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(void)
		:scalarType(SCALAR)
		{
		}
	ScalarExtractor(int sScalarType)
		:scalarType(sScalarType)
		{
		}
	
	/* Methods: */
	void setScalarType(int newScalarType) // Sets scalar type of extractor
		{
		scalarType=newScalarType;
		}
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		switch(scalarType)
			{
			case SCALAR:
				return DestValue(source.scalar);
				break;
			
			case VECTOR_X:
			case VECTOR_Y:
			case VECTOR_Z:
				return DestValue(source.vector[scalarType-VECTOR_X]);
				break;
			
			case VECTOR_MAG:
				return DestValue(Geometry::mag(source.vector));
				break;
			}
		
		return DestValue();
		}
	};

template <class VectorParam,class VScalarParam,int vectorDimensionParam>
class VectorExtractor<VectorParam,Visualization::Wrappers::ScalarVectorValue<VScalarParam,vectorDimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef Visualization::Wrappers::ScalarVectorValue<VScalarParam,vectorDimensionParam> SourceValue; // Source value type
	
	/* Constructors and destructors: */
	public:
	VectorExtractor(void)
		{
		}
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts vector from source value
		{
		return DestValue(source.vector);
		}
	};

}

namespace Wrappers {

/*********************************************************
Data value descriptor class for scalar-vector data values:
*********************************************************/

template <class DSParam,class VScalarParam>
class ScalarVectorDataValue:public DataValue<DSParam,VScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef DataValue<DSParam,VScalarParam> Base;
	typedef typename Base::SE SE;
	typedef typename Base::VE VE;
	
	/* Elements: */
	private:
	char* scalarVariableNames[5]; // Names of the given and the four derived scalar variables
	char* vectorVariableName; // Name of the single vector variable
	
	/* Constructors and destructors: */
	public:
	ScalarVectorDataValue(void) // Dummy constructor
		:vectorVariableName(0)
		{
		for(int i=0;i<5;++i)
			scalarVariableNames[i]=0;
		}
	ScalarVectorDataValue(const char* sScalarVariableName,const char* sVectorVariableName) // Constructor for given names
		:vectorVariableName(0)
		{
		for(int i=0;i<5;++i)
			scalarVariableNames[i]=0;
		setScalarVariableName(sScalarVariableName);
		setVectorVariableName(sVectorVariableName);
		}
	private:
	ScalarVectorDataValue(const ScalarVectorDataValue& source); // Prohibit copy constructor
	ScalarVectorDataValue& operator=(const ScalarVectorDataValue& source); // Prohibit assignment operator
	public:
	~ScalarVectorDataValue(void)
		{
		for(int i=0;i<5;++i)
			delete[] scalarVariableNames[i];
		delete[] vectorVariableName;
		}
	
	/* Methods: */
	void setScalarVariableName(const char* newScalarVariableName) // Sets the scalar variable's name
		{
		delete[] scalarVariableNames[0];
		scalarVariableNames[0]=new char[strlen(newScalarVariableName)+1];
		strcpy(scalarVariableNames[0],newScalarVariableName);
		}
	void setVectorVariableName(const char* newVectorVariableName) // Sets the vector variable's name
		{
		/* Delete the old variable names: */
		for(int i=1;i<5;++i)
			delete[] scalarVariableNames[i];
		delete[] vectorVariableName;
		
		/* Create the new derived scalar variable names: */
		size_t nameLen=strlen(newVectorVariableName);
		for(int i=0;i<3;++i)
			{
			scalarVariableNames[i+1]=new char[nameLen+3];
			memcpy(scalarVariableNames[i+1],newVectorVariableName,nameLen);
			scalarVariableNames[i+1][nameLen+0]=' ';
			scalarVariableNames[i+1][nameLen+1]=char(i+'X');
			scalarVariableNames[i+1][nameLen+2]='\0';
			}
		scalarVariableNames[4]=new char[nameLen+11];
		memcpy(scalarVariableNames[4],newVectorVariableName,nameLen);
		memcpy(scalarVariableNames[4]+nameLen," Magnitude",10);
		scalarVariableNames[4][nameLen+10]='\0';
		
		/* Store the new vector variable name: */
		vectorVariableName=new char[nameLen+1];
		memcpy(vectorVariableName,newVectorVariableName,nameLen+1);
		}
	int getNumScalarVariables(void) const
		{
		return 5;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		return scalarVariableNames[scalarVariableIndex];
		}
	SE getScalarExtractor(int scalarVariableIndex) const
		{
		return SE(scalarVariableIndex);
		}
	int getNumVectorVariables(void) const
		{
		return 1;
		}
	const char* getVectorVariableName(int) const
		{
		return vectorVariableName;
		}
	VE getVectorExtractor(int) const
		{
		return VE();
		}
	};

}

}

#endif
