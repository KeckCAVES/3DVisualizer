/***********************************************************************
SlicedVectorExtractor - Specialized vector extractor class to extract
vector values from data set containing scalar slices.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef VISUALIZATION_SLICEDVECTOREXTRACTOR_INCLUDED
#define VISUALIZATION_SLICEDVECTOREXTRACTOR_INCLUDED

#include <stddef.h>
#include <Geometry/Vector.h>

#include <Templatized/SlicedDataValue.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class VectorParam,class SourceValueParam>
class VectorExtractor;
}
}

namespace Visualization {

namespace Templatized {

template <class VectorParam,class SourceValueScalarParam>
class VectorExtractor<VectorParam,SlicedDataValue<SourceValueScalarParam> >
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	static const int dimension=Vector::dimension; // Dimension of returned vector type
	typedef SourceValueScalarParam SourceValueScalar; // Source value scalar type
	
	/* Elements: */
	private:
	const SourceValueScalar* valueArrays[dimension]; // Pointers to the used slice value arrays
	
	/* Constructors and destructors: */
	public:
	VectorExtractor(void) // Creates an undefined vector extractor
		{
		}
	
	/* Methods: */
	void setSlice(int sliceIndex,const SourceValueScalar* sValueArray) // Sets the value array for one result vector component
		{
		valueArrays[sliceIndex]=sValueArray;
		}
	DestValue getValue(ptrdiff_t linearIndex) const // Extracts vector from given linear index in all slice value arrays
		{
		DestValue result;
		for(int i=0;i<dimension;++i)
			result[i]=typename Vector::Scalar(valueArrays[i][linearIndex]);
		return result;
		}
	};

}

}

#endif
