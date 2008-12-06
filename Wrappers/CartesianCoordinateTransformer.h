/***********************************************************************
CartesianCoordinateTransformer - Coordinate transformer from Cartesian
coordinates.
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

#ifndef VISUALIZATION_WRAPPERS_CARTESIANCOORDINATETRANSFORMER_INCLUDED
#define VISUALIZATION_WRAPPERS_CARTESIANCOORDINATETRANSFORMER_INCLUDED

#include <Abstract/CoordinateTransformer.h>

namespace Visualization {

namespace Wrappers {

class CartesianCoordinateTransformer:public Visualization::Abstract::CoordinateTransformer
	{
	/* Constructors and destructors: */
	public:
	CartesianCoordinateTransformer(void) // Default constructor
		{
		}
	protected:
	CartesianCoordinateTransformer(const CartesianCoordinateTransformer& source) // Protect copy constructor
		{
		}
	private:
	CartesianCoordinateTransformer& operator=(const CartesianCoordinateTransformer& source); // Prohibit assignment operator
	
	/* Methods inherited from CoordinateTransformer: */
	public:
	virtual Visualization::Abstract::CoordinateTransformer* clone(void) const;
	virtual const char* getComponentName(int index) const;
	virtual Point transformCoordinate(const Point& cartesian) const
		{
		return cartesian;
		}
	virtual Vector transformVector(const Point& sourcePoint,const Vector& cartesianVector) const
		{
		return cartesianVector;
		}
	};

}

}

#endif
