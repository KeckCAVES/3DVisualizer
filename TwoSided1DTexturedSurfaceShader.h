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

#ifndef TWOSIDED1DTEXTUREDSURFACESHADER_INCLUDED
#define TWOSIDED1DTEXTUREDSURFACESHADER_INCLUDED

#include <Threads/Spinlock.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLShader.h>

class TwoSided1DTexturedSurfaceShader:public GLObject
	{
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLShader shader; // The GLSL shader object containing the linked shader program
		unsigned int lightTrackerVersion; // Version number of the OpenGL lighting state for which the shader program was built
		unsigned int clipPlaneTrackerVersion; // Version number of the OpenGL clipping plane state for which the shader program was built
		int colorTextureLocation; // Location of color texture sampler in linked shader
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		
		/* Methods: */
		void buildShader(GLContextData& contextData); // Rebuilds the shader program according to the given OpenGL context's current state
		};
	
	/* Elements: */
	private:
	static Threads::Spinlock theShaderMutex; // Mutex protecting the shared shader object and its reference counter
	static unsigned int theShaderRefCount; // Reference counter for the shared shader object
	static TwoSided1DTexturedSurfaceShader* theShader; // Pointer to the shared two-sided surface shader object
	
	/* Constructors and destructors: */
	public:
	static bool isSupported(GLContextData& contextData); // Returns true if simulated two-sided lighting is supported in the given OpenGL context
	static TwoSided1DTexturedSurfaceShader* acquireShader(void); // Returns a pointer to a shared two-sided surface shader
	static void releaseShader(TwoSided1DTexturedSurfaceShader* shader); // Releases the given two-sided surface shader
	private:
	TwoSided1DTexturedSurfaceShader(void); // Creates a two-sided surface shader
	virtual ~TwoSided1DTexturedSurfaceShader(void); // Destroys the shader
	
	/* Methods from GLObject: */
	public:
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	void set(int colorTextureUnit,GLContextData& contextData) const; // Sets up two-sided 1D-textured surface shading in the given OpenGL context using the given texture unit for the 1D texture
	void reset(GLContextData& contextData) const; // Resets the given OpenGL context to the state before set() was called
	};

#endif
