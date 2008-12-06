/***********************************************************************
Plot3DValue - Structure to represent data values stored in NASA Plot3D
files.
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

#ifndef VISUALIZATION_CONCRETE_PLOT3DVALUE_INCLUDED
#define VISUALIZATION_CONCRETE_PLOT3DVALUE_INCLUDED

#include <Misc/Endianness.h>
#include <Geometry/Vector.h>
#include <Geometry/Endianness.h>

#include <Templatized/ScalarExtractor.h>
#include <Templatized/VectorExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Concrete {

/*******************************************
Memory representation of Plot3D file values:
*******************************************/

struct Plot3DValue
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Data type for scalar values
	typedef Geometry::Vector<Scalar,3> Vector; // Data type for vector values
	
	/* Elements: */
	Scalar density;
	Vector momentum;
	Scalar energy;
	
	/* Methods: */
	friend Plot3DValue affineCombination(const Plot3DValue& v1,const Plot3DValue& v2,float weight2)
		{
		float weight1=1.0f-weight2;
		Plot3DValue result;
		result.density=v1.density*weight1+v2.density*weight2;
		for(int i=0;i<3;++i)
			result.momentum[i]=v1.momentum[i]*weight1+v2.momentum[i]*weight2;
		result.energy=v1.energy*weight1+v2.energy*weight2;
		return result;
		}
	};

}

}

namespace Misc {

/****************
Helper functions:
****************/

template <>
inline
void
swapEndianness(Visualization::Concrete::Plot3DValue& value)
	{
	swapEndianness(value.density);
	swapEndianness(value.momentum);
	swapEndianness(value.energy);
	}

}

namespace Visualization {

namespace Templatized {

/***********************************************************
Specialized value extractor classes for Plot3DValue objects:
***********************************************************/

template <class ScalarParam>
class ScalarExtractor<ScalarParam,Visualization::Concrete::Plot3DValue>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Visualization::Concrete::Plot3DValue SourceValue; // Source value type
	
	enum ScalarType
		{
		DENSITY,MOMENTUM_X,MOMENTUM_Y,MOMENTUM_Z,MOMENTUM_MAG,ENERGY
		};
	
	/* Elements: */
	private:
	int scalarType; // Which scalar part is extracted by this object
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(void)
		:scalarType(DENSITY)
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
	DestValue getValue(const SourceValue& source) const // Returns scalar value from data value
		{
		switch(scalarType)
			{
			case DENSITY:
				return DestValue(source.density);
				break;
			
			case MOMENTUM_X:
			case MOMENTUM_Y:
			case MOMENTUM_Z:
				return DestValue(source.momentum[scalarType-MOMENTUM_X]);
				break;
			
			case MOMENTUM_MAG:
				return DestValue(Geometry::mag(source.momentum));
				break;
			
			case ENERGY:
				return DestValue(source.energy);
				break;
			}
		
		return DestValue();
		}
	};

template <class VectorParam>
class VectorExtractor<VectorParam,Visualization::Concrete::Plot3DValue>
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef Visualization::Concrete::Plot3DValue SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Returns scalar value from data value
		{
		return DestValue(source.momentum);
		}
	};

}

namespace Concrete {

/*********************************************
Data value descriptor class for Plot3D values:
*********************************************/

template <class DataSetParam>
class Plot3DDataValue:public Visualization::Wrappers::DataValue<DataSetParam,float>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef Visualization::Wrappers::DataValue<DataSetParam,float> Base;
	typedef typename Base::SE SE;
	typedef typename Base::VE VE;
	
	/* Methods: */
	int getNumScalarVariables(void) const
		{
		return 6;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		static const char* scalarVariableNames[]=
			{
			"Density",
			"Momentum X","Momentum Y","Momentum Z","Momentum Magnitude",
			"Energy"
			};
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
	const char* getVectorVariableName(int vectorVariableIndex) const
		{
		static const char* vectorVariableNames[]=
			{
			"Momentum"
			};
		return vectorVariableNames[vectorVariableIndex];
		}
	VE getVectorExtractor(int vectorVariableIndex) const
		{
		return VE();
		}
	};

}

}

#endif
