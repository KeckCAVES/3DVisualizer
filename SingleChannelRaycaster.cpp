/***********************************************************************
SingleChannelRaycaster - Class for volume renderers with a single scalar
channel.
Copyright (c) 2007-2010 Oliver Kreylos

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

#include <SingleChannelRaycaster.h>

#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBTextureFloat.h>
#include <GL/Extensions/GLEXTTexture3D.h>
#include <GL/GLShader.h>

/*************************************************
Methods of class SingleChannelRaycaster::DataItem:
*************************************************/

SingleChannelRaycaster::DataItem::DataItem(void)
	:haveFloatTextures(GLARBTextureFloat::isSupported()),
	 volumeTextureID(0),volumeTextureVersion(0),
	 colorMapTextureID(0),
	 volumeSamplerLoc(-1),colorMapSamplerLoc(-1)
	{
	/* Initialize all required OpenGL extensions: */
	GLARBMultitexture::initExtension();
	if(haveFloatTextures)
		GLARBTextureFloat::initExtension();
	GLEXTTexture3D::initExtension();

	/* Create the volume texture object: */
	glGenTextures(1,&volumeTextureID);
	
	/* Create the color map texture object: */
	glGenTextures(1,&colorMapTextureID);
	}

SingleChannelRaycaster::DataItem::~DataItem(void)
	{
	/* Destroy the volume texture object: */
	glDeleteTextures(1,&volumeTextureID);
	
	/* Destroy the color map texture object: */
	glDeleteTextures(1,&colorMapTextureID);
	}

/***************************************
Methods of class SingleChannelRaycaster:
***************************************/

void SingleChannelRaycaster::initDataItem(Raycaster::DataItem* dataItem) const
	{
	/* Call the base class method: */
	Raycaster::initDataItem(dataItem);
	
	/* Get a pointer to the data item: */
	DataItem* myDataItem=dynamic_cast<DataItem*>(dataItem);
	
	/* Create the data volume texture: */
	glBindTexture(GL_TEXTURE_3D,myDataItem->volumeTextureID);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP);
	glTexImage3DEXT(GL_TEXTURE_3D,0,GL_INTENSITY,myDataItem->textureSize[0],myDataItem->textureSize[1],myDataItem->textureSize[2],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
	glBindTexture(GL_TEXTURE_3D,0);
	
	/* Create the color map texture: */
	glBindTexture(GL_TEXTURE_1D,myDataItem->colorMapTextureID);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_1D,0);
	}

void SingleChannelRaycaster::initShader(Raycaster::DataItem* dataItem) const
	{
	/* Call the base class method: */
	Raycaster::initShader(dataItem);
	
	/* Get a pointer to the data item: */
	DataItem* myDataItem=dynamic_cast<DataItem*>(dataItem);
	
	/* Get the shader's uniform locations: */
	myDataItem->volumeSamplerLoc=myDataItem->shader.getUniformLocation("volumeSampler");
	myDataItem->colorMapSamplerLoc=myDataItem->shader.getUniformLocation("colorMapSampler");
	}

void SingleChannelRaycaster::bindShader(const Raycaster::PTransform& pmv,const Raycaster::PTransform& mv,Raycaster::DataItem* dataItem) const
	{
	/* Call the base class method: */
	Raycaster::bindShader(pmv,mv,dataItem);
	
	/* Get a pointer to the data item: */
	DataItem* myDataItem=dynamic_cast<DataItem*>(dataItem);
	
	/* Bind the volume texture: */
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_3D,myDataItem->volumeTextureID);
	glUniform1iARB(myDataItem->volumeSamplerLoc,1);
	
	/* Check if the volume texture needs to be updated: */
	if(myDataItem->volumeTextureVersion!=dataVersion)
		{
		/* Upload the new volume data: */
		glTexSubImage3DEXT(GL_TEXTURE_3D,0,0,0,0,dataSize[0],dataSize[1],dataSize[2],GL_LUMINANCE,GL_UNSIGNED_BYTE,data);
		
		/* Mark the volume texture as up-to-date: */
		myDataItem->volumeTextureVersion=dataVersion;
		}
	
	/* Bind the color map texture: */
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_1D,myDataItem->colorMapTextureID);
	glUniform1iARB(myDataItem->colorMapSamplerLoc,2);
	
	/* Create the stepsize-adjusted colormap with pre-multiplied alpha: */
	GLColorMap adjustedColorMap(*colorMap);
	adjustedColorMap.changeTransparency(stepSize*transparencyGamma);
	adjustedColorMap.premultiplyAlpha();
	glTexImage1D(GL_TEXTURE_1D,0,myDataItem->haveFloatTextures?GL_RGBA32F_ARB:GL_RGBA,256,0,GL_RGBA,GL_FLOAT,adjustedColorMap.getColors());
	}

void SingleChannelRaycaster::unbindShader(Raycaster::DataItem* dataItem) const
	{
	/* Unbind the color map texture: */
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_1D,0);
	
	/* Bind the volume texture: */
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_3D,0);
	
	/* Call the base class method: */
	Raycaster::unbindShader(dataItem);
	}

SingleChannelRaycaster::SingleChannelRaycaster(const unsigned int sDataSize[3],const Raycaster::Box& sDomain)
	:Raycaster(sDataSize,sDomain),
	 data(new Voxel[dataSize[0]*dataSize[1]*dataSize[2]]),dataVersion(0),
	 colorMap(0),transparencyGamma(1.0f)
	{
	}

SingleChannelRaycaster::~SingleChannelRaycaster(void)
	{
	/* Delete the volume dataset: */
	delete[] data;
	}

void SingleChannelRaycaster::initContext(GLContextData& contextData) const
	{
	/* Create a new data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Initialize the data item: */
	initDataItem(dataItem);
	
	try
		{
		/* Load and compile the vertex program: */
		std::string vertexShaderName=VISUALIZER_SHADERDIR;
		vertexShaderName.append("/SingleChannelRaycaster.vs");
		dataItem->shader.compileVertexShader(vertexShaderName.c_str());
		std::string fragmentShaderName=VISUALIZER_SHADERDIR;
		fragmentShaderName.append("/SingleChannelRaycaster.fs");
		dataItem->shader.compileFragmentShader(fragmentShaderName.c_str());
		dataItem->shader.linkShader();
		
		/* Initialize the raycasting shader: */
		initShader(dataItem);
		}
	catch(std::runtime_error err)
		{
		/* Print an error message, but continue: */
		std::cerr<<"SingleChannelRaycaster::initContext: Caught exception "<<err.what()<<std::endl;
		}
	}

void SingleChannelRaycaster::setStepSize(Raycaster::Scalar newStepSize)
	{
	/* Call the base class method: */
	Raycaster::setStepSize(newStepSize);
	}

void SingleChannelRaycaster::updateData(void)
	{
	/* Bump up the data version number: */
	++dataVersion;
	}

void SingleChannelRaycaster::setColorMap(const GLColorMap* newColorMap)
	{
	colorMap=newColorMap;
	}

void SingleChannelRaycaster::setTransparencyGamma(GLfloat newTransparencyGamma)
	{
	transparencyGamma=newTransparencyGamma;
	}
