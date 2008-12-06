/***********************************************************************
Tesseract - Policy class to select appropriate cell algorithms for a
given data set class.
The order of vertices in a tesseract is bit-wise, such that the LSB
corresponds to the first coordinate axis, and the MSB corresponds to the
last coordinate axis.
The order of edges is dominated by the index of the coordinate axis
parallel to an edge, and then bit-wise through the remaining axes in the
same order as vertices.
The order of faces is dominated by the index of the coordinate axis
orthogonal to a face, and then by position along the same axis.
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

#include <Templatized/Tesseract.h>

namespace Visualization {

namespace Templatized {

/*************************************
Static elements of class Tesseract<2>:
*************************************/

const int Tesseract<2>::edgeVertexIndices[Tesseract<2>::numEdges][2]=
	{
	{0,1},{2,3},
	{0,2},{1,3}
	};

const int Tesseract<2>::faceVertexIndices[Tesseract<2>::numFaces][Tesseract<2>::numFaceVertices]=
	{
	{2,0},{1,3},
	{0,2},{3,2}
	};

/*************************************
Static elements of class Tesseract<3>:
*************************************/

const int Tesseract<3>::edgeVertexIndices[Tesseract<3>::numEdges][2]=
	{
	{0,1},{2,3},{4,5},{6,7},
	{0,2},{1,3},{4,6},{5,7},
	{0,4},{1,5},{2,6},{3,7}
	};

const int Tesseract<3>::faceVertexIndices[Tesseract<3>::numFaces][Tesseract<3>::numFaceVertices]=
	{
	{0,2,6,4},{1,5,7,3},
	{0,4,5,1},{2,3,7,6},
	{0,1,3,2},{4,6,7,5}
	};

}

}
