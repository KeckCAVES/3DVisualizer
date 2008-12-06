/***********************************************************************
VectorScalarExtractor - Specialized class to extract scalar values from
source vector value types.
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

#ifndef VISUALIZATION_VECTORSCALAREXTRACTOR_INCLUDED
#define VISUALIZATION_VECTORSCALAREXTRACTOR_INCLUDED

#include <Geometry/Vector.h>

#include <Templatized/ScalarExtractor.h>

namespace Visualization {

namespace Templatized {

/*****************************************************************
Specialized versions of ScalarExtractor for standard vector types:
*****************************************************************/

template <class ScalarParam,class SourceScalarParam,int sourceDimensionParam>
class ScalarExtractor<ScalarParam,Geometry::Vector<SourceScalarParam,sourceDimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef SourceScalarParam SourceScalar; // The source vector's scalar type
	static const int sourceDimension=sourceDimensionParam; // The source vector's dimension
	typedef Geometry::Vector<SourceScalar,sourceDimensionParam> SourceValue; // Source value type
	
	/* Elements: */
	private:
	int componentIndex; // Returns one component of the vector, or magnitude if componentIndex==dimension
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(void)
		:componentIndex(sourceDimension)
		{
		}
	ScalarExtractor(int sComponentIndex)
		:componentIndex(sComponentIndex)
		{
		}
	
	/* Methods: */
	void setComponentIndex(int newComponentIndex) // Sets component index of extractor
		{
		componentIndex=newComponentIndex;
		}
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		if(componentIndex<sourceDimension)
			return DestValue(source[componentIndex]);
		else
			return DestValue(Geometry::mag(source));
		}
	};

}

}

#endif


