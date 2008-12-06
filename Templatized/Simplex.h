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

#ifndef VISUALIZATION_TEMPLATIZED_SIMPLEX_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SIMPLEX_INCLUDED

namespace Visualization {

namespace Templatized {

/*****************************************
Generic class for n-dimensional simplices:
*****************************************/

template <int dimensionParam>
class Simplex
	{
	/* Embedded classes: */
	public:
	static const int dimension=dimensionParam; // Dimension of the simplex
	static const int numVertices=dimensionParam+1; // Number of vertices
	static const int numEdges=((dimensionParam+1)*dimensionParam)/2; // Number of edges
	static const int numFaces=dimensionParam+1; // Number of faces
	static const int numFaceVertices=dimensionParam; // Number of vertices per face
	};

/***********************************************************
Specialized class for two-dimensional simplices (triangles):
***********************************************************/

template <>
class Simplex<2>
	{
	/* Embedded classes: */
	public:
	static const int dimension=2; // Dimension of the simplex
	static const int numVertices=3; // Number of vertices
	static const int numEdges=3; // Number of edges
	static const int numFaces=3; // Number of faces
	static const int numFaceVertices=2; // Number of vertices per face
	static const int edgeVertexIndices[numEdges][2]; // Indices of edge vertices
	static const int faceVertexIndices[numFaces][numFaceVertices]; // Indices of face vertices
	};

/**************************************************************
Specialized class for three-dimensional simplices (tetrahedra):
**************************************************************/

template <>
class Simplex<3>
	{
	/* Embedded classes: */
	public:
	static const int dimension=3; // Dimension of the simplex
	static const int numVertices=4; // Number of vertices
	static const int numEdges=6; // Number of edges
	static const int numFaces=4; // Number of faces
	static const int numFaceVertices=3; // Number of vertices per face
	static const int edgeVertexIndices[numEdges][2]; // Indices of edge vertices
	static const int faceVertexIndices[numFaces][numFaceVertices]; // Indices of face vertices
	};

}

}

#endif
