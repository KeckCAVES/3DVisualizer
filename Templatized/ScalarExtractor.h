/***********************************************************************
ScalarExtractor - Generic class to extract scalar types from arbitrary
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

#ifndef VISUALIZATION_SCALAREXTRACTOR_INCLUDED
#define VISUALIZATION_SCALAREXTRACTOR_INCLUDED

#include <stdexcept>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,class SourceValueParam>
class ScalarExtractor
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef SourceValueParam SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		throw std::runtime_error("ScalarExtractor::getValue: no default behaviour defined");
		}
	};

/*****************************************************************
Specialized versions of ScalarExtractor for standard scalar types:
*****************************************************************/

template <class ScalarParam>
class ScalarExtractor<ScalarParam,unsigned char>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef unsigned char SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

template <class ScalarParam>
class ScalarExtractor<ScalarParam,signed char>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef signed char SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

template <class ScalarParam>
class ScalarExtractor<ScalarParam,unsigned short>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef unsigned short SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

template <class ScalarParam>
class ScalarExtractor<ScalarParam,signed short>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef signed short SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

template <class ScalarParam>
class ScalarExtractor<ScalarParam,float>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef float SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

template <class ScalarParam>
class ScalarExtractor<ScalarParam,double>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Returned scalar type
	typedef ScalarParam DestValue; // Alias to use scalar extractor as generic value extractor
	typedef double SourceValue; // Source value type
	
	/* Methods: */
	DestValue getValue(const SourceValue& source) const // Extracts scalar from source value
		{
		return DestValue(source);
		}
	};

}

}

#endif
