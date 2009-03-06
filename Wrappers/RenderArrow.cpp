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

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLVertex.h>
#include <GL/GLGeometryWrappers.h>

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
	GLuint numPoints)
	{
	/* Define local types: */
	typedef ScalarParam Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	
	/* Erect a coordinate frame at the base point, with z facing along the direction: */
	Vector z=direction;
	Scalar arrowLen=Geometry::mag(z);
	z.normalize();
	Vector x=Geometry::normal(z);
	x.normalize();
	Vector y=Geometry::cross(z,x);
	y.normalize();
	
	glBegin(GL_POLYGON);
	glNormal(-z);
	for(GLuint i=numPoints;i>0;--i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i-1)/Scalar(numPoints);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		Vector r=x*c+y*s;
		r*=arrowShaftRadius;
		glVertex(base+r);
		}
	glEnd();
	Point tipBase=base+z*(arrowLen-arrowTipLength);
	glBegin(GL_QUAD_STRIP);
	for(GLuint i=0;i<=numPoints;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(numPoints);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		Vector r=x*c+y*s;
		glNormal(r);
		r*=arrowShaftRadius;
		glVertex(tipBase+r);
		glVertex(base+r);
		}
	glEnd();
	glBegin(GL_QUAD_STRIP);
	glNormal(-z);
	for(GLuint i=0;i<=numPoints;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(numPoints);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		Vector r=x*c+y*s;
		glVertex(tipBase+r*arrowTipRadius);
		glVertex(tipBase+r*arrowShaftRadius);
		}
	glEnd();
	Point tip=base+z*arrowLen;
	glBegin(GL_QUAD_STRIP);
	for(GLuint i=0;i<=numPoints;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(numPoints);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		Vector r=x*c+y*s;
		glNormal(r*arrowTipLength+z*arrowTipRadius);
		r*=arrowTipRadius;
		glVertex(tip);
		glVertex(tipBase+r);
		}
	glEnd();
	}

GLuint getArrowNumVertices(GLuint numPoints)
	{
	return numPoints*7;
	}

GLuint getArrowNumIndices(GLuint numPoints)
	{
	return numPoints+(numPoints*2+2)*3;
	}

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
	GLuint* indices)
	{
	/* Define local types: */
	typedef ScalarParam Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef GLVertex<GLvoid,0,GLvoid,0,ScalarParam,ScalarParam,3> Vertex;
	typedef typename Vertex::Normal Normal;
	typedef typename Vertex::Position Position;
	
	/* Erect a coordinate frame at the base point, with z facing along the direction: */
	Vector z=direction;
	Scalar arrowLen=Geometry::mag(z);
	z.normalize();
	Vector x=Geometry::normal(z);
	x.normalize();
	Vector y=Geometry::cross(z,x);
	y.normalize();
	
	/* Create the arrow vertices: */
	Point tipBase=base+z*(arrowLen-arrowTipLength);
	Point tip=base+z*arrowLen;
	for(GLuint i=0;i<numPoints;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(numPoints);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		Vector r=x*c+y*s;
		Vector nz=-z;
		Vector rShaft=r*arrowShaftRadius;
		Vector rTipBase=r*arrowTipRadius;
		Vector nTip=(r*arrowTipLength+z*arrowTipRadius)/Math::sqrt(Math::sqr(arrowTipLength)+Math::sqr(arrowTipRadius));
		
		/* Vertex for base polygon: */
		vertices[vertexBase+numPoints*0+i].normal=Normal(nz.getComponents());
		vertices[vertexBase+numPoints*0+i].position=Position((base+rShaft).getComponents());
		
		/* Vertices for arrow shaft quad strip: */
		vertices[vertexBase+numPoints*1+i].normal=Normal(r.getComponents());
		vertices[vertexBase+numPoints*1+i].position=Position((tipBase+rShaft).getComponents());
		vertices[vertexBase+numPoints*2+i].normal=Normal(r.getComponents());
		vertices[vertexBase+numPoints*2+i].position=Position((base+rShaft).getComponents());
		
		/* Vertices for arrow tip base quad strip: */
		vertices[vertexBase+numPoints*3+i].normal=Normal(nz.getComponents());
		vertices[vertexBase+numPoints*3+i].position=Position((tipBase+rTipBase).getComponents());
		vertices[vertexBase+numPoints*4+i].normal=Normal(nz.getComponents());
		vertices[vertexBase+numPoints*4+i].position=Position((tipBase+rShaft).getComponents());
		
		/* Vertices for arrow tip quad strip: */
		vertices[vertexBase+numPoints*5+i].normal=Normal(nTip.getComponents());
		vertices[vertexBase+numPoints*5+i].position=Position(tip.getComponents());
		vertices[vertexBase+numPoints*6+i].normal=Normal(nTip.getComponents());
		vertices[vertexBase+numPoints*6+i].position=Position((tipBase+rTipBase).getComponents());
		}
	
	/* Create a polygon to render the arrow base: */
	GLuint* indexPtr=indices;
	for(GLuint i=numPoints;i>0;--i,++indexPtr)
		*indexPtr=vertexBase+(i-1);
	
	/* Create a quad strip to render the arrow shaft: */
	for(GLuint i=0;i<numPoints;++i,indexPtr+=2)
		{
		indexPtr[0]=vertexBase+numPoints*1+i;
		indexPtr[1]=vertexBase+numPoints*2+i;
		}
	indexPtr[0]=vertexBase+numPoints*1;
	indexPtr[1]=vertexBase+numPoints*2;
	indexPtr+=2;
	
	/* Create a quad strip to render the arrow tip base: */
	for(GLuint i=0;i<numPoints;++i,indexPtr+=2)
		{
		indexPtr[0]=vertexBase+numPoints*3+i;
		indexPtr[1]=vertexBase+numPoints*4+i;
		}
	indexPtr[0]=vertexBase+numPoints*3;
	indexPtr[1]=vertexBase+numPoints*4;
	indexPtr+=2;
	
	/* Create a quad strip to render the arrow tip: */
	for(GLuint i=0;i<numPoints;++i,indexPtr+=2)
		{
		indexPtr[0]=vertexBase+numPoints*5+i;
		indexPtr[1]=vertexBase+numPoints*6+i;
		}
	indexPtr[0]=vertexBase+numPoints*5;
	indexPtr[1]=vertexBase+numPoints*6;
	indexPtr+=2;
	}

void
renderArrow(
	GLuint numPoints,
	const GLuint* indices)
	{
	const GLuint* indexPtr=indices;
	
	/* Render a polygon for the arrow base: */
	glDrawElements(GL_POLYGON,numPoints,GL_UNSIGNED_INT,indexPtr);
	indexPtr+=numPoints;
	
	/* Render a quad strip for the arrow shaft: */
	glDrawElements(GL_QUAD_STRIP,numPoints*2+2,GL_UNSIGNED_INT,indexPtr);
	indexPtr+=numPoints*2+2;
	
	/* Render a quad strip for the arrow tip base: */
	glDrawElements(GL_QUAD_STRIP,numPoints*2+2,GL_UNSIGNED_INT,indexPtr);
	indexPtr+=numPoints*2+2;
	
	/* Render a quad strip for the arrow tip: */
	glDrawElements(GL_QUAD_STRIP,numPoints*2+2,GL_UNSIGNED_INT,indexPtr);
	indexPtr+=numPoints*2+2;
	}

/*********************************************
Force instantiation of all standard functions:
*********************************************/

template
void
renderArrow<GLfloat>(
	const Geometry::Point<GLfloat,3>& base,
	const Geometry::Vector<GLfloat,3>& direction,
	GLfloat arrowShaftRadius,
	GLfloat arrowTipRadius,
	GLfloat arrowTipLength,
	GLuint numPoints);

template
void
createArrow<GLfloat>(
	const Geometry::Point<GLfloat,3>& base,
	const Geometry::Vector<GLfloat,3>& direction,
	GLfloat arrowShaftRadius,
	GLfloat arrowTipRadius,
	GLfloat arrowTipLength,
	GLuint numPoints,
	GLVertex<GLvoid,0,GLvoid,0,GLfloat,GLfloat,3>* vertices,
	GLuint vertexBase,
	GLuint* indices);

template
void
renderArrow<GLdouble>(
	const Geometry::Point<GLdouble,3>& base,
	const Geometry::Vector<GLdouble,3>& direction,
	GLdouble arrowShaftRadius,
	GLdouble arrowTipRadius,
	GLdouble arrowTipLength,
	GLuint numPoints);

template
void
createArrow<GLdouble>(
	const Geometry::Point<GLdouble,3>& base,
	const Geometry::Vector<GLdouble,3>& direction,
	GLdouble arrowShaftRadius,
	GLdouble arrowTipRadius,
	GLdouble arrowTipLength,
	GLuint numPoints,
	GLVertex<GLvoid,0,GLvoid,0,GLdouble,GLdouble,3>* vertices,
	GLuint vertexBase,
	GLuint* indices);

}

}
