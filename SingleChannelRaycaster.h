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

#ifndef SINGLECHANNELRAYCASTER_INCLUDED
#define SINGLECHANNELRAYCASTER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColorMap.h>

#include <Raycaster.h>

class SingleChannelRaycaster:public Raycaster
	{
	/* Embedded classes: */
	protected:
	typedef GLubyte Voxel; // Type for voxel data
	
	struct DataItem:public Raycaster::DataItem
		{
		/* Elements: */
		public:
		bool haveFloatTextures; // Flag whether the local OpenGL supports floating-point textures
		
		GLuint volumeTextureID; // Texture object ID for volume data texture
		unsigned int volumeTextureVersion; // Version number of volume data texture
		GLuint colorMapTextureID; // Texture object ID for stepsize-adjusted color map texture
		
		int volumeSamplerLoc; // Location of the volume data texture sampler
		int colorMapSamplerLoc; // Location of the color map texture sampler
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	protected:
	Voxel* data; // Pointer to the volume dataset
	unsigned int dataVersion; // Version number of the volume dataset to track changes
	const GLColorMap* colorMap; // Pointer to the color map
	GLfloat transparencyGamma; // Adjustment factor for color map's overall opacity
	
	/* Protected methods: */
	protected:
	virtual void initDataItem(Raycaster::DataItem* dataItem) const;
	virtual void initShader(Raycaster::DataItem* dataItem) const;
	virtual void bindShader(const PTransform& pmv,const PTransform& mv,Raycaster::DataItem* dataItem) const;
	virtual void unbindShader(Raycaster::DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	SingleChannelRaycaster(const unsigned int sDataSize[3],const Box& sDomain); // Creates a volume renderer
	virtual ~SingleChannelRaycaster(void); // Destroys the raycaster
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods from Raycaster: */
	virtual void setStepSize(Scalar newStepSize);
	
	/* New methods: */
	const Voxel* getData(void) const // Returns pointer to the volume dataset
		{
		return data;
		}
	Voxel* getData(void) // Ditto
		{
		return data;
		}
	virtual void updateData(void); // Notifies the raycaster that the volume dataset has changed
	const GLColorMap* getColorMap(void) const // Returns the raycaster's color map
		{
		return colorMap;
		}
	void setColorMap(const GLColorMap* newColorMap); // Sets the raycaster's color map
	GLfloat getTransparencyGamma(void) const // Returns the opacity adjustment factor
		{
		return transparencyGamma;
		}
	void setTransparencyGamma(GLfloat newTransparencyGamma); // Sets the opacity adjustment factor
	};

#endif
