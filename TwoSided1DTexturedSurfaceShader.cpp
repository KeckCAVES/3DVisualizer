/***********************************************************************
TwoSided1DTexturedSurfaceShader - Class to simulate OpenGL two-sided
lighting with a 1D color texture without the ridiculous and arbitrary
performance penalty it incurs on newer Nvidia Geforce graphics cards
(shame on you, Nvidia!).
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

#include <TwoSided1DTexturedSurfaceShader.h>

#include <Misc/PrintInteger.h>
#include <GL/gl.h>
#include <GL/GLLightTracker.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>

/**********************************************************
Methods of class TwoSided1DTexturedSurfaceShader::DataItem:
**********************************************************/

TwoSided1DTexturedSurfaceShader::DataItem::DataItem(void)
	:lightTrackerVersion(0),clipPlaneTrackerVersion(0),
	 colorTextureLocation(-1)
	{
	}

TwoSided1DTexturedSurfaceShader::DataItem::~DataItem(void)
	{
	}

void TwoSided1DTexturedSurfaceShader::DataItem::buildShader(GLContextData& contextData)
	{
	GLLightTracker* lt=contextData.getLightTracker();
	GLClipPlaneTracker* cpt=contextData.getClipPlaneTracker();
	
	/* Reset the shader: */
	shader.reset();
	
	/* Start the vertex shader's main function: */
	std::string vertexShaderMain="\
		varying vec3 frontAmbientDiffuseColor;\n\
		varying vec3 frontSpecularColor;\n\
		varying vec3 backAmbientDiffuseColor;\n\
		varying vec3 backSpecularColor;\n\
		varying float colorTextureCoord;\n\
		\n\
		void main()\n\
			{\n\
			/* Compute the vertex position and normal vector in eye space: */\n\
			vec4 vertexEc=gl_ModelViewMatrix*gl_Vertex;\n\
			vec3 normalEc=normalize(gl_NormalMatrix*gl_Normal);\n\
			\n\
			/* Initialize the front color accumulators: */\n\
			vec4 frontAmbientDiffuseAccumulator=gl_LightModel.ambient*gl_FrontMaterial.ambient;\n\
			vec4 frontSpecularAccumulator=vec4(0.0,0.0,0.0,0.0);\n\
			\n";
	
	/* Call the appropriate light accumulation function for every enabled light source: */
	std::string vertexShaderFunctions;
	for(int lightIndex=0;lightIndex<lt->getMaxNumLights();++lightIndex)
		if(lt->getLightState(lightIndex).isEnabled())
			{
			/* Create the light accumulation function: */
			vertexShaderFunctions+=lt->createAccumulateLightFunction(lightIndex);
			
			/* Call the light accumulation function from the vertex shader's main function: */
			vertexShaderMain+="\
				accumulateLight";
			char liBuffer[12];
			vertexShaderMain.append(Misc::print(lightIndex,liBuffer+11));
			vertexShaderMain+="(vertexEc,normalEc,gl_FrontMaterial.ambient,gl_FrontMaterial.diffuse,gl_FrontMaterial.specular,gl_FrontMaterial.shininess,frontAmbientDiffuseAccumulator,frontSpecularAccumulator);\n";
			}
	
	/* Continue the vertex shader's main function: */
	vertexShaderMain+="\
			\n\
			/* Assign the final front ambient+diffuse and specular colors: */\n\
			frontAmbientDiffuseColor=frontAmbientDiffuseAccumulator.xyz;\n\
			frontSpecularColor=frontSpecularAccumulator.xyz;\n\
			\n\
			/* Flip the normal vector to calculate back-face illumination: */\n\
			normalEc=-normalEc;\n\
			\n\
			/* Initialize the back color accumulators: */\n\
			vec4 backAmbientDiffuseAccumulator=gl_LightModel.ambient*gl_BackMaterial.ambient;\n\
			vec4 backSpecularAccumulator=vec4(0.0,0.0,0.0,0.0);\n\
			\n";
	
	/* Call the appropriate light accumulation function for every enabled light source: */
	for(int lightIndex=0;lightIndex<lt->getMaxNumLights();++lightIndex)
		if(lt->getLightState(lightIndex).isEnabled())
			{
			/* Call the previously created light accumulation function from the vertex shader's main function: */
			vertexShaderMain+="\
				accumulateLight";
			char liBuffer[12];
			vertexShaderMain.append(Misc::print(lightIndex,liBuffer+11));
			vertexShaderMain+="(vertexEc,normalEc,gl_BackMaterial.ambient,gl_BackMaterial.diffuse,gl_BackMaterial.specular,gl_BackMaterial.shininess,backAmbientDiffuseAccumulator,backSpecularAccumulator);\n";
			}
	
	/* Continue the vertex shader's main function: */
	vertexShaderMain+="\
			\n";
	
	/* Insert code to calculate the vertex' position relative to all user-specified clipping planes: */
	vertexShaderMain+=cpt->createCalcClipDistances("vertexEc");
	
	/* Finish the vertex shader's main function: */
	vertexShaderMain+="\
			\n\
			/* Assign the final back ambient+diffuse and specular colors: */\n\
			backAmbientDiffuseColor=backAmbientDiffuseAccumulator.xyz;\n\
			backSpecularColor=backSpecularAccumulator.xyz;\n\
			\n\
			/* Calculate the 1D texture coordinate: */\n\
			colorTextureCoord=(gl_TextureMatrix[0]*gl_MultiTexCoord0).x;\n\
			\n\
			/* Use standard vertex position: */\n\
			gl_Position=ftransform();\n\
			}\n";
	
	/* Assemble the full vertex shader source: */
	std::string vertexShader=vertexShaderFunctions+vertexShaderMain;
	
	/* Compile the vertex shader: */
	shader.compileVertexShaderFromString(vertexShader.c_str());
	
	/* Assemble the fragment shader's source: */
	std::string fragmentShaderMain="\
		uniform sampler1D colorTexture;\n\
		\n\
		varying vec3 frontAmbientDiffuseColor;\n\
		varying vec3 frontSpecularColor;\n\
		varying vec3 backAmbientDiffuseColor;\n\
		varying vec3 backSpecularColor;\n\
		varying float colorTextureCoord;\n\
		\n\
		void main()\n\
			{\n\
			vec3 texColor=texture1D(colorTexture,colorTextureCoord).xyz;\n\
			if(gl_FrontFacing)\n\
				gl_FragColor=vec4(frontAmbientDiffuseColor*texColor+frontSpecularColor,1.0);\n\
			else\n\
				gl_FragColor=vec4(backAmbientDiffuseColor*texColor+backSpecularColor,1.0);\n\
			}\n";
	
	/* Compile the fragment shader: */
	shader.compileFragmentShaderFromString(fragmentShaderMain.c_str());
	
	/* Link the shader: */
	shader.linkShader();
	
	/* Query the color texture sampler location: */
	colorTextureLocation=shader.getUniformLocation("colorTexture");
	}

/********************************************************
Static elements of class TwoSided1DTexturedSurfaceShader:
********************************************************/

Threads::Spinlock TwoSided1DTexturedSurfaceShader::theShaderMutex;
unsigned int TwoSided1DTexturedSurfaceShader::theShaderRefCount(0);
TwoSided1DTexturedSurfaceShader* TwoSided1DTexturedSurfaceShader::theShader=0;

/************************************************
Methods of class TwoSided1DTexturedSurfaceShader:
************************************************/

bool TwoSided1DTexturedSurfaceShader::isSupported(GLContextData& contextData)
	{
	/* Return true if shaders are supported: */
	return GLShader::isSupported();
	}

TwoSided1DTexturedSurfaceShader* TwoSided1DTexturedSurfaceShader::acquireShader(void)
	{
	Threads::Spinlock::Lock theShaderLock(theShaderMutex);
	
	/* Check if the shared shader has no references yet: */
	if(theShaderRefCount==0)
		{
		/* Allocate a new shared shader object: */
		theShader=new TwoSided1DTexturedSurfaceShader;
		}
	
	/* Increment the reference counter and return the shared shader object: */
	++theShaderRefCount;
	return theShader;
	}

void TwoSided1DTexturedSurfaceShader::releaseShader(TwoSided1DTexturedSurfaceShader* shader)
	{
	Threads::Spinlock::Lock theShaderLock(theShaderMutex);
	
	/* Check if the to-be-released shader is really the shared shader: */
	if(shader!=0&&shader==theShader)
		{
		/* Decrement the reference counter: */
		--theShaderRefCount;
		
		/* Check if the shared shader can be deleted: */
		if(theShaderRefCount==0)
			{
			delete theShader;
			theShader=0;
			}
		}
	}

TwoSided1DTexturedSurfaceShader::TwoSided1DTexturedSurfaceShader(void)
	{
	}

TwoSided1DTexturedSurfaceShader::~TwoSided1DTexturedSurfaceShader(void)
	{
	}

void TwoSided1DTexturedSurfaceShader::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void TwoSided1DTexturedSurfaceShader::set(int colorTextureUnit,GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Check if the shader needs to be updated: */
	if(dataItem->lightTrackerVersion!=contextData.getLightTracker()->getVersion()||dataItem->clipPlaneTrackerVersion!=contextData.getClipPlaneTracker()->getVersion())
		{
		/* Rebuild the shader: */
		dataItem->buildShader(contextData);
		
		/* Mark the shader as up-to-date: */
		dataItem->lightTrackerVersion=contextData.getLightTracker()->getVersion();
		dataItem->clipPlaneTrackerVersion=contextData.getClipPlaneTracker()->getVersion();
		}
	
	/* Install the shader: */
	dataItem->shader.useProgram();
	glUniformARB(dataItem->colorTextureLocation,colorTextureUnit);
	}

void TwoSided1DTexturedSurfaceShader::reset(GLContextData& contextData) const
	{
	/* Uninstall the shader: */
	GLShader::disablePrograms();
	}
