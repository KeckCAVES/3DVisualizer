/***********************************************************************
MagaliSubductionValue - Structure to represent data values computed by
Magali Billen's plate subduction simulations.
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

#ifndef VISUALIZATION_CONCRETE_MAGALISUBDUCTIONVALUE_INCLUDED
#define VISUALIZATION_CONCRETE_MAGALISUBDUCTIONVALUE_INCLUDED

#include <Misc/Endianness.h>
#include <Geometry/Vector.h>
#include <Geometry/Endianness.h>

#include <Templatized/ScalarExtractor.h>
#include <Templatized/VectorExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Concrete {

/**********************************************************
Memory representation of subduction simulation file values:
**********************************************************/

struct MagaliSubductionValue
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Data type for scalar values
	typedef Geometry::Vector<Scalar,3> Vector; // Data type for vector values
	
	/* Elements: */
	Scalar temperature; // Temperature in C
	Scalar viscosity; // Viscosity in Pa s
	Vector velocity; // Velocity in cm/year
	Scalar eigenValues[3]; // Stress tensor eigenvalues (compression, intermediate, tension)
	Vector eigenVectors[3]; // Stress tensor eigenvectors (compression, intermediate, tension)
	
	/* Methods: */
	friend MagaliSubductionValue affineCombination(const MagaliSubductionValue& v1,const MagaliSubductionValue& v2,float weight2)
		{
		float weight1=1.0f-weight2;
		MagaliSubductionValue result;
		result.temperature=v1.temperature*weight1+v2.temperature*weight2;
		result.viscosity=v1.viscosity*weight1+v2.viscosity*weight2;
		for(int i=0;i<3;++i)
			result.velocity[i]=v1.velocity[i]*weight1+v2.velocity[i]*weight2;
		for(int i=0;i<3;++i)
			result.eigenValues[i]=v1.eigenValues[i]*weight1+v2.eigenValues[i]*weight2;
		for(int i=0;i<3;++i)
			for(int j=0;j<3;++j)
				result.eigenVectors[i][j]=v1.eigenVectors[i][j]*weight1+v2.eigenVectors[i][j]*weight2;
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
swapEndianness(Visualization::Concrete::MagaliSubductionValue& value)
	{
	swapEndianness(value.temperature);
	swapEndianness(value.viscosity);
	swapEndianness(value.velocity);
	swapEndianness(value.eigenValues,3);
	swapEndianness(value.eigenVectors,3);
	}

}


namespace Visualization {

namespace Templatized {

/*********************************************************************
Specialized value extractor classes for MagaliSubductionValue objects:
*********************************************************************/

template <class ScalarParam>
class ScalarExtractor<ScalarParam,Visualization::Concrete::MagaliSubductionValue>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef Visualization::Concrete::MagaliSubductionValue SourceValue; // Source value type
	
	enum ScalarType
		{
		TEMPERATURE=0,VISCOSITY,
		VELOCITY_X,VELOCITY_Y,VELOCITY_Z,VELOCITY_MAG,
		COMPRESSION,INTERMEDIATE,TENSION,
		COMPRESSION_X,COMPRESSION_Y,COMPRESSION_Z,
		INTERMEDIATE_X,INTERMEDIATE_Y,INTERMEDIATE_Z,
		TENSION_X,TENSION_Y,TENSION_Z
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
			
			case COMPRESSION:
			case INTERMEDIATE:
			case TENSION:
				return DestValue(source.eigenValues[scalarType-COMPRESSION]);
				break;
			
			case COMPRESSION_X:
			case COMPRESSION_Y:
			case COMPRESSION_Z:
				return DestValue(source.eigenVectors[0][scalarType-COMPRESSION_X]);
				break;
			
			case INTERMEDIATE_X:
			case INTERMEDIATE_Y:
			case INTERMEDIATE_Z:
				return DestValue(source.eigenVectors[1][scalarType-INTERMEDIATE_X]);
				break;
			
			case TENSION_X:
			case TENSION_Y:
			case TENSION_Z:
				return DestValue(source.eigenVectors[2][scalarType-TENSION_X]);
				break;
			}
		
		return DestValue();
		}
	};

template <class VectorParam>
class VectorExtractor<VectorParam,Visualization::Concrete::MagaliSubductionValue>
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef Visualization::Concrete::MagaliSubductionValue SourceValue; // Source value type
	
	enum VectorType
		{
		VELOCITY=0,COMPRESSION,INTERMEDIATE,TENSION
		};
	
	/* Elements: */
	private:
	int vectorType; // Which vector part is extracted by this object
	
	/* Constructors and destructors: */
	public:
	VectorExtractor(void)
		:vectorType(VELOCITY)
		{
		}
	VectorExtractor(int sVectorType)
		:vectorType(sVectorType)
		{
		}
	
	/* Methods: */
	void setVectorType(int newVectorType) // Sets vector type of extractor
		{
		vectorType=newVectorType;
		}
	DestValue getValue(const SourceValue& source) const // Extracts vector from source value
		{
		switch(vectorType)
			{
			case VELOCITY:
				return DestValue(source.velocity);
				break;
			
			case COMPRESSION:
			case INTERMEDIATE:
			case TENSION:
				return DestValue(source.eigenVectors[vectorType-COMPRESSION]);
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
class MagaliSubductionDataValue:public Visualization::Wrappers::DataValue<DataSetParam,float>
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
		return 18;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		static const char* scalarVariableNames[]=
			{
			"Temperature","Viscosity",
			"Velocity X","Velocity Y","Velocity Z","Velocity Magnitude",
			"Compression","Intermediate","Tension",
			"Compression X","Compression Y","Compression Z",
			"Intermediate X","Intermediate Y","Intermediate Z",
			"Tension X","Tension Y","Tension Z"
			};
		return scalarVariableNames[scalarVariableIndex];
		}
	SE getScalarExtractor(int scalarVariableIndex) const
		{
		return SE(scalarVariableIndex);
		}
	int getNumVectorVariables(void) const
		{
		return 4;
		}
	const char* getVectorVariableName(int vectorVariableIndex) const
		{
		static const char* vectorVariableNames[]=
			{
			"Velocity",
			"Compression","Intermediate","Tension",
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
