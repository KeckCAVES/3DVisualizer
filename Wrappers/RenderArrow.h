/***********************************************************************
RenderArrow - Helper functions to render arrow glyphs for vector field
visualization.
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

#ifndef VISUALIZATION_WRAPPERS_RENDERARROW_INCLUDED
#define VISUALIZATION_WRAPPERS_RENDERARROW_INCLUDED

#include <GL/gl.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class Vector;
}
template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex;

namespace Visualization {

namespace Wrappers {

template <class ScalarParam>
void
renderArrow(
	const Geometry::Point<ScalarParam,3>& base,
	const Geometry::Vector<ScalarParam,3>& direction,
	ScalarParam arrowShaftRadius,
	ScalarParam arrowTipRadius,
	ScalarParam arrowTipLength,
	GLuint numPoints); // Function to render an arrow glyph in immediate mode

GLuint getArrowNumVertices(GLuint numPoints); // Function returning the number of vertex array items needed to render an arrow glyph

GLuint getArrowNumIndices(GLuint numPoints); // Function returning the number of index array items needed to render an arrow glyph

template <class ScalarParam>
void
createArrow(
	const Geometry::Point<ScalarParam,3>& base,
	const Geometry::Vector<ScalarParam,3>& direction,
	ScalarParam arrowShaftRadius,
	ScalarParam arrowTipRadius,
	ScalarParam arrowTipLength,
	GLuint numPoints,
	GLVertex<GLvoid,0,GLvoid,0,ScalarParam,ScalarParam,3>* vertices,
	GLuint vertexBase,
	GLuint* indices); // Function to upload the vertices and indices to render an arrow glyph into vertex/index arrays

void renderArrow(GLuint numPoints,const GLuint* indices); // Function to render an arrow glyph from vertex/index arrays

}

}

#endif
