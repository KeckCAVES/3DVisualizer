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

#ifndef PALETTERENDERER_INCLUDED
#define PALETTERENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColorMap.h>

#include "VolumeRenderer.h"

class PaletteRenderer:public VolumeRenderer
	{
	/* Embedded classes: */
	protected:
	enum RenderingPath // Enumerated type for OpenGL rendering paths
		{
		FragmentProgram,TextureShader,PalettedTexture
		};
	
	struct DataItem:public VolumeRenderer::DataItem
		{
		/* Elements: */
		public:
		RenderingPath renderingPath; // Appropriate rendering path for the current OpenGL context
		GLuint fragmentProgramId; // ID of fragment program for texture mapping
		GLuint paletteTextureObjectId; // Texture object ID of the palette texture in fragment program or texture shader mode
		unsigned int cachedColorMapVersion; // The currently cached color map version
		
		/* More texture cache update flags: */
		bool uploadColorMap; // Flag to upload color map during texture upload (costly and not always necessary)
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		
		/* Methods inherited from VolumeRenderer::DataItem: */
		virtual void updateTextureCache(const VolumeRenderer* renderer,int majorAxis);
		virtual void deleteTextureCache(void);
		};
	
	friend class DataItem;
	
	/* Elements: */
	unsigned int colorMapVersion; // Version number of the current color map
	const GLColorMap* colorMap; // Colormap containing transfer functions. Must have 256 entries
	bool sharePalette; // Flag if palette renderer uses the global texture palette
	
	/* Protected methods inherited from VolumeRenderer: */
	void uploadTexture2D(VolumeRenderer::DataItem* dataItem,int axis,int index) const;
	void uploadTexture3D(VolumeRenderer::DataItem* dataItem) const;
	void prepareRenderAxisAligned(VolumeRenderer::DataItem* dataItem) const;
	void prepareRenderViewPerpendicular(VolumeRenderer::DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	PaletteRenderer(void) // Creates an uninitialized paletted renderer
		:colorMapVersion(0),colorMap(0),sharePalette(true)
		{
		};
	PaletteRenderer(const char* filename) // Loads a private voxel block from a volume file
		:VolumeRenderer(filename),colorMapVersion(0),colorMap(0),sharePalette(true)
		{
		};
	PaletteRenderer(const Voxel* sValues,const int sSize[3],int sBorderSize,VoxelAlignment sAlignment) // Sets the paletted renderer to a volume block
		:VolumeRenderer(sValues,sSize,sBorderSize,sAlignment),colorMapVersion(0),colorMap(0),sharePalette(true)
		{
		};
	
	/* Methods inherited from VolumeRenderer: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void setGLState(GLContextData& contextData) const;
	virtual void resetGLState(GLContextData& contextData) const;
	
	/* New methods: */
	const GLColorMap* getColorMap(void) const // Returns a pointer to the used color map
		{
		return colorMap;
		};
	void setColorMap(const GLColorMap* newColorMap); // Sets a new color map
	void setSharePalette(bool newSharePalette); // Sets color palette sharing flag
	static void setGlobalColorMap(const GLColorMap* newGlobalColorMap); // Uploads a global color map to be shared by multiple palette renderers
	};

#endif
