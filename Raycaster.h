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

#ifndef RAYCASTER_INCLUDED
#define RAYCASTER_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Box.h>
#include <Geometry/Plane.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLShader.h>

#include <Polyhedron.h>

class Raycaster:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Box<Scalar,3> Box;
	typedef Geometry::Plane<Scalar,3> Plane;
	typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
	
	protected:
	struct DataItem:public GLObject::DataItem // Structure containing per-context state
		{
		/* Elements: */
		public:
		bool hasNPOTDTextures; // Flag whether the local OpenGL supports non-power of two textures
		
		GLsizei textureSize[3]; // Size of textures able to hold the volume data
		Box texCoords; // Domain of texture coordinates to access the volume data
		GLfloat mcScale[3],mcOffset[3]; // Scale factors and offsets from model space to data space
		
		GLuint depthTextureID; // Texture object ID of the depth texture used for ray termination
		GLuint depthFramebufferID; // Framebuffer object ID to render to the ray termination buffer
		GLsizei depthTextureSize[2]; // Current size of the depth texture
		
		GLShader shader; // Shader object for the raycasting algorithm
		int mcScaleLoc; // Location of the scale factors from model coordinates to data coordinates
		int mcOffsetLoc; // Location of the offset from model coordinates to data coordinates
		int depthSamplerLoc; // Location of the depth texture sampler uniform variable
		int depthMatrixLoc; // Location of the depth texture transformation matrix uniform variable
		int depthSizeLoc; // Location of the depth texture size uniform variable
		int eyePositionLoc; // Location of the eye position uniform variable
		int stepSizeLoc; // Location of the step size uniform variable
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		
		/* Methods: */
		virtual void initDepthBuffer(const int* windowSize); // Creates the depth texture and framebuffer based on current OpenGL state and copies the current depth buffer contents
		};
	
	/* Elements: */
	protected:
	unsigned int dataSize[3]; // Size of volume data
	ptrdiff_t dataStrides[3]; // Volume data strides in x, y, z dimensions
	Box domain; // The volume renderer's domain box in model space
	Scalar domainExtent; // Length of longest ray through domain
	Scalar cellSize; // The data set's cell size
	Polyhedron<Scalar> renderDomain; // Polyhedron used to render the clipped data set
	
	Scalar stepSize; // The ray casting step size in cell size units
	
	/* Protected methods: */
	protected:
	virtual void initDataItem(DataItem* dataItem) const; // Initializes the given context data item
	virtual void initShader(DataItem* dataItem) const; // Initializes the GLSL raycasting shader
	virtual void bindShader(const PTransform& pmv,const PTransform& mv,DataItem* dataItem) const; // Prepares the GLSL raycasting shader for rendering
	virtual void unbindShader(DataItem* dataItem) const; // Unbinds the GLSL raycasting shader after rendering
	Polyhedron<Scalar>* clipDomain(const PTransform& pmv,const PTransform& mv) const; // Clips the domain against the view frustum and all clipping planes and returns the resulting polyhedron
	
	/* Constructors and destructors: */
	public:
	Raycaster(const unsigned int sDataSize[3],const Box& sDomain); // Creates a raycaster for the given data and domain sizes
	virtual ~Raycaster(void); // Destroys the raycaster
	
	/* New methods: */
	const unsigned int* getDataSize(void) const // Returns the raycaster's data size
		{
		return dataSize;
		}
	unsigned int getDataSize(int dimension) const // Returns one dimension of the raycaster's data size
		{
		return dataSize[dimension];
		}
	const ptrdiff_t* getDataStrides(void) const // Returns the volume data's strides in x, y, z directions
		{
		return dataStrides;
		}
	ptrdiff_t getDataStrides(int dimension) const // Returns one dimension of the volume data's strides
		{
		return dataStrides[dimension];
		}
	const Box& getDomain(void) const // Returns the raycaster's domain box in model space
		{
		return domain;
		}
	Scalar getCellSize(void) const // Returns the data's average cell size
		{
		return cellSize;
		}
	Scalar getStepSize(void) const // Returns the raycaster's step size in cell size units
		{
		return stepSize;
		}
	virtual void setStepSize(Scalar newStepSize); // Sets the raycaster's step size in cell size units
	virtual void glRenderAction(GLContextData& contextData) const; // Renders the data using current settings from the current OpenGL context
	};

#endif
