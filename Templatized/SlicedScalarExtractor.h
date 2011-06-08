/***********************************************************************
SlicedScalarExtractor - Specialized scalar extractor class to extract
scalar values from data sets containing scalar slices.
Copyright (c) 2008-2009 Oliver Kreylos

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

#ifndef VISUALIZATION_SLICEDSCALAREXTRACTOR_INCLUDED
#define VISUALIZATION_SLICEDSCALAREXTRACTOR_INCLUDED

#include <stddef.h>

#include <Templatized/SlicedDataValue.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Templatized {

template <class ScalarParam,class SourceValueScalarParam>
class ScalarExtractor<ScalarParam,SlicedDataValue<SourceValueScalarParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef SourceValueScalarParam SourceValueScalar; // Source value scalar type
	
	/* Elements: */
	private:
	int sliceIndex; // Index of the value slice from which this extractor reads
	const SourceValueScalar* valueArray; // Pointer to the used slice value array
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(int sSliceIndex,const SourceValueScalar* sValueArray) // Creates extractor for given value array
		:sliceIndex(sSliceIndex),valueArray(sValueArray)
		{
		}
	
	/* Methods: */
	int getSliceIndex(void) const // Returns this extractor's slice index
		{
		return sliceIndex;
		}
	DestValue getValue(ptrdiff_t linearIndex) const // Extracts scalar from given linear index in slice value array
		{
		return DestValue(valueArray[linearIndex]);
		}
	};

}

}

#endif
