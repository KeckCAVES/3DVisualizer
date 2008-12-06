/***********************************************************************
TriangleRenderer - Class to immediately render triangles created by
visualization algorithms.
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

#ifndef VISUALIZATION_TEMPLATIZED_TRIANGLERENDERER_INCLUDED
#define VISUALIZATION_TEMPLATIZED_TRIANGLERENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLVertex.h>

namespace Visualization {

namespace Templatized {

template <class VertexParam>
class TriangleRenderer
	{
	/* Embedded classes: */
	public:
	typedef VertexParam Vertex; // Type for triangle vertices
	
	/* Elements: */
	private:
	Vertex triangle[3]; // Array of vertices defining one triangle
	
	/* Constructors and destructors: */
	public:
	TriangleRenderer(void) // Creates triangle renderer
		{
		/* Start rendering triangles: */
		glBegin(GL_TRIANGLES);
		}
	private:
	TriangleRenderer(const TriangleRenderer& source); // Prohibit copy constructor
	TriangleRenderer& operator=(const TriangleRenderer& source); // Prohibit assignment operator
	public:
	~TriangleRenderer(void) // Destroys triangle renderer
		{
		/* Stop rendering triangles: */
		glEnd();
		}
	
	/* Methods: */
	Vertex* getNextTriangleVertices(void) // Returns pointer to next vertex triple
		{
		/* Return pointer to the triangle buffer: */
		return triangle;
		}
	void addTriangle(void) // Immediately renders the triangle
		{
		/* Pass the last three vertices to OpenGL: */
		for(int i=0;i<3;++i)
			glVertex(triangle[i]);
		}
	};

}

}

#endif
