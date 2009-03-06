/***********************************************************************
MultiStreamline - Wrapper class for multiple related streamlines as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2006-2009 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_MULTISTREAMLINE_IMPLEMENTATION

#include <GL/gl.h>

#include <Wrappers/MultiStreamline.h>

namespace Visualization {

namespace Wrappers {

/********************************
Methods of class MultiStreamline:
********************************/

template <class DataSetWrapperParam>
inline
MultiStreamline<DataSetWrapperParam>::MultiStreamline(
	Visualization::Abstract::Parameters* sParameters,
	unsigned int numStreamlines,
	const GLColorMap* sColorMap,
	Comm::MulticastPipe* pipe)
	:Visualization::Abstract::Element(sParameters),
	 colorMap(sColorMap),
	 multiPolyline(numStreamlines,pipe)
	{
	}

template <class DataSetWrapperParam>
inline
MultiStreamline<DataSetWrapperParam>::~MultiStreamline(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
std::string
MultiStreamline<DataSetWrapperParam>::getName(
	void) const
	{
	return "Streamline Bundle";
	}

template <class DataSetWrapperParam>
inline
size_t
MultiStreamline<DataSetWrapperParam>::getSize(
	void) const
	{
	return multiPolyline.getMaxNumVertices();
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamline<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Set up OpenGL state for streamline rendering: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLboolean texture1DEnabled=glIsEnabled(GL_TEXTURE_1D);
	if(!texture1DEnabled)
		glEnable(GL_TEXTURE_1D);
	GLboolean texture2DEnabled=glIsEnabled(GL_TEXTURE_2D);
	if(texture2DEnabled)
		glDisable(GL_TEXTURE_2D);
	GLboolean texture3DEnabled=glIsEnabled(GL_TEXTURE_3D);
	if(texture3DEnabled)
		glDisable(GL_TEXTURE_3D);
	
	/* Upload the color map as a 1D texture: */
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA8,256,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE,&matrixMode);
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	double mapMin=colorMap->getScalarRangeMin();
	double mapRange=colorMap->getScalarRangeMax()-mapMin;
	glScaled(1.0/mapRange,1.0,1.0);
	glTranslated(-mapMin,0.0,0.0);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	/* Render the streamline representations: */
	multiPolyline.glRenderAction(contextData);
	
	/* Reset OpenGL state: */
	glPopMatrix();
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(matrixMode);
	if(texture3DEnabled)
		glEnable(GL_TEXTURE_3D);
	if(texture2DEnabled)
		glEnable(GL_TEXTURE_2D);
	if(!texture1DEnabled)
		glDisable(GL_TEXTURE_1D);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}

}

}
