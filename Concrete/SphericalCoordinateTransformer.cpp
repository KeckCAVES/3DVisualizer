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

#include <Math/Math.h>

#include <Concrete/SphericalCoordinateTransformer.h>

namespace Visualization {

namespace Concrete {

/***********************************************
Methods of class SphericalCoordinateTransformer:
***********************************************/

SphericalCoordinateTransformer::SphericalCoordinateTransformer(void)
	:radius(6378137.0),
	 flatteningFactor(1.0/298.257223563),
	 e2((2.0-flatteningFactor)*flatteningFactor),
	 colatitude(false),radians(false),depth(false)
	{
	}

SphericalCoordinateTransformer::SphericalCoordinateTransformer(const SphericalCoordinateTransformer& source)
	:radius(source.radius),
	 flatteningFactor(source.flatteningFactor),
	 e2(source.e2),
	 colatitude(source.colatitude),radians(source.radians),depth(source.depth)
	{
	}

Visualization::Abstract::CoordinateTransformer* SphericalCoordinateTransformer::clone(void) const
	{
	return new SphericalCoordinateTransformer(*this);
	}

const char* SphericalCoordinateTransformer::getComponentName(int index) const
	{
	const char* result=0;
	switch(index)
		{
		case 0:
			result=colatitude?"Colatitude":"Latitude";
			break;
		
		case 1:
			result="Longitude";
			break;
		
		case 2:
			result=depth?"Depth":"Radius";
			break;
		
		default:
			; // Just to make compiler happy
		}
	
	return result;
	}

Visualization::Abstract::CoordinateTransformer::Point SphericalCoordinateTransformer::transformCoordinate(const Visualization::Abstract::CoordinateTransformer::Point& cartesian) const
	{
	/*****************************************************************************************
	Caution: this is a different formula than the one currently used for spherical->Cartesian:
	*****************************************************************************************/
	
	Point spherical;
	
	Scalar xy=Math::sqrt(Math::sqr(cartesian[0])+Math::sqr(cartesian[1]));
	spherical[0]=Math::atan2(cartesian[2],(Scalar(1)-e2)*xy);
	Scalar lats=Math::sin(spherical[0]);
	Scalar nu=radius/Math::sqrt(Scalar(1)-e2*Math::sqr(lats));
	for(int i=0;i<6;++i)
		{
		spherical[0]=atan2(cartesian[2]+e2*nu*lats,xy);
		lats=Math::sin(spherical[0]);
		nu=radius/Math::sqrt(Scalar(1)-e2*Math::sqr(lats));
		}
	spherical[1]=Math::atan2(cartesian[1],cartesian[0]);
	if(Math::abs(spherical[0])<=Math::rad(Scalar(45)))
		spherical[2]=nu-xy/Math::cos(spherical[0]);
	else
		spherical[2]=(Scalar(1)-e2)*nu-cartesian[2]/lats;
	
	if(colatitude)
		spherical[0]=Math::rad(Scalar(90))-spherical[0];
	if(!radians)
		{
		spherical[0]=Math::deg(spherical[0]);
		spherical[1]=Math::deg(spherical[1]);
		}
	if(!depth)
		spherical[2]=radius-spherical[2];
	
	return spherical;
	}

Visualization::Abstract::CoordinateTransformer::Vector SphericalCoordinateTransformer::transformVector(const Visualization::Abstract::CoordinateTransformer::Point& sourcePoint,const Visualization::Abstract::CoordinateTransformer::Vector& cartesianVector) const
	{
	/***************************************************************************
	Caution: transformation currently assumes that the flattening factor is 0.0:
	***************************************************************************/
	
	/* Get the base point's spherical coordinates: */
	double latitude=double(sourcePoint[0]);
	double longitude=double(sourcePoint[1]);
	if(!radians)
		{
		latitude=Math::rad(latitude);
		longitude=Math::rad(longitude);
		}
	if(colatitude)
		latitude=Math::rad(90.0)-latitude;
	
	return cartesianVector;
	}

void SphericalCoordinateTransformer::setRadius(double newRadius)
	{
	radius=newRadius;
	}

void SphericalCoordinateTransformer::setFlatteningFactor(double newFlatteningFactor)
	{
	flatteningFactor=newFlatteningFactor;
	e2=(2.0-flatteningFactor)*flatteningFactor;
	}

void SphericalCoordinateTransformer::setColatitude(bool newColatitude)
	{
	colatitude=newColatitude;
	}

void SphericalCoordinateTransformer::setRadians(bool newRadians)
	{
	radians=newRadians;
	}

void SphericalCoordinateTransformer::setDepth(bool newDepth)
	{
	depth=newDepth;
	}

}

}
