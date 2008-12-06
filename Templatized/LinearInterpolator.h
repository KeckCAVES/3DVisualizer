/***********************************************************************
LinearInterpolator - Generic class to perform linear interpolation of
data values.
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

#ifndef VISUALIZATION_TEMPLATIZED_LINEARINTERPOLATOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_LINEARINTERPOLATOR_INCLUDED

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Vector;
}

namespace Visualization {

namespace Templatized {

template <class ValueParam,class WeightParam>
class LinearInterpolator
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value;
	typedef WeightParam Weight;
	
	/* Methods: */
	inline static Value interpolate(Value v0,Weight w0,Value v1,Weight w1)
		{
		return v0*w0+v1*w1;
		}
	inline static Value interpolate(int numValues,const Value vs[],const Weight ws[])
		{
		Value result=vs[0]*ws[0];
		for(int i=1;i<numValues;++i)
			result+=vs[i]*ws[i];
		return result;
		}
	};

/***********************************************************
Specialized version of LinearInterpolator for vector values:
***********************************************************/

template <class ValueScalarParam,int valueDimensionParam,class WeightParam>
class LinearInterpolator<Geometry::Vector<ValueScalarParam,valueDimensionParam>,WeightParam>
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Vector<ValueScalarParam,valueDimensionParam> Value;
	typedef WeightParam Weight;
	
	/* Methods: */
	inline static Value interpolate(const Value& v0,Weight w0,const Value& v1,Weight w1)
		{
		Value result;
		for(int i=0;i<Value::dimension;++i)
			result[i]=v0[i]*w0+v1[i]*w1;
		return result;
		}
	inline static Value interpolate(int numValues,const Value vs[],const Weight ws[])
		{
		Value result=vs[0];
		result*=ws[0];
		for(int i=1;i<numValues;++i)
			for(int j=0;j<Value::dimension;++j)
				result[j]+=vs[i][j]*ws[i];
		return result;
		}
	};

}

}

#endif
