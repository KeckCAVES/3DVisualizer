/***********************************************************************
TripleChannelRaycaster - Class for volume renderers with three
independent scalar channels.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef TRIPLECHANNELRAYCASTER_INCLUDED
#define TRIPLECHANNELRAYCASTER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLColorMap.h>

#include <Raycaster.h>

class TripleChannelRaycaster:public Raycaster
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
		GLuint colorMapTextureIDs[3]; // Texture object IDs for per-channel stepsize-adjusted color map textures
		
		int volumeSamplerLoc; // Location of the volume data texture sampler
		int channelEnabledsLoc; // Location of the three channel enable flags
		int colorMapSamplersLoc; // Location of the three color map texture samplers
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	protected:
	Voxel* data; // Pointer to the volume dataset
	unsigned int dataVersion; // Version number of the volume dataset to track changes
	bool channelEnableds[3]; // Flags to enable/disable each channel separately
	const GLColorMap* colorMaps[3]; // Pointers to the three channel color maps
	GLfloat transparencyGammas[3]; // Adjustment factor for each color map's overall opacity
	
	/* Protected methods: */
	protected:
	virtual void initDataItem(Raycaster::DataItem* dataItem) const;
	virtual void initShader(Raycaster::DataItem* dataItem) const;
	virtual void bindShader(const PTransform& pmv,const PTransform& mv,Raycaster::DataItem* dataItem) const;
	virtual void unbindShader(Raycaster::DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	TripleChannelRaycaster(const unsigned int sDataSize[3],const Box& sDomain); // Creates a volume renderer
	virtual ~TripleChannelRaycaster(void); // Destroys the raycaster
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods from Raycaster: */
	virtual void setStepSize(Scalar newStepSize);
	
	/* New methods: */
	const Voxel* getData(int channel) const // Returns pointer to the volume dataset for the given channel
		{
		return data+channel;
		}
	Voxel* getData(int channel) // Ditto
		{
		return data+channel;
		}
	virtual void updateData(void); // Notifies the raycaster that the volume dataset has changed
	bool getChannelEnabled(int channel) const // Returns the enabled flag for the given channel
		{
		return channelEnableds[channel];
		}
	void setChannelEnabled(int channel,bool newChannelEnabled); // Enables or disables the given channel
	const GLColorMap* getColorMap(int channel) const // Returns the raycaster's color map for the given scalar channel
		{
		return colorMaps[channel];
		}
	void setColorMap(int channel,const GLColorMap* newColorMap); // Sets the raycaster's color map for the given scalar channel
	GLfloat getTransparencyGamma(int channel) const // Returns the opacity adjustment factor for the given scalar channel
		{
		return transparencyGammas[channel];
		}
	void setTransparencyGamma(int channel,GLfloat newTransparencyGamma); // Sets the opacity adjustment factor for the given scalar channel
	};

#endif
