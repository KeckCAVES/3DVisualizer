/***********************************************************************
SlicedCartesianRenderer - Class to render sliced Cartesian data sets.
Implemented as a specialization of the generic DataSetRenderer class.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDCARTESIANRENDERER_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEDCARTESIANRENDERER_INCLUDED

#include <Templatized/DataSetRenderer.h>
#include <Templatized/SlicedCartesian.h>
#include <Templatized/CartesianGridRenderer.h>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class VScalarParam>
class DataSetRenderer<SlicedCartesian<ScalarParam,dimensionParam,VScalarParam> >:public CartesianGridRenderer<SlicedCartesian<ScalarParam,dimensionParam,VScalarParam> >
	{
	/* Constructors and destructors: */
	public:
	DataSetRenderer(const SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>* sDataSet) // Creates a renderer for the given data set
		:CartesianGridRenderer<SlicedCartesian<ScalarParam,dimensionParam,VScalarParam> >(sDataSet)
		{
		}
	};

}

}

#endif
