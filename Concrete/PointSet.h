/***********************************************************************
PointSet - Class to represent and render sets of scattered 3D points.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_CONCRETE_POINTSET_INCLUDED
#define VISUALIZATION_CONCRETE_POINTSET_INCLUDED

#include <Misc/ChunkedArray.h>
#include <GL/gl.h>
#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLObject.h>

#if 0
/* Forward declarations: */
template <class T1,int i1,class T2,int i2,class T3,class T4,int i4>
class GLVertex;
#endif

namespace Visualization {

namespace Concrete {

class PointSet:public GLObject
	{
	/* Embedded classes: */
	private:
	typedef float Scalar; // Scalar type for point coordinates
	typedef GLVertex<void,0,void,0,void,GLfloat,3> Vertex; // Vertex type for points (position only)
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object that contains the point set (0 if extension not supported)
		
		/* Constructors and destructors: */
		public:
		DataItem(void); // Creates a data item
		virtual ~DataItem(void); // Destroys a data item
		};
	
	/* Elements: */
	Misc::ChunkedArray<Vertex> points; // Array of points
	
	/* Constructors and destructors: */
	public:
	PointSet(const char* pointFileName,double flatteningFactor,double scaleFactor); // Creates a point set by reading a file; applies flattening factor to geoid formula and scale factor to Cartesian coordinates
	virtual ~PointSet(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	void glRenderAction(GLContextData& contextData) const; // Renders point set into the current OpenGL context
	};

}

}

#endif
