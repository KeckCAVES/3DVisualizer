/***********************************************************************
CoordinateTransformer - Abstract base class encapsulating how to convert
the Cartesian coordinates used to visualize data sets back to the
original coordinates on which they are defined.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_COORDINATETRANSFORMER_INCLUDED
#define VISUALIZATION_ABSTRACT_COORDINATETRANSFORMER_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>

namespace Visualization {

namespace Abstract {

class CoordinateTransformer
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Type of coordinate scalars
	typedef Geometry::Point<Scalar,3> Point; // Type for Cartesian points and native coordinate points
	typedef Geometry::Vector<Scalar,3> Vector; // Type for Cartesian vectors
	
	/* Constructors and destructors: */
	CoordinateTransformer(void) // Default constructor
		{
		}
	protected:
	CoordinateTransformer(const CoordinateTransformer& source) // Protect copy constructor
		{
		}
	private:
	CoordinateTransformer& operator=(const CoordinateTransformer& source); // Prohibit assignment operator
	public:
	virtual ~CoordinateTransformer(void) // Destructor
		{
		}
	
	/* Methods: */
	virtual CoordinateTransformer* clone(void) const =0; // Returns an identical copy of the coordinate transformer object
	virtual const char* getComponentName(int index) const =0; // Returns the name of one of the three source coordinate components
	virtual Point transformCoordinate(const Point& cartesian) const =0; // Returns the source coordinate point yielding the given Cartesian point
	virtual Vector transformVector(const Point& sourcePoint,const Vector& cartesianVector) const =0; // Returns the source vector yielding the given Cartesian vector based at the given source coordinate point
	};

}

}

#endif
