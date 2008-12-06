/***********************************************************************
Simplex - Policy class to select appropriate cell algorithms for a
given data set class.
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

#include <Templatized/Simplex.h>

namespace Visualization {

namespace Templatized {

/***********************************
Static elements of class Simplex<2>:
***********************************/

const int Simplex<2>::edgeVertexIndices[Simplex<2>::numEdges][2]=
	{
	{0,1},{0,2},{1,2}
	};

const int Simplex<2>::faceVertexIndices[Simplex<2>::numFaces][Simplex<2>::numFaceVertices]=
	{
	{0,1},{1,2},{2,0}
	};

/***********************************
Static elements of class Simplex<3>:
***********************************/

const int Simplex<3>::edgeVertexIndices[Simplex<3>::numEdges][2]=
	{
	{0,1},{0,2},{0,3},
	{1,2},{1,3},{2,3}
	};

const int Simplex<3>::faceVertexIndices[Simplex<3>::numFaces][Simplex<3>::numFaceVertices]=
	{
	{1,3,2},{0,2,3},
	{0,3,1},{0,1,2}
	};

}

}
