/***********************************************************************
SphericalCordinateTransformer - Coordinate transformer from spherical
coordinates on a variety of Geoid models.
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

#ifndef VISUALIZATION_CONCRETE_SPHERICALCOORDINATETRANSFORMER_INCLUDED
#define VISUALIZATION_CONCRETE_SPHERICALCOORDINATETRANSFORMER_INCLUDED

#include <Abstract/CoordinateTransformer.h>

namespace Visualization {

namespace Concrete {

class SphericalCoordinateTransformer:public Visualization::Abstract::CoordinateTransformer
	{
	/* Elements: */
	private:
	Scalar radius; // Equatorial radius of the Geoid
	Scalar flatteningFactor; // Flattening factor of the Geoid
	Scalar e2; // Geoid's eccentricity; derived from flattening factor
	bool colatitude; // Flag whether to return colatitude instead of latitude
	bool radians; // Flag whether to return angles in radians instead of degrees
	bool depth; // Flag whether to return depths instead of radii
	
	/* Constructors and destructors: */
	public:
	SphericalCoordinateTransformer(void); // Default constructor
	protected:
	SphericalCoordinateTransformer(const SphericalCoordinateTransformer& source); // Protect copy constructor
	private:
	SphericalCoordinateTransformer& operator=(const SphericalCoordinateTransformer& source); // Prohibit assignment operator
	
	/* Methods inherited from CoordinateTransformer: */
	public:
	virtual Visualization::Abstract::CoordinateTransformer* clone(void) const;
	virtual const char* getComponentName(int index) const;
	virtual Point transformCoordinate(const Point& cartesian) const;
	virtual Vector transformVector(const Point& sourcePoint,const Vector& cartesianVector) const;
	
	/* New methods: */
	void setRadius(double newRadius); // Sets equatorial radius of Geoid
	void setFlatteningFactor(double newFlatteningFactor); // Sets flattening factor of Geoid
	void setColatitude(bool newColatitude); // Sets the colatitude switch
	void setRadians(bool newRadians); // Sets the radians switch
	void setDepth(bool newDepth); // Sets the depth switch
	};

}

}

#endif
