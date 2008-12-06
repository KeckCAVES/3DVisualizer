/***********************************************************************
PaletteRenderer - Class for texture-based volume renderers using
palette-based transfer functions
Copyright (c) 2001-2006 Oliver Kreylos

This file is part of the 3D Data Visualization Library (3DVis).

The 3D Data Visualization Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The 3D Data Visualization Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the 3D Data Visualization Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBFragmentProgram.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLEXTPalettedTexture.h>
#include <GL/Extensions/GLEXTTexture3D.h>
#include <GL/Extensions/GLNVTextureShader.h>
#include <GLTextures.h>

#include "PaletteRenderer.h"

/*************************************
Fragment programs for PaletteRenderer:
*************************************/

/* 2D texture mapping, REPLACE mode: */
static const char* fp1="!!ARBfp1.0\n\
ATTRIB tex = fragment.texcoord;\n\
OUTPUT outCol = result.color;\n\
TEMP temp;\n\
TXP temp, tex, texture[0], 2D;\n\
TEX outCol, temp, texture[1], 2D;\n\
END";

/* 2D texture mapping, MODULATE mode: */
static const char* fp2="!!ARBfp1.0\n\
ATTRIB tex = fragment.texcoord;\n\
ATTRIB col = fragment.color.primary;\n\
OUTPUT outCol = result.color;\n\
TEMP temp;\n\
TXP temp, tex, texture[0], 2D;\n\
TEX temp, temp, texture[1], 2D;\n\
MUL outCol, temp, col;\n\
END";

/* 3D texture mapping, REPLACE mode: */
static const char* fp3="!!ARBfp1.0\n\
ATTRIB tex = fragment.texcoord;\n\
OUTPUT outCol = result.color;\n\
TEMP temp;\n\
TXP temp, tex, texture[0], 3D;\n\
TEX outCol, temp, texture[1], 2D;\n\
END";

/* 3D texture mapping, MODULATE mode: */
static const char* fp4="!!ARBfp1.0\n\
ATTRIB tex = fragment.texcoord;\n\
ATTRIB col = fragment.color.primary;\n\
OUTPUT outCol = result.color;\n\
TEMP temp1, temp2;\n\
TXP temp1, tex, texture[0], 3D;\n\
TEX temp2, temp1, texture[1], 2D;\n\
MUL outCol, temp2, col;\n\
END";

/******************************************
Methods of class PaletteRenderer::DataItem:
******************************************/

PaletteRenderer::DataItem::DataItem(void)
	:fragmentProgramId(0),paletteTextureObjectId(0),cachedColorMapVersion(0),uploadColorMap(false)
	{
	GLARBMultitexture::initExtension();
	
	/* Determine the optimal rendering path: */
	if(GLARBFragmentProgram::isSupported())
		{
		renderingPath=FragmentProgram;
		
		/* Initialize the extension: */
		GLARBFragmentProgram::initExtension();
		
		/* Create fragment programs: */
		glGenProgramsARB(1,&fragmentProgramId);
		}
	else if(GLARBMultitexture::isSupported()&&GLNVTextureShader::isSupported())
		{
		renderingPath=TextureShader;
		
		/* Initialize the extensions: */
		GLARBMultitexture::initExtension();
		GLNVTextureShader::initExtension();
		}
	else if(GLEXTPalettedTexture::isSupported())
		{
		renderingPath=PalettedTexture;
		
		/* Initialize the extension: */
		GLEXTPalettedTexture::initExtension();
		}
	
	/* Create a texture object for the palette texture if the palette renderer is in fragment program or texture shader mode: */
	if(renderingPath==FragmentProgram||renderingPath==TextureShader)
		glGenTextures(1,&paletteTextureObjectId);
	}

PaletteRenderer::DataItem::~DataItem(void)
	{
	if(renderingPath==FragmentProgram)
		glDeleteProgramsARB(1,&fragmentProgramId);
	if(renderingPath==FragmentProgram||renderingPath==TextureShader)
		glDeleteTextures(1,&paletteTextureObjectId);
	}

void PaletteRenderer::DataItem::updateTextureCache(const VolumeRenderer* renderer,int majorAxis)
	{
	/* Call the base class method: */
	VolumeRenderer::DataItem::updateTextureCache(renderer,majorAxis);
	
	if(renderingPath==FragmentProgram)
		{
		if(setParameters)
			{
			/* Upload the appropriate fragment program: */
			if(has3DTextures&&renderer->getRenderingMode()==VIEW_PERPENDICULAR)
				{
				if(renderer->getTextureFunction()==REPLACE)
					glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fp3),fp3);
				else
					glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fp4),fp4);
				}
			else
				{
				if(renderer->getTextureFunction()==REPLACE)
					glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fp1),fp1);
				else
					glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fp2),fp2);
				}
			if(glGetError()==GL_INVALID_OPERATION)
				{
				GLint errorPos;
				glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB,&errorPos);
				const GLubyte* error=glGetString(GL_PROGRAM_ERROR_STRING_ARB);
				Misc::throwStdErr("Fragment program error \"%s\" at position %d",error,errorPos);
				}
			}
		}
	else if(renderingPath==PalettedTexture)
		{
		/* Check if the color map needs to be uploaded: */
		const PaletteRenderer* paletteRenderer=static_cast<const PaletteRenderer*>(renderer);
		
		/*************************************************************************
		Another bug in SGI's OpenGL implementation: Though color maps are treated
		as a texture object resource, they are not always installed when a texture
		object is bound. Thus, we have to upload them manually everytime we bind;
		painful and sloow.
		*************************************************************************/
		
		uploadColorMap=false;
		#ifndef __SGI_IRIX__
		if(uploadData||cachedColorMapVersion!=paletteRenderer->colorMapVersion)
		#endif
			{
			cachedColorMapVersion=paletteRenderer->colorMapVersion;
			textureCacheValid=false;
			uploadColorMap=!paletteRenderer->sharePalette;
			}
		}
	}

void PaletteRenderer::DataItem::deleteTextureCache(void)
	{
	uploadColorMap=false;
	
	/* Call the base class method: */
	VolumeRenderer::DataItem::deleteTextureCache();
	}

/********************************
Methods of class PaletteRenderer:
********************************/

void PaletteRenderer::uploadTexture2D(VolumeRenderer::DataItem* dataItem,int axis,int index) const
	{
	/* Get pointer to our own data item: */
	DataItem* myDataItem=static_cast<DataItem*>(dataItem);
	
	if(myDataItem->setParameters)
		{
		/* Set the OpenGL texturing parameters: */
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolationMode);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolationMode);
		}
	
	/* Upload a color map only if necessary: */
	if(myDataItem->renderingPath==PalettedTexture&&myDataItem->uploadColorMap)
		{
		/* Set the texture's color map: */
		#ifdef __SGI_IRIX__
		glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#else
		glColorTableEXT(GL_TEXTURE_2D,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#endif
		}
	
	if(myDataItem->uploadData)
		{
		/* Determine the texture's format: */
		GLenum internalFormat=GL_INTENSITY8;
		GLenum uploadFormat=GL_LUMINANCE;
		
		#ifndef __SGI_IRIX__
		if(myDataItem->renderingPath==PalettedTexture)
			{
			internalFormat=GL_COLOR_INDEX8_EXT;
			uploadFormat=GL_COLOR_INDEX;
			}
		#endif
		
		/* Upload a texture slice: */
		const Voxel* slicePtr=values+index*increments[axis];
		switch(axis)
			{
			case 0:
				glTexImage2D(GL_TEXTURE_2D,0,internalFormat,textureSize[2],textureSize[1],0,uploadFormat,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[2],size[1],increments[2],increments[1],uploadFormat,GL_UNSIGNED_BYTE,slicePtr);
				break;
			
			case 1:
				glTexImage2D(GL_TEXTURE_2D,0,internalFormat,textureSize[2],textureSize[0],0,uploadFormat,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[2],size[0],increments[2],increments[0],uploadFormat,GL_UNSIGNED_BYTE,slicePtr);
				break;
			
			case 2:
				glTexImage2D(GL_TEXTURE_2D,0,internalFormat,textureSize[1],textureSize[0],0,uploadFormat,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[1],size[0],increments[1],increments[0],uploadFormat,GL_UNSIGNED_BYTE,slicePtr);
				break;
			}
		}
	}

void PaletteRenderer::uploadTexture3D(VolumeRenderer::DataItem* dataItem) const
	{
	/* Get pointer to our own data item: */
	DataItem* myDataItem=static_cast<DataItem*>(dataItem);
	
	if(myDataItem->setParameters)
		{
		/* Set the OpenGL texturing parameters: */
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,interpolationMode);
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,interpolationMode);
		}
	
	/* Upload a color map only if necessary: */
	if(myDataItem->renderingPath==PalettedTexture&&myDataItem->uploadColorMap)
		{
		/* Set the texture's color map: */
		#ifdef __SGI_IRIX__
		glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#else
		glColorTableEXT(GL_TEXTURE_3D,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#endif
		}
	
	if(myDataItem->uploadData)
		{
		/* Determine the texture's format: */
		GLenum internalFormat=GL_INTENSITY8;
		GLenum uploadFormat=GL_LUMINANCE;
		
		#ifndef __SGI_IRIX__
		if(myDataItem->renderingPath==PalettedTexture)
			{
			internalFormat=GL_COLOR_INDEX8_EXT;
			uploadFormat=GL_COLOR_INDEX;
			}
		#endif
		
		/* Upload the texture block: */
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0); // increments[1]); // Seems to be a bug in OpenGL - consistent across SGI/nVidia platforms
		glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,0); // increments[0]);
		glPixelStorei(GL_UNPACK_SKIP_IMAGES,0);
		#ifdef __SGI_IRIX__
		glTexImage3D(GL_TEXTURE_3D,0,internalFormat,textureSize[2],textureSize[1],textureSize[0],0,uploadFormat,GL_UNSIGNED_BYTE,values);
		#else
		glTexImage3DEXT(GL_TEXTURE_3D,0,internalFormat,textureSize[2],textureSize[1],textureSize[0],0,uploadFormat,GL_UNSIGNED_BYTE,0);
		glTexSubImage3DEXT(GL_TEXTURE_3D,0,0,0,0,size[2],size[1],size[0],uploadFormat,GL_UNSIGNED_BYTE,values);
		#endif
		}
	}

void PaletteRenderer::prepareRenderAxisAligned(VolumeRenderer::DataItem* dataItem) const
	{
	DataItem* myDataItem=static_cast<DataItem*>(dataItem);
	
	/* Manage color palette uploads: */
	if(myDataItem->renderingPath==FragmentProgram)
		{
		/* Bind the color map texture to texture unit 1: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,myDataItem->paletteTextureObjectId);
		if(myDataItem->cachedColorMapVersion!=colorMapVersion)
			{
			/* Upload the color map as a 2D texture: */
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,1,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
			
			/* Mark the cached color map as up-to-date: */
			myDataItem->cachedColorMapVersion=colorMapVersion;
			}
		glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	else if(myDataItem->renderingPath==TextureShader)
		{
		/* Bind the color map texture to texture unit 1: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,myDataItem->paletteTextureObjectId);
		if(myDataItem->cachedColorMapVersion!=colorMapVersion)
			{
			/* Upload the color map as a 2D texture: */
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,1,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
			
			/* Mark the cached color map as up-to-date: */
			myDataItem->cachedColorMapVersion=colorMapVersion;
			}
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_SHADER_OPERATION_NV,GL_DEPENDENT_AR_TEXTURE_2D_NV);
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_PREVIOUS_TEXTURE_INPUT_NV,GL_TEXTURE0_ARB);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,textureFunction);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	else if(!textureCachingEnabled)
		{
		/* Sufficient to upload palette right here: */
		#ifdef __SGI_IRIX__
		glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#else
		glColorTableEXT(GL_TEXTURE_2D,GL_RGBA,256,GL_RGBA,GL_FLOAT,colorMap->getColors());
		#endif
		}
	}

void PaletteRenderer::prepareRenderViewPerpendicular(VolumeRenderer::DataItem* dataItem) const
	{
	DataItem* myDataItem=static_cast<DataItem*>(dataItem);
	
	/* Manage color palette uploads: */
	if(myDataItem->renderingPath==FragmentProgram)
		{
		/* Bind the color map texture to texture unit 1: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,myDataItem->paletteTextureObjectId);
		if(myDataItem->cachedColorMapVersion!=colorMapVersion)
			{
			/* Upload the color map as a 2D texture: */
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,1,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
			
			/* Mark the cached color map as up-to-date: */
			myDataItem->cachedColorMapVersion=colorMapVersion;
			}
		glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	else if(myDataItem->renderingPath==TextureShader)
		{
		/* Bind the color map texture to texture unit 1: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,myDataItem->paletteTextureObjectId);
		if(myDataItem->cachedColorMapVersion!=colorMapVersion)
			{
			/* Upload the color map as a 2D texture: */
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,1,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
			
			/* Mark the cached color map as up-to-date: */
			myDataItem->cachedColorMapVersion=colorMapVersion;
			}
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_SHADER_OPERATION_NV,GL_DEPENDENT_AR_TEXTURE_2D_NV);
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_PREVIOUS_TEXTURE_INPUT_NV,GL_TEXTURE0_ARB);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,textureFunction);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	}

void PaletteRenderer::initContext(GLContextData& contextData) const
	{
	contextData.addDataItem(this,new DataItem);
	}

void PaletteRenderer::setGLState(GLContextData& contextData) const
	{
	/* Get pointer to data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Call base class method: */ 
	VolumeRenderer::setGLState(contextData);
	
	if(dataItem->renderingPath==FragmentProgram)
		{
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,dataItem->fragmentProgramId);
		}
	else if(dataItem->renderingPath==TextureShader)
		{
		/* Enable texture shaders: */
		glEnable(GL_TEXTURE_SHADER_NV);
		
		/* Set up the texture shader pipeline: */
		
		/* Texture unit 0 gets density data from the 2D/3D data texture: */
		glActiveTextureARB(GL_TEXTURE0_ARB);
		switch(renderingMode)
			{
			case AXIS_ALIGNED:
				glTexEnvi(GL_TEXTURE_SHADER_NV,GL_SHADER_OPERATION_NV,GL_TEXTURE_2D);
				break;
			
			case VIEW_PERPENDICULAR:
				glTexEnvi(GL_TEXTURE_SHADER_NV,GL_SHADER_OPERATION_NV,GL_TEXTURE_3D);
				break;
			}
		
		/* Texture unit 1 performs transfer function look-up using a dependent texture: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_SHADER_OPERATION_NV,GL_DEPENDENT_AR_TEXTURE_2D_NV);
		glTexEnvi(GL_TEXTURE_SHADER_NV,GL_PREVIOUS_TEXTURE_INPUT_NV,GL_TEXTURE0_ARB);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,textureFunction);
		
		/* Go back to initial texture unit: */
		glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	else
		{
		#ifdef __SGI_IRIX__
		glEnable(GL_TEXTURE_COLOR_TABLE_SGI);
		#endif
		if(sharePalette)
			glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
		}
	}

void PaletteRenderer::resetGLState(GLContextData& contextData) const
	{
	/* Get pointer to data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	if(dataItem->renderingPath==FragmentProgram)
		{
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		}
	else if(dataItem->renderingPath==TextureShader)
		glDisable(GL_TEXTURE_SHADER_NV);
	else
		{
		#ifdef __SGI_IRIX__
		glDisable(GL_TEXTURE_COLOR_TABLE_SGI);
		#endif
		if(sharePalette)
			glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
		}
	
	/* Call base class method: */ 
	VolumeRenderer::resetGLState(contextData);
	}

void PaletteRenderer::setColorMap(const GLColorMap* newColorMap)
	{
	if(newColorMap->getNumEntries()==256) // Only 8-bit palettes!
		{
		++colorMapVersion;
		colorMap=newColorMap;
		}
	}

void PaletteRenderer::setSharePalette(bool newSharePalette)
	{
	sharePalette=newSharePalette;
	}

void PaletteRenderer::setGlobalColorMap(const GLColorMap* newGlobalColorMap)
	{
	if(newGlobalColorMap->getNumEntries()==256)
		{
		/* Sufficient to upload palette right here: */
		#ifdef __SGI_IRIX__
		glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI,GL_RGBA,256,GL_RGBA,GL_FLOAT,newGlobalColorMap->getColors());
		#else
		glColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT,GL_RGBA,256,GL_RGBA,GL_FLOAT,newGlobalColorMap->getColors());
		#endif
		}
	}
