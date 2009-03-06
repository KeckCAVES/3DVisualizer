/***********************************************************************
CuttingPlane - Helper structure to describe user-defined OpenGL cutting
planes.
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

#ifndef CUTTINGPLANE_INCLUDED
#define CUTTINGPLANE_INCLUDED

#include <Geometry/Plane.h>
#include <Vrui/Geometry.h>

struct CuttingPlane // Structure describing a cutting plane
	{
	/* Embedded classes: */
	public:
	typedef Vrui::Plane Plane; // Data type for planes

	/* Elements: */
	bool allocated; // Flag if this cutting plane is currently allocated by a cutting plane locator
	Plane plane; // Current plane equation of cutting plane
	bool active; // Flag if this cutting plane is currently enabled
	};

#endif
