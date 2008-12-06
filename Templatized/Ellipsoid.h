/***********************************************************************
Ellipsoid - Helper class to transform Cartesian coordinates from/to
geodetical coordinates on an ellipsoid.
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

#ifndef VISUALIZATION_TEMPLATIZED_ELLIPSOID_INCLUDED
#define VISUALIZATION_TEMPLATIZED_ELLIPSOID_INCLUDED

#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>

namespace Visualization {

namespace Templatized {

class Ellipsoid
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Point<double,3> Point; // Type for points in Cartesian and ellipsoid coordinates
	typedef Geometry::Vector<double,3> Vector; // Type for vectors in Cartesian and ellipsoid coordinates
	
	enum StandardEllipsoid // Enumerated type for standard ellipsoids
		{
		SPHERE,WGS84
		};
	
	/* Elements: */
	private:
	double equatorialRadius; // Ellipsoid's radius at the equator, in scaled units (meter * scaleFactor)
	double flatteningFactor; // Ellipsoid's flattening factor
	double eccentricity; // Ellipsoid's eccentricity derived from flattening factor
	
	/* Constructors and destructors: */
	public:
	Ellipsoid(void); // Creates a sphere with Earth's equatorial radius in meters
	Ellipsoid(StandardEllipsoid ellipsoid,double scaleFactor); // Creates an ellipsoid for the given standard
	
	/* Methods: */
	void setEllipsoid(StandardEllipsoid ellipsoid,double scaleFactor); // Creates an ellipsoid for the given standard
	Point latitudeLongitudeRadiusToCartesian(const Point& spherical) const // Converts spherical point in latitude, longitude, radius to Cartesian
		{
		double lats=Math::sin(spherical[0]);
		double r=spherical[2]*(1.0-f*Math::sqr(lats));
		double xy=r*Math::cos(spherical[0]);
		return Point(xy*Math::cos(spherical[1]),xy*Math::sin(spherical[1]),r*lats);
		}
	Point latitudeLongitudeScaledRadiusToCartesian(const Point& spherical) const // Converts spherical point in latitude, longitude, scaled radius (0.0-1.0) to Cartesian
		{
		double lats=Math::sin(spherical[0]);
		double r=equatorialRadius*spherical[2]*(1.0-f*Math::sqr(lats));
		double xy=r*Math::cos(spherical[0]);
		return Point(xy*Math::cos(spherical[1]),xy*Math::sin(spherical[1]),r*lats);
		}
	Point latitudeLongitudeDepthToCartesian(const Point& spherical) const // Converts spherical point in latitude, longitude, depth to Cartesian
		{
		double lats=Math::sin(spherical[0]);
		double r=equatorialRadius*(1.0-f*Math::sqr(lats))-spherical[2];
		double xy=r*Math::cos(spherical[0]);
		return Point(xy*Math::cos(spherical[1]),xy*Math::sin(spherical[1]),r*lats);
		}
	Point cartesianToLatitudeLongitudeRadius(const Point& cartesian) const; // Converts a point in Cartesian coordinates to latitude, longitude, radius
	};

}

}

#endif
