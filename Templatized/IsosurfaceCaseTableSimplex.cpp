/***********************************************************************
IsosurfaceCaseTableSimplex - Specialized versions of IsosurfaceCaseTable
for two- and three-dimensional simplices.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <Templatized/IsosurfaceCaseTableSimplex.h>

namespace Visualization {

namespace Templatized {

/*********************************************************
Static elements of class IsosurfaceCaseTable<Simplex<3> >:
*********************************************************/

const int IsosurfaceCaseTable<Simplex<3> >::triangleEdgeIndices[16][7]=
	{
	{-1, -1, -1, -1, -1, -1, -1}, //  0
	{ 0,  1,  2, -1, -1, -1, -1}, //  1
	{ 0,  4,  3, -1, -1, -1, -1}, //  2
	{ 1,  2,  4,  4,  3,  1, -1}, //  3
	{ 1,  3,  5, -1, -1, -1, -1}, //  4
	{ 0,  3,  5,  5,  2,  0, -1}, //  5
	{ 0,  4,  5,  5,  1,  0, -1}, //  6
	{ 2,  4,  5, -1, -1, -1, -1}, //  7
	{ 2,  5,  4, -1, -1, -1, -1}, //  8
	{ 0,  1,  5,  5,  4,  0, -1}, //  9
	{ 0,  2,  5,  5,  3,  0, -1}, // 10
	{ 1,  5,  3, -1, -1, -1, -1}, // 11
	{ 1,  3,  4,  4,  2,  1, -1}, // 12
	{ 0,  3,  4, -1, -1, -1, -1}, // 13
	{ 0,  2,  1, -1, -1, -1, -1}, // 14
	{-1, -1, -1, -1, -1, -1, -1}  // 15
	};

const int IsosurfaceCaseTable<Simplex<3> >::edgeMasks[16]=
	{
	0x0000, 0x0007, 0x0019, 0x001e,
	0x002a, 0x002d, 0x0033, 0x0034,
	0x0034, 0x0033, 0x002d, 0x002a,
	0x001e, 0x0019, 0x0007, 0x0000
	};

const int IsosurfaceCaseTable<Simplex<3> >::neighbourMasks[16]=
	{
	0x0000, 0x000e, 0x000d, 0x000f,
	0x000b, 0x000f, 0x000f, 0x0007,
	0x0007, 0x000f, 0x000f, 0x000b,
	0x000f, 0x000d, 0x000e, 0x0000
	};

}

}
