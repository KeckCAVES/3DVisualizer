/***********************************************************************
CSConvectionValue - Structure to represent data values computed by
C.S. Natarajan's convection simulations.
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

#ifndef VISUALIZATION_CONCRETE_CSCONVECTIONVALUE_INCLUDED
#define VISUALIZATION_CONCRETE_CSCONVECTIONVALUE_INCLUDED

#include <Misc/Endianness.h>
#include <Geometry/Vector.h>
#include <Geometry/Endianness.h>

#include <Templatized/ScalarExtractor.h>
#include <Templatized/VectorExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Concrete {

/***********************************************************
Memory representation for convection simulation data values:
***********************************************************/

struct CSConvectionValue
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Data type for scalar values
	typedef Geometry::Vector<Scalar,3> Vector; // Data type for vector values
	
	/* Elements: */
	Scalar temperature; // Temperature in C
	Scalar viscosity; // Viscosity in Pa s
	Vector velocity; // Velocity in cm/year
	
	/* Methods: */
	friend CSConvectionValue affineCombination(const CSConvectionValue& v1,const CSConvectionValue& v2,float weight2)
		{
		float weight1=1.0f-weight2;
		CSConvectionValue result;
		result.temperature=v1.temperature*weight1+v2.temperature*weight2;
		result.viscosity=v1.viscosity*weight1+v2.viscosity*weight2;
		for(int i=0;i<3;++i)
			result.velocity[i]=v1.velocity[i]*weight1+v2.velocity[i]*weight2;
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
swapEndianness(Visualization::Concrete::CSConvectionValue& value)
	{
	swapEndianness(value.temperature);
	swapEndianness(value.viscosity);
	swapEndianness(value.velocity);
	}

}

namespace Visualization {

namespace Templatized {

/*****************************************************************
Specialized value extractor classes for CSConvectionValue objects:
*****************************************************************/

template <class ScalarParam>
class ScalarExtractor<ScalarParam,Visualization::Concrete::CSConvectionValue>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Visualization::Concrete::CSConvectionValue SourceValue; // Source value type
	
	enum ScalarType
		{
		TEMPERATURE=0,VISCOSITY,
		VELOCITY_X,VELOCITY_Y,VELOCITY_Z,VELOCITY_MAG
		};
	
	/* Elements: */
	private:
	int scalarType; // Which scalar part is extracted by this object
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(void)
		:scalarType(TEMPERATURE)
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
			case TEMPERATURE:
				return DestValue(source.temperature);
				break;
			
			case VISCOSITY:
				return DestValue(source.viscosity);
				break;
			
			case VELOCITY_X:
			case VELOCITY_Y:
			case VELOCITY_Z:
				return DestValue(source.velocity[scalarType-VELOCITY_X]);
				break;
			
			case VELOCITY_MAG:
				return DestValue(Geometry::mag(source.velocity));
				break;
			}
		
		return DestValue();
		}
	};

template <class VectorParam>
class VectorExtractor<VectorParam,Visualization::Concrete::CSConvectionValue>
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef Visualization::Concrete::CSConvectionValue SourceValue; // Source value type
	
	/* Constructors and destructors: */
	public:
	VectorExtractor(void)
		{
		}
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts vector from source value
		{
		return DestValue(source.velocity);
		}
	};

}

namespace Concrete {

/*****************************************************************
Data value descriptor class for convection simulation file values:
*****************************************************************/

template <class DataSetParam>
class CSConvectionDataValue:public Visualization::Wrappers::DataValue<DataSetParam,float>
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
			"Temperature","Viscosity",
			"Velocity X","Velocity Y","Velocity Z","Velocity Magnitude"
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
			"Velocity"
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
