/***********************************************************************
Polyhedron - Class to represent convex polyhedra resulting from
intersections of half spaces.
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

#ifndef POLYHEDRON_INCLUDED
#define POLYHEDRON_INCLUDED

#include <vector>
#include <Geometry/Point.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Plane;
}

template <class ScalarParam>
class Polyhedron
	{
	/* Embedded classes: */
	public:
	typedef unsigned int Card; // Type for indices
	typedef std::vector<Card> CardList; // Type for lists of indices
	typedef ScalarParam Scalar; // Scalar type for geometric objects
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	typedef Geometry::Plane<Scalar,3> Plane; // Type for planes
	
	private:
	struct Edge // Structure for half-edges
		{
		/* Elements: */
		public:
		Point start; // Start point of the edge
		Card next; // Index of next edge around the same polygon
		Card opposite; // Index of opposite half-edge
		
		/* Constructors and destructors: */
		Edge(void)
			{
			}
		Edge(const Point& sStart,Card sNext,Card sOpposite)
			:start(sStart),next(sNext),opposite(sOpposite)
			{
			}
		};
	
	typedef std::vector<Edge> EdgeList; // Type for lists of edges
	
	/* Elements: */
	EdgeList edges; // Vector of half-edges in no particular order
	
	/* Private methods: */
	void check(void);
	
	/* Constructors and destructors: */
	public:
	Polyhedron(void); // Creates an empty polyhedron
	Polyhedron(const Point& min,const Point& max); // Creates an axis-aligned box
	Polyhedron(const Polyhedron& source); // Copy constructor
	
	/* Methods: */
	Polyhedron* clip(const Plane& plane) const; // Returns a new polyhedron resulting from clipping this one against the given plane
	void drawEdges(void) const; // Draws the polyhedron's edges
	void drawFaces(void) const; // Draws the polyhedron's faces
	void drawIntersection(const Plane& plane) const; // Draws the intersection polygon of the given plane and the polyhedron
	};

#endif
