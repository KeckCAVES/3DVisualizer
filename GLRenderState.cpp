/***********************************************************************
GLRenderState - Class to track changes to OpenGL state during rendering
of 3D Visualizer's visualization elements.
Copyright (c) 2012 Oliver Kreylos

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

#include <GLRenderState.h>

#include <GL/GLLightTracker.h>
#include <GL/GLContextData.h>

/**************************************
Static elements of class GLRenderState:
**************************************/

const GLenum GLRenderState::textureLevelEnums[3]={GL_TEXTURE_1D,GL_TEXTURE_2D,GL_TEXTURE_3D};
const GLenum GLRenderState::matrixModeEnums[3]={GL_PROJECTION,GL_MODELVIEW,GL_TEXTURE};

/******************************
Methods of class GLRenderState:
******************************/

GLRenderState::GLRenderState(GLContextData& sContextData)
	:contextData(sContextData)
	{
	/* Get the light tracker: */
	GLLightTracker& lt=*(contextData.getLightTracker());
	
	/* Initialize render state from OpenGL context: */
	glGetFloatv(GL_POINT_SIZE,&savedPointSize);
	pointSize=savedPointSize;
	glGetFloatv(GL_LINE_WIDTH,&savedLineWidth);
	lineWidth=savedLineWidth;
	cullingEnabled=savedCullingEnabled=glIsEnabled(GL_CULL_FACE);
	glGetIntegerv(GL_CULL_FACE_MODE,&savedCulledFace);
	culledFace=savedCulledFace;
	lightingEnabled=lt.isLightingEnabled();
	twoSidedLightingEnabled=lt.isLightingTwoSided();
	colorMaterialEnabled=lt.isColorMaterials();
	colorMaterialFace=lt.getColorMaterialFace();
	colorMaterialProperty=lt.getColorMaterialProperty();
	
	/* Get enable states for the three texture levels: */
	for(int i=0;i<3;++i)
		savedTextureEnableds[i]=glIsEnabled(textureLevelEnums[i]);
	
	/* Remember the highest enabled texture level, and disable all lower levels: */
	int tli;
	for(tli=2;tli>=0&&!savedTextureEnableds[tli];--tli)
		;
	textureLevel=tli+1;
	for(--tli;tli>=0;--tli)
		if(savedTextureEnableds[tli])
			glDisable(textureLevelEnums[tli]);
	
	glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,&savedTextureMode);
	textureMode=savedTextureMode;
	separateSpecularColorEnabled=lt.isSpecularColorSeparate();
	
	/* Save the texture objects currently bound to the three texture levels: */
	glGetIntegerv(GL_TEXTURE_BINDING_1D,&(savedBoundTextures[0]));
	glGetIntegerv(GL_TEXTURE_BINDING_2D,&(savedBoundTextures[1]));
	glGetIntegerv(GL_TEXTURE_BINDING_3D,&(savedBoundTextures[2]));
	for(int i=0;i<3;++i)
		boundTextures[i]=savedBoundTextures[i];
	
	/* Save the current matrix mode and initialize the matrix version numbers: */
	glGetIntegerv(GL_MATRIX_MODE,&savedMatrixMode);
	for(matrixModeIndex=0;matrixModeIndex<3&&GLenum(savedMatrixMode)==matrixModeEnums[matrixModeIndex];++matrixModeIndex)
		;
	for(int i=0;i<3;++i)
		matrixVersions[i]=0;
	}

GLRenderState::~GLRenderState(void)
	{
	/* Get the light tracker: */
	GLLightTracker& lt=*(contextData.getLightTracker());
	
	/* Reset OpenGL context to original state: */
	setPointSize(savedPointSize);
	setLineWidth(savedLineWidth);
	if(savedCullingEnabled)
		enableCulling(savedCulledFace);
	else
		disableCulling();
	setLighting(lt.isLightingEnabled());
	setTwoSidedLighting(lt.isLightingTwoSided());
	if(lt.isColorMaterials())
		enableColorMaterial(lt.getColorMaterialFace(),lt.getColorMaterialProperty());
	else
		disableColorMaterial();
	setSeparateSpecularColor(lt.isSpecularColorSeparate());
	for(int i=0;i<3;++i)
		if(savedTextureEnableds[i])
			{
			if(textureLevel!=i+1)
				glEnable(textureLevelEnums[i]);
			}
		else
			{
			if(textureLevel==i+1)
				glDisable(textureLevelEnums[i]);
			}
	setTextureMode(savedTextureMode);
	setSeparateSpecularColor(lt.isSpecularColorSeparate());
	for(int i=0;i<3;++i)
		if(GLuint(savedBoundTextures[i])!=boundTextures[i])
			glBindTexture(textureLevelEnums[i],savedBoundTextures[i]);
	if(GLenum(savedMatrixMode)!=matrixModeEnums[matrixModeIndex])
		glMatrixMode(savedMatrixMode);
	}
