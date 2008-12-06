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

#include <Templatized/Ellipsoid.h>

namespace Visualization {

namespace Abstract {

/**************************
Methods of class Ellipsoid:
**************************/

Ellipsoid::Ellipsoid(void)
	{
	setEllipsoid(SPHERE,1.0);
	}

Ellipsoid::Ellipsoid(Ellipsoid::StandardEllipsoid ellipsoid,double scaleFactor)
	{
	setEllipsoid(ellipsoid,scaleFactor);
	}

void Ellipsoid::setEllipsoid(Ellipsoid::StandardEllipsoid ellipsoid,double scaleFactor)
	{
	equatorialRadius=6378137.0*scaleFactor;
	switch(ellipsoid)
		{
		case SPHERE:
			flatteningFactor=0.0;
			break;
		
		case WGS84:
			flatteningFactor=1.0/298.257223563;
			break;
		}
	eccentricity=2.0*flatteningFactor-Math::sqr(flatteningFactor);
	}

Point Ellipsoid::cartesianToLatitudeLongitudeRadius(const Point& cartesian) const
	{
	double rad=Math::sqrt(Math::sqr(cartesian[0])+Math::sqr(cartesian[1])+Math::sqr(cartesian[2]));
	double lat=Math::asin(cartesian[2]/rad);
	double lon=Math::atan2(cartesian[1],cartesian[0]);
	
	double r=
	
	latitude=Math::asin(cartesian[2]/radius*(1.0-f*Math::sqr(Math::sin(latitude)));
	
	
	
	
	return Point(lat,lon,rad);
	}

}

}
