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

#ifndef GLRENDERSTATE_INCLUDED
#define GLRENDERSTATE_INCLUDED

#include <GL/gl.h>

/* Forward declarations: */
class GLContextData;

class GLRenderState
	{
	/* Elements: */
	private:
	GLContextData& contextData; // OpenGL context whose state is tracked
	
	/* Saved OpenGL context state: */
	static const GLenum textureLevelEnums[3];
	static const GLenum matrixModeEnums[3];
	GLfloat savedPointSize;
	GLfloat savedLineWidth;
	bool savedCullingEnabled;
	GLint savedCulledFace;
	bool savedTextureEnableds[3];
	GLint savedTextureMode;
	GLint savedBoundTextures[3];
	GLint savedMatrixMode;
	
	/* Elements shadowing current OpenGL state: */
	GLfloat pointSize;
	GLfloat lineWidth;
	bool cullingEnabled;
	GLenum culledFace;
	bool lightingEnabled;
	bool twoSidedLightingEnabled;
	bool colorMaterialEnabled;
	GLenum colorMaterialFace;
	GLenum colorMaterialProperty;
	int textureLevel;
	GLenum textureMode;
	bool separateSpecularColorEnabled;
	GLuint boundTextures[3];
	int matrixModeIndex; // Index of the currently selected OpenGL matrix (0: GL_PROJECTION, 1: GL_MODELVIEW, 2: GL_TEXTURE)
	unsigned int matrixVersions[3]; // Version numbers to track changes to the projection, modelview, and texture OpenGL matrices
	
	/* Constructors and destructors: */
	public:
	GLRenderState(GLContextData& sContextData); // Initializes render state from current state of given OpenGL context and saves OpenGL state
	~GLRenderState(void); // Resets OpenGL state to what it was when this object was created
	
	/* Methods: */
	GLContextData& getContextData(void) // Returns the OpenGL context
		{
		return contextData;
		}
	void setPointSize(GLfloat newPointSize) // Sets the point size
		{
		if(pointSize!=newPointSize)
			{
			pointSize=newPointSize;
			glPointSize(pointSize);
			}
		}
	void setLineWidth(GLfloat newLineWidth) // Sets the line width
		{
		if(lineWidth!=newLineWidth)
			{
			lineWidth=newLineWidth;
			glLineWidth(lineWidth);
			}
		}
	void enableCulling(GLenum newCulledFace) // Enables OpenGL face culling
		{
		if(!cullingEnabled)
			{
			cullingEnabled=true;
			glEnable(GL_CULL_FACE);
			}
		if(culledFace!=newCulledFace)
			{
			culledFace=newCulledFace;
			glCullFace(culledFace);
			}
		}
	void disableCulling(void) // Disables OpenGL face culling
		{
		if(cullingEnabled)
			{
			cullingEnabled=false;
			glDisable(GL_CULL_FACE);
			}
		}
	void setLighting(bool newLightingEnabled) // Enables or disables lighting
		{
		if(lightingEnabled!=newLightingEnabled)
			{
			lightingEnabled=newLightingEnabled;
			if(lightingEnabled)
				glEnable(GL_LIGHTING);
			else
				glDisable(GL_LIGHTING);
			}
		}
	void setTwoSidedLighting(bool newTwoSidedLightingEnabled)
		{
		if(twoSidedLightingEnabled!=newTwoSidedLightingEnabled)
			{
			twoSidedLightingEnabled=newTwoSidedLightingEnabled;
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,twoSidedLightingEnabled?GL_TRUE:GL_FALSE);
			}
		}
	void enableColorMaterial(GLenum newColorMaterialFace,GLenum newColorMaterialProperty) // Enables material color tracking for the given face and material components
		{
		if(colorMaterialFace!=newColorMaterialFace||colorMaterialProperty!=newColorMaterialProperty)
			{
			colorMaterialFace=newColorMaterialFace;
			colorMaterialProperty=newColorMaterialProperty;
			glColorMaterial(colorMaterialFace,colorMaterialProperty);
			}
		if(!colorMaterialEnabled)
			{
			colorMaterialEnabled=true;
			glEnable(GL_COLOR_MATERIAL);
			}
		}
	void disableColorMaterial(void) // Disables material color tracking
		{
		if(colorMaterialEnabled)
			{
			colorMaterialEnabled=false;
			glDisable(GL_COLOR_MATERIAL);
			}
		}
	void setTextureLevel(int newTextureLevel) // Enables the given texture level (0: disabled, 1: 1D, 2: 2D, 3: 3D)
		{
		if(textureLevel!=newTextureLevel)
			{
			if(textureLevel>0)
				glDisable(textureLevelEnums[textureLevel-1]);
			textureLevel=newTextureLevel;
			if(textureLevel>0)
				glEnable(textureLevelEnums[textureLevel-1]);
			}
		}
	void setTextureMode(GLenum newTextureMode) // Sets the texture mapping mode for the currently enabled texture level
		{
		if(textureMode!=newTextureMode)
			{
			textureMode=newTextureMode;
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,textureMode);
			}
		}
	void setSeparateSpecularColor(bool newSeparateSpecularColorEnabled) // Enables or disables separate handling of specular color
		{
		if(separateSpecularColorEnabled!=newSeparateSpecularColorEnabled)
			{
			separateSpecularColorEnabled=newSeparateSpecularColorEnabled;
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,separateSpecularColorEnabled?GL_SEPARATE_SPECULAR_COLOR:GL_SINGLE_COLOR);
			}
		}
	void bindTexture(GLuint newBoundTexture) // Binds the given texture object to the currently activated texture level
		{
		if(textureLevel>0&&boundTextures[textureLevel-1]!=newBoundTexture)
			{
			boundTextures[textureLevel-1]=newBoundTexture;
			glBindTexture(textureLevelEnums[textureLevel-1],boundTextures[textureLevel-1]);
			}
		}
	void setMatrixMode(int newMatrixModeIndex) // Selects the OpenGL matrix to which following matrix operations are applied
		{
		if(matrixModeIndex!=newMatrixModeIndex)
			{
			matrixModeIndex=newMatrixModeIndex;
			glMatrixMode(matrixModeEnums[matrixModeIndex]);
			}
		}
	void updateMatrix(void) // Signals that the currently selected OpenGL matrix has changed state
		{
		++matrixVersions[matrixModeIndex];
		}
	unsigned int getMatrixVersion(void) const // Returns the version number of the currently selected OpenGL matrix
		{
		return matrixVersions[matrixModeIndex];
		}
	};

#endif
