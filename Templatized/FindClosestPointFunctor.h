/***********************************************************************
FindClosestPointFunctor - Helper class to find closest points in
kd-trees. Used by non-Cartesian data set classes to locate cells
containing query points.
Copyright (c) 2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_FINDCLOSESTPOINTFUNCTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_FINDCLOSESTPOINTFUNCTOR_INCLUDED

namespace Visualization {

namespace Templatized {

template <class StoredPointParam>
class FindClosestPointFunctor
	{
	/* Embedded classes: */
	public:
	typedef StoredPointParam StoredPoint;
	typedef typename StoredPoint::Scalar Scalar;
	typedef typename StoredPoint::Point Point;
	
	/* Elements: */
	private:
	Point queryPosition;
	const StoredPoint* closestPoint;
	Scalar minDist2;
	
	/* Constructors and destructors: */
	public:
	FindClosestPointFunctor(const Point& sQueryPosition,Scalar sMinDist2)
		:queryPosition(sQueryPosition),
		 closestPoint(0),
		 minDist2(sMinDist2)
		{
		}
	
	/* Methods: */
	bool operator()(const StoredPoint& node,int splitDimension)
		{
		/* Compare node's point to current closest point: */
		Scalar dist2=Geometry::sqrDist(node,queryPosition);
		if(minDist2>dist2)
			{
			closestPoint=&node;
			minDist2=dist2;
			}
		
		/* Stop traversal if split plane is farther away than closest point: */
		return minDist2>Math::sqr(node[splitDimension]-queryPosition[splitDimension]);
		}
	const Point& getQueryPosition(void) const
		{
		return queryPosition;
		}
	const StoredPoint* getClosestPoint(void) const
		{
		return closestPoint;
		}
	};

}

}

#endif
