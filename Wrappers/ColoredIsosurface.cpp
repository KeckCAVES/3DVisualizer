/***********************************************************************
ColoredIsosurface - Wrapper class for color-mapped isosurfaces as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2008-2009 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_ISOSURFACE_IMPLEMENTATION

#include <GL/gl.h>
#include <GL/GLMaterial.h>

#include <Wrappers/ColoredIsosurface.h>

namespace Visualization {

namespace Wrappers {

/**********************************
Methods of class ColoredIsosurface:
**********************************/

template <class DataSetWrapperParam>
inline
ColoredIsosurface<DataSetWrapperParam>::ColoredIsosurface(
	Visualization::Abstract::Parameters* sParameters,
	bool sLighting,
	const GLColorMap* sColorMap,
	Comm::MulticastPipe* pipe)
	:Visualization::Abstract::Element(sParameters),
	 lighting(sLighting),
	 colorMap(sColorMap),
	 surface(pipe)
	{
	}

template <class DataSetWrapperParam>
inline
ColoredIsosurface<DataSetWrapperParam>::~ColoredIsosurface(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
std::string
ColoredIsosurface<DataSetWrapperParam>::getName(
	void) const
	{
	return "Colored Isosurface";
	}

template <class DataSetWrapperParam>
inline
size_t
ColoredIsosurface<DataSetWrapperParam>::getSize(
	void) const
	{
	return surface.getNumTriangles();
	}

template <class DataSetWrapperParam>
inline
void
ColoredIsosurface<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Set up OpenGL state for colored isosurface rendering: */
	GLboolean cullFaceEnabled=glIsEnabled(GL_CULL_FACE);
	if(cullFaceEnabled)
		glDisable(GL_CULL_FACE);
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	GLboolean normalizeEnabled=glIsEnabled(GL_NORMALIZE);
	GLboolean lightModelTwoSide;
	glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE,&lightModelTwoSide);
	GLint lightModelColorControl;
	glGetIntegerv(GL_LIGHT_MODEL_COLOR_CONTROL,&lightModelColorControl);
	if(lighting)
		{
		if(!lightingEnabled)
			glEnable(GL_LIGHTING);
		if(!normalizeEnabled)
			glEnable(GL_NORMALIZE);
		if(!lightModelTwoSide)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		if(lightModelColorControl!=GL_SEPARATE_SPECULAR_COLOR)
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		}
	else
		{
		if(lightingEnabled)
			glDisable(GL_LIGHTING);
		if(normalizeEnabled)
			glDisable(GL_NORMALIZE);
		if(lightModelTwoSide)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		if(lightModelColorControl!=GL_SINGLE_COLOR)
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
		}
	GLboolean texture1DEnabled=glIsEnabled(GL_TEXTURE_1D);
	if(!texture1DEnabled)
		glEnable(GL_TEXTURE_1D);
	GLboolean texture2DEnabled=glIsEnabled(GL_TEXTURE_2D);
	if(texture2DEnabled)
		glDisable(GL_TEXTURE_2D);
	GLboolean texture3DEnabled=glIsEnabled(GL_TEXTURE_3D);
	if(texture3DEnabled)
		glDisable(GL_TEXTURE_3D);
	GLboolean colorMaterialEnabled=false; // Redundant initialization; just to make compiler happy
	GLMaterial frontMaterial,backMaterial;
	if(lighting)
		{
		colorMaterialEnabled=glIsEnabled(GL_COLOR_MATERIAL);
		if(colorMaterialEnabled)
			glDisable(GL_COLOR_MATERIAL);
		frontMaterial=glGetMaterial(GLMaterialEnums::FRONT);
		backMaterial=glGetMaterial(GLMaterialEnums::BACK);
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,GLMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(0.6f,0.6f,0.6f),25.0f));
		}
	
	/* Upload the color map as a 1D texture: */
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA8,256,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
	
	/* Set the texture environment to disable or enable lighting: */
	if(lighting)
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	else
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
	
	/* Render the surface representation: */
	surface.glRenderAction(contextData);
	
	/* Reset OpenGL state: */
	glPopMatrix();
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(matrixMode);
	if(lighting)
		{
		glMaterial(GLMaterialEnums::FRONT,frontMaterial);
		glMaterial(GLMaterialEnums::BACK,backMaterial);
		if(colorMaterialEnabled)
			glEnable(GL_COLOR_MATERIAL);
		}
	if(texture3DEnabled)
		glEnable(GL_TEXTURE_3D);
	if(texture2DEnabled)
		glEnable(GL_TEXTURE_2D);
	if(!texture1DEnabled)
		glDisable(GL_TEXTURE_1D);
	if(lighting)
		{
		if(lightModelColorControl!=GL_SEPARATE_SPECULAR_COLOR)
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,lightModelColorControl);
		if(!lightModelTwoSide)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		if(!normalizeEnabled)
			glDisable(GL_NORMALIZE);
		if(!lightingEnabled)
			glDisable(GL_LIGHTING);
		}
	else
		{
		if(lightModelColorControl!=GL_SINGLE_COLOR)
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,lightModelColorControl);
		if(lightModelTwoSide)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		if(normalizeEnabled)
			glEnable(GL_NORMALIZE);
		if(lightingEnabled)
			glEnable(GL_LIGHTING);
		}
	if(cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	}

}

}
