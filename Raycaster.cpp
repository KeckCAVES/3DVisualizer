/***********************************************************************
Raycaster - Base class for volume renderers for Cartesian gridded data
using GLSL shaders.
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

#include <Raycaster.h>

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBDepthTexture.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBShadow.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>
#include <GL/Extensions/GLEXTTexture3D.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/VRWindow.h>
#include <Vrui/DisplayState.h>

/************************************
Methods of class Raycaster::DataItem:
************************************/

Raycaster::DataItem::DataItem(void)
	:hasNPOTDTextures(GLARBTextureNonPowerOfTwo::isSupported()),
	 depthTextureID(0),depthFramebufferID(0),
	 mcScaleLoc(-1),mcOffsetLoc(-1),
	 depthSamplerLoc(-1),depthMatrixLoc(-1),depthSizeLoc(-1),
	 eyePositionLoc(-1),stepSizeLoc(-1)
	{
	/* Check for the required OpenGL extensions: */
	if(!GLShader::isSupported())
		Misc::throwStdErr("GPURaycasting::initContext: Shader objects not supported by local OpenGL");
	//if(!GLARBMultitexture::isSupported()||!GLEXTTexture3D::isSupported())
	//	Misc::throwStdErr("GPURaycasting::initContext: Multitexture or 3D texture extension not supported by local OpenGL");
	if(!GLEXTFramebufferObject::isSupported()||!GLARBDepthTexture::isSupported()||!GLARBShadow::isSupported())
		Misc::throwStdErr("GPURaycasting::initContext: Framebuffer object extension or depth/shadow texture extension not supported by local OpenGL");
	
	/* Initialize all required OpenGL extensions: */
	GLARBDepthTexture::initExtension();
	GLARBMultitexture::initExtension();
	GLARBShadow::initExtension();
	if(hasNPOTDTextures)
		GLARBTextureNonPowerOfTwo::initExtension();
	GLEXTFramebufferObject::initExtension();
	GLEXTTexture3D::initExtension();
	
	/* Create the depth texture: */
	glGenTextures(1,&depthTextureID);
	glBindTexture(GL_TEXTURE_2D,depthTextureID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE_ARB,GL_NONE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24_ARB,1,1,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,0);
	depthTextureSize[0]=depthTextureSize[1]=1;
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Create the depth framebuffer and attach the depth texture to it: */
	glGenFramebuffersEXT(1,&depthFramebufferID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,depthFramebufferID);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,depthTextureID,0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	}

Raycaster::DataItem::~DataItem(void)
	{
	/* Destroy the depth texture and framebuffer: */
	glDeleteFramebuffersEXT(1,&depthFramebufferID);
	glDeleteTextures(1,&depthTextureID);
	}

void Raycaster::DataItem::initDepthBuffer(const int* windowSize)
	{
	/* Calculate the new depth texture size: */
	GLsizei newDepthTextureSize[2];
	if(hasNPOTDTextures)
		{
		/* Use the viewport size: */
		for(int i=0;i<2;++i)
			newDepthTextureSize[i]=windowSize[i];
		}
	else
		{
		/* Pad the viewport size to the next power of two: */
		for(int i=0;i<2;++i)
			for(newDepthTextureSize[i]=1;newDepthTextureSize[i]<windowSize[i];newDepthTextureSize[i]<<=1)
				;
		}
	
	/* Bind the depth texture: */
	glBindTexture(GL_TEXTURE_2D,depthTextureID);
	
	/* Check if the depth texture size needs to change: */
	if(newDepthTextureSize[0]!=depthTextureSize[0]||newDepthTextureSize[1]!=depthTextureSize[1])
		{
		/* Re-allocate the depth texture: */
		glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24_ARB,newDepthTextureSize[0],newDepthTextureSize[1],0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,0);
		
		/* Store the new depth texture size: */
		for(int i=0;i<2;++i)
			depthTextureSize[i]=newDepthTextureSize[i];
		}
	
	/* Query the current viewport: */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	
	/* Copy the current depth buffer into the depth texture: */
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,viewport[0],viewport[1],viewport[0],viewport[1],viewport[2],viewport[3]);
	
	/* Unbind the depth texture: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

/**************************
Methods of class Raycaster:
**************************/

void Raycaster::initDataItem(Raycaster::DataItem* dataItem) const
	{
	/* Calculate the appropriate volume texture's size: */
	if(dataItem->hasNPOTDTextures)
		{
		/* Use the data size directly: */
		for(int i=0;i<3;++i)
			dataItem->textureSize[i]=dataSize[i];
		}
	else
		{
		/* Pad to the next power of two: */
		for(int i=0;i<3;++i)
			for(dataItem->textureSize[i]=1;dataItem->textureSize[i]<GLsizei(dataSize[i]);dataItem->textureSize[i]<<=1)
				;
		}
	
	/* Calculate the texture coordinate box for trilinear interpolation and the transformation from model space to data space: */
	Point tcMin,tcMax;
	for(int i=0;i<3;++i)
		{
		tcMin[i]=Scalar(0.5)/Scalar(dataItem->textureSize[i]);
		tcMax[i]=(Scalar(dataSize[i])-Scalar(0.5))/Scalar(dataItem->textureSize[i]);
		Scalar scale=(tcMax[i]-tcMin[i])/domain.getSize(i);
		dataItem->mcScale[i]=GLfloat(scale);
		dataItem->mcOffset[i]=GLfloat(tcMin[i]-domain.min[i]*scale);
		}
	dataItem->texCoords=Box(tcMin,tcMax);
	}

void Raycaster::initShader(Raycaster::DataItem* dataItem) const
	{
	/* Get the shader's uniform locations: */
	dataItem->mcScaleLoc=dataItem->shader.getUniformLocation("mcScale");
	dataItem->mcOffsetLoc=dataItem->shader.getUniformLocation("mcOffset");
	
	dataItem->depthSamplerLoc=dataItem->shader.getUniformLocation("depthSampler");
	dataItem->depthMatrixLoc=dataItem->shader.getUniformLocation("depthMatrix");
	dataItem->depthSizeLoc=dataItem->shader.getUniformLocation("depthSize");
	
	dataItem->eyePositionLoc=dataItem->shader.getUniformLocation("eyePosition");
	dataItem->stepSizeLoc=dataItem->shader.getUniformLocation("stepSize");
	}

void Raycaster::bindShader(const Raycaster::PTransform& pmv,const Raycaster::PTransform& mv,Raycaster::DataItem* dataItem) const
	{
	/* Set up the data space transformation: */
	glUniform3fvARB(dataItem->mcScaleLoc,1,dataItem->mcScale);
	glUniform3fvARB(dataItem->mcOffsetLoc,1,dataItem->mcOffset);
	
	/* Bind the ray termination texture: */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,dataItem->depthTextureID);
	glUniform1iARB(dataItem->depthSamplerLoc,0);
	
	/* Set the termination matrix: */
	glUniformMatrix4fvARB(dataItem->depthMatrixLoc,1,GL_TRUE,pmv.getMatrix().getEntries());
	
	/* Set the depth texture size: */
	glUniform2fARB(dataItem->depthSizeLoc,float(dataItem->depthTextureSize[0]),float(dataItem->depthTextureSize[1]));
	
	/* Calculate the eye position in model coordinates: */
	Point eye=pmv.inverseTransform(PTransform::HVector(0,0,1,0)).toPoint();
	glUniform3fvARB(dataItem->eyePositionLoc,1,eye.getComponents());
	
	/* Set the sampling step size: */
	glUniform1fARB(dataItem->stepSizeLoc,stepSize*cellSize);
	}

void Raycaster::unbindShader(Raycaster::DataItem* dataItem) const
	{
	/* Unbind the ray termination texture: */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	}

Polyhedron<Raycaster::Scalar>* Raycaster::clipDomain(const Raycaster::PTransform& pmv,const Raycaster::PTransform& mv) const
	{
	/* Clip the render domain against the view frustum's front plane: */
	Point fv0=pmv.inverseTransform(Point(-1,-1,-1));
	Point fv1=pmv.inverseTransform(Point( 1,-1,-1));
	Point fv2=pmv.inverseTransform(Point(-1, 1,-1));
	Point fv3=pmv.inverseTransform(Point( 1, 1,-1));
	Plane::Vector normal=Geometry::cross(fv1-fv0,fv2-fv0)
	                     +Geometry::cross(fv3-fv1,fv0-fv1)
	                     +Geometry::cross(fv2-fv3,fv1-fv3)
	                     +Geometry::cross(fv0-fv2,fv3-fv2);
	Scalar offset=(normal*fv0+normal*fv1+normal*fv2+normal*fv3)*Scalar(0.25);
	Polyhedron<Scalar>* clippedDomain=renderDomain.clip(Plane(normal,offset));
	
	/* Clip the render domain against all active clipping planes: */
	GLint numClipPlanes;
	glGetIntegerv(GL_MAX_CLIP_PLANES,&numClipPlanes);
	for(GLint i=0;i<numClipPlanes;++i)
		if(glIsEnabled(GL_CLIP_PLANE0+i))
			{
			/* Get the clipping plane's plane equation in eye coordinates: */
			GLdouble planeEq[4];
			glGetClipPlane(GL_CLIP_PLANE0+i,planeEq);
			
			/* Transform the clipping plane to model coordinates: */
			Geometry::ComponentArray<Scalar,4> hn;
			for(int i=0;i<3;++i)
				hn[i]=Scalar(-planeEq[i]);
			hn[3]=-planeEq[3];
			hn=mv.getMatrix().transposeMultiply(hn);
			
			/* Clip the domain: */
			Polyhedron<Scalar>* newClippedDomain=clippedDomain->clip(Polyhedron<Scalar>::Plane(Polyhedron<Scalar>::Plane::Vector(hn.getComponents()),-hn[3]-Scalar(1.0e-4)));
			delete clippedDomain;
			clippedDomain=newClippedDomain;
			}
	
	return clippedDomain;
	}

Raycaster::Raycaster(const unsigned int sDataSize[3],const Raycaster::Box& sDomain)
	:domain(sDomain),domainExtent(0),cellSize(0),
	 renderDomain(Polyhedron<Scalar>::Point(domain.min),Polyhedron<Scalar>::Point(domain.max)),
	 stepSize(1)
	{
	/* Copy the data sizes and calculate the data strides and cell size: */
	ptrdiff_t stride=1;
	for(int i=0;i<3;++i)
		{
		dataSize[i]=sDataSize[i];
		dataStrides[i]=stride;
		stride*=ptrdiff_t(dataSize[i]);
		domainExtent+=Math::sqr(domain.max[i]-domain.min[i]);
		cellSize+=Math::sqr((domain.max[i]-domain.min[i])/Scalar(dataSize[i]-1));
		}
	domainExtent=Math::sqrt(domainExtent);
	cellSize=Math::sqrt(cellSize);
	}

Raycaster::~Raycaster(void)
	{
	}

void Raycaster::setStepSize(Raycaster::Scalar newStepSize)
	{
	/* Set the new step size: */
	stepSize=newStepSize;
	}

void Raycaster::glRenderAction(GLContextData& contextData) const
	{
	/* Get the OpenGL-dependent application data from the GLContextData object: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Bail out if shader is invalid: */
	if(!dataItem->shader.isValid())
		return;
	
	/* Save OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT);
	
	/* Initialize the ray termination depth frame buffer: */
	const Vrui::DisplayState& vds=Vrui::getDisplayState(contextData);
	dataItem->initDepthBuffer(vds.window->getWindowSize());
	
	/* Bind the ray termination framebuffer: */
	GLint currentFramebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT,&currentFramebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,dataItem->depthFramebufferID);
	
	/* Get the projection and modelview matrices: */
	PTransform mv=glGetModelviewMatrix<Scalar>();
	PTransform pmv=glGetProjectionMatrix<Scalar>();
	pmv*=mv;
	
	/* Clip the render domain against the view frustum's front plane and all clipping planes: */
	Polyhedron<Scalar>* clippedDomain=clipDomain(pmv,mv);
	
	/* Draw the clipped domain's back faces to the depth buffer as ray termination conditions: */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	clippedDomain->drawFaces();
	
	/* Unbind the depth framebuffer: */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,currentFramebuffer);
	
	/* Install the GLSL shader program: */
	dataItem->shader.useProgram();
	bindShader(pmv,mv,dataItem);
	
	/* Draw the clipped domain's front faces: */
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glCullFace(GL_BACK);
	clippedDomain->drawFaces();
	
	/* Uninstall the GLSL shader program: */
	unbindShader(dataItem);
	GLShader::disablePrograms();
	
	/* Clean up: */
	delete clippedDomain;
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}
