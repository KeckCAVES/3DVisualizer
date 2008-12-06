/***********************************************************************
VectorExtractor - Generic class to extract vector types from arbitrary
source value types.
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

#ifndef VISUALIZATION_VECTOREXTRACTOR_INCLUDED
#define VISUALIZATION_VECTOREXTRACTOR_INCLUDED

#include <Misc/ThrowStdErr.h>
#include <Geometry/Vector.h>

namespace Visualization {

namespace Templatized {

template <class VectorParam,class SourceValueParam>
class VectorExtractor
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef SourceValueParam SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts vector from source value
		{
		Misc::throwStdErr("VectorExtractor::getValue: no default behaviour defined");
		return DestValue();
		}
	};

/*****************************************************************
Specialized versions of VectorExtractor for standard vector types:
*****************************************************************/

template <class VectorParam,class SourceScalarParam,int sourceDimensionParam>
class VectorExtractor<VectorParam,Geometry::Vector<SourceScalarParam,sourceDimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef VectorParam Vector; // Returned vector type
	typedef VectorParam DestValue; // Alias to use vector extractor as generic value extractor
	typedef SourceScalarParam SourceScalar; // The source vector's scalar type
	static const int sourceDimension=sourceDimensionParam; // The source vector's dimension
	typedef Geometry::Vector<SourceScalar,sourceDimensionParam> SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts vector from source value
		{
		return DestValue(source);
		}
	};

}

}

#endif
