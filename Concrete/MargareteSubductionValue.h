/***********************************************************************
MargareteSubductionValue - Structure to represent data values computed
by Margarete Jadamec's plate subduction simulations.
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

#ifndef VISUALIZATION_CONCRETE_MARGARETEISUBDUCTIONVALUE_INCLUDED
#define VISUALIZATION_CONCRETE_MARGARETEISUBDUCTIONVALUE_INCLUDED

#include <Misc/Endianness.h>
#include <Geometry/Vector.h>

#include <Templatized/ScalarExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Concrete {

/**********************************************************
Memory representation of subduction simulation file values:
**********************************************************/

struct MargareteSubductionValue
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Data type for scalar values
	
	/* Elements: */
	Scalar temperature; // Temperature in C
	Scalar viscosity; // Viscosity in Pa s
	
	/* Methods: */
	friend MargareteSubductionValue affineCombination(const MargareteSubductionValue& v1,const MargareteSubductionValue& v2,float weight2)
		{
		float weight1=1.0f-weight2;
		MargareteSubductionValue result;
		result.temperature=v1.temperature*weight1+v2.temperature*weight2;
		result.viscosity=v1.viscosity*weight1+v2.viscosity*weight2;
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
swapEndianness(
	Visualization::Concrete::MargareteSubductionValue& value)
	{
	swapEndianness(value.temperature);
	swapEndianness(value.viscosity);
	}

}

namespace Visualization {

namespace Templatized {

/************************************************************************
Specialized value extractor classes for MargareteSubductionValue objects:
************************************************************************/

template <class ScalarParam>
class ScalarExtractor<ScalarParam,Visualization::Concrete::MargareteSubductionValue>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Visualization::Concrete::MargareteSubductionValue SourceValue; // Source value type
	
	enum ScalarType
		{
		TEMPERATURE=0,VISCOSITY
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
			}
		
		return DestValue();
		}
	};

}

namespace Concrete {

/*****************************************************************
Data value descriptor class for subduction simulation file values:
*****************************************************************/

template <class DataSetParam>
class MargareteSubductionDataValue:public Visualization::Wrappers::DataValue<DataSetParam,float>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef Visualization::Wrappers::DataValue<DataSetParam,float> Base;
	typedef typename Base::SE SE;
	
	/* Methods: */
	int getNumScalarVariables(void) const
		{
		return 2;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		static const char* scalarVariableNames[]=
			{
			"Temperature","Log(Viscosity)"
			};
		return scalarVariableNames[scalarVariableIndex];
		}
	SE getScalarExtractor(int scalarVariableIndex) const
		{
		return SE(scalarVariableIndex);
		}
	};

}

}

#endif
