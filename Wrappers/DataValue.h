/***********************************************************************
DataValue - Base class for data value descriptors that allow users to
enumerate scalar and vector variables stored in a data set, and to
access them using extractors.
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

#ifndef VISUALIZATION_WRAPPERS_DATAVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_DATAVALUE_INCLUDED

#include <Misc/ThrowStdErr.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Vector;
}
namespace Visualization {
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class VectorParam,class SourceValueParam>
class VectorExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DSParam,class VScalarParam>
class DataValue
	{
	/* Embedded classes: */
	public:
	typedef DSParam DS;
	static const int dimension=DS::dimension;
	typedef typename DS::Value Value;
	typedef VScalarParam VScalar;
	typedef Geometry::Vector<VScalar,DS::dimension> VVector;
	typedef Visualization::Templatized::ScalarExtractor<VScalar,Value> SE;
	typedef Visualization::Templatized::VectorExtractor<VVector,Value> VE;
	
	/* Methods: */
	int getNumScalarVariables(void) const // Returns number of scalar variables contained in the data value
		{
		return 0;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const // Returns descriptive name of a scalar variable
		{
		/* This method is never called */
		Misc::throwStdErr("DataValue::getScalarVariableName: unimplemented method called");
		return 0;
		}
	SE getScalarExtractor(int scalarVariableIndex) const // Returns scalar extractor for a scalar variable
		{
		/* This method is never called */
		Misc::throwStdErr("DataValue::getScalarExtractor: unimplemented method called");
		}
	int getNumVectorVariables(void) const // Returns number of vector variables contained in the data value
		{
		return 0;
		}
	const char* getVectorVariableName(int vectorVariableIndex) const // Returns descriptive name of a vector variable
		{
		/* This method is never called */
		Misc::throwStdErr("DataValue::getVectorVariableName: unimplemented method called");
		return 0;
		}
	VE getVectorExtractor(int vectorVariableIndex) const // Returns vector extractor for a vector variable
		{
		/* This method is never called */
		Misc::throwStdErr("DataValue::getVectorExtractor: unimplemented method called");
		return VE();
		}
	};

}

}

#endif
