/***********************************************************************
Isosurface - Wrapper class for isosurfaces as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2005-2009 Oliver Kreylos

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

#include <Wrappers/Isosurface.h>

namespace Visualization {

namespace Wrappers {

/***************************
Methods of class Isosurface:
***************************/

template <class DataSetWrapperParam>
inline
Isosurface<DataSetWrapperParam>::Isosurface(
	Visualization::Abstract::Parameters* sParameters,
	typename Isosurface<DataSetWrapperParam>::VScalar sIsovalue,
	const GLColorMap* sColorMap,
	Comm::MulticastPipe* pipe)
	:Visualization::Abstract::Element(sParameters),
	 isovalue(sIsovalue),
	 colorMap(sColorMap),
	 surface(pipe)
	{
	}

template <class DataSetWrapperParam>
inline
Isosurface<DataSetWrapperParam>::~Isosurface(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
std::string
Isosurface<DataSetWrapperParam>::getName(
	void) const
	{
	return "Isosurface";
	}

template <class DataSetWrapperParam>
inline
size_t
Isosurface<DataSetWrapperParam>::getSize(
	void) const
	{
	return surface.getNumTriangles();
	}

template <class DataSetWrapperParam>
inline
void
Isosurface<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Set up OpenGL state for isosurface rendering: */
	GLboolean cullFaceEnabled=glIsEnabled(GL_CULL_FACE);
	if(cullFaceEnabled)
		glDisable(GL_CULL_FACE);
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(!lightingEnabled)
		glEnable(GL_LIGHTING);
	GLboolean normalizeEnabled=glIsEnabled(GL_NORMALIZE);
	if(!normalizeEnabled)
		glEnable(GL_NORMALIZE);
	GLboolean lightModelTwoSide;
	glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE,&lightModelTwoSide);
	if(!lightModelTwoSide)
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	GLboolean colorMaterialEnabled=glIsEnabled(GL_COLOR_MATERIAL);
	if(colorMaterialEnabled)
		glDisable(GL_COLOR_MATERIAL);
	GLMaterial frontMaterial=glGetMaterial(GLMaterialEnums::FRONT);
	GLMaterial backMaterial=glGetMaterial(GLMaterialEnums::BACK);
	GLMaterial::Color surfaceColor=(*colorMap)(isovalue);
	glMaterial(GLMaterialEnums::FRONT_AND_BACK,GLMaterial(surfaceColor,GLMaterial::Color(0.6f,0.6f,0.6f),25.0f));
	
	/* Render the surface representation: */
	surface.glRenderAction(contextData);
	
	/* Reset OpenGL state: */
	glMaterial(GLMaterialEnums::FRONT,frontMaterial);
	glMaterial(GLMaterialEnums::BACK,backMaterial);
	if(colorMaterialEnabled)
		glEnable(GL_COLOR_MATERIAL);
	if(!lightModelTwoSide)
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	if(!normalizeEnabled)
		glDisable(GL_NORMALIZE);
	if(!lightingEnabled)
		glDisable(GL_LIGHTING);
	if(cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	}

}

}
