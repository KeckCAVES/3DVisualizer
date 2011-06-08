/***********************************************************************
HypercubicLocator - Helper class to perform cell location in data sets
consisting of hypercubic cells.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_HYPERCUBICLOCATOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_HYPERCUBICLOCATOR_INCLUDED

namespace Visualization {

namespace Templatized {

template <class DataSetParam>
class HypercubicLocator
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of data set acted upon
	typedef typename DataSet::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DataSet::dimension; // Dimension of data set's domain
	typedef typename DataSet::Point Point; // Type for points in data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in data set's domain
	typedef typename DataSet::CellTopology CellTopology; // Data set's cell topology
	typedef typename DataSet::CellID CellID; // Data set's cell ID type
	typedef typename DataSet::Cell Cell; // Data set's cell type
	typedef typename DataSet::Locator Locator; // Data set's locator type
	typedef typename Locator::CellPosition CellPosition; // Type for cell-relative positions
	
	/* Private methods: */
	private:
	static bool newtonRaphsonStep(Locator& loc,const Point& position);
	
	/* Methods: */
	public:
	static bool locatePoint(Locator& loc,const Point& position,bool traceHint);
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_HYPERCUBICLOCATOR_IMPLEMENTATION
#include <Templatized/HypercubicLocator.icpp>
#endif

#endif
