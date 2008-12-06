/***********************************************************************
VolumeRenderer - Base class for texture-based volume renderers for
blocks of cartesian voxel data
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

#ifndef VOLUMERENDERER_INCLUDED
#define VOLUMERENDERER_INCLUDED

#include <Math/Math.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLObject.h>

class VolumeRenderer:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Type for scalar values
	typedef Geometry::ComponentArray<Scalar,3> Size; // Type for sizes
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	typedef Geometry::Vector<Scalar,3> Vector; // Type for vectors
	typedef unsigned char Voxel; // Type for voxel data
	
	enum VoxelAlignment // Alignment types for voxel data
		{
		VERTEX_CENTERED,CELL_CENTERED
		};
	
	enum RenderingMode // Rendering types (2D vs. 3D texturing)
		{
		AXIS_ALIGNED,VIEW_PERPENDICULAR
		};
	
	enum InterpolationMode // Interpolation types
		{
		CONSTANT,LINEAR
		};
	
	enum TextureFunction // Texture functions
		{
		REPLACE,MODULATE
		};
	
	protected:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool has3DTextures; // Flag if the local OpenGL supports 3D texture mapping in hardware
		bool hasNPOTDTextures; // Flag if the local OpenGL supports non-power-of-two-dimension textures
		unsigned int dataVersion,settingsVersion; // Counters to synchronize volume renderer and texture cache states
		int numTextureObjects; // Number of cached textures (texture objects)
		GLuint* textureObjectIDs; // Array of texture object IDs
		bool textureCacheValid; // Flag if the currently cached textures are valid
		int cachedAxis; // When axis-aligned textures are used, major axis for which textures are cached
		
		/* Flags telling how to exactly update the texture cache: */
		bool setParameters;
		bool uploadData;
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		
		/* Methods: */
		virtual void updateTextureCache(const VolumeRenderer* renderer,int majorAxis);
		virtual void deleteTextureCache(void);
		};
	
	struct BoxCorner // Structure to store each one of the box's eight corners
		{
		/* Elements: */
		public:
		Point position; // Box corner's position in model coordinates
		Point texture; // Box corner's texture coordinates
		int neighbours[3]; // Indices of the corner's three neighbours
		int incomingEdgeSuccessors[8];
		};
	
	struct ActiveEdge // Structure to store active edges during 3D proxy geometry generation
		{
		/* Elements: */
		public:
		bool expired; // Flag if the edge has already been expired
		int startIndex,endIndex; // Corner indices of edge's end points
		Point point; // Current intersection point on edge
		Vector dPoint; // Point increment between slices
		Point texture; // Texture coordinates associated with intersection point
		Vector dTexture; // Texture coordinate increment between slices
		ActiveEdge* pred; // Previous edge in counter-clockwise order
		ActiveEdge* succ; // Next edge in counter-clockwise order
		};
	
	struct EdgeExpiration // Structure to build a priority queue of edge expirations
		{
		/* Elements: */
		public:
		Scalar endD; // Distance of vertex ending edge (expiration point of edge)
		ActiveEdge* edge; // Pointer to associated active edge structure
		
		/* Constructors and destructors: */
		EdgeExpiration(Scalar sEndD,ActiveEdge* sEdge)
			:endD(sEndD),edge(sEdge)
			{
			}
		
		/* Methods: */
		static bool lessEqual(const EdgeExpiration& ee1,const EdgeExpiration& ee2)
			{
			return ee1.endD>=ee2.endD; // Note: Expirations are processed in reverse order of distance
			}
		};
	
	friend class DataItem;
	
	/* Elements: */
	/* Data block description: */
	bool privateData; // Flag if the voxel array has been allocated by the volume renderer itself
	const Voxel* values; // Pointer to the first interior voxel (not border voxel)
	int size[3]; // Extents of voxel block
	int borderSize; // Width of border around voxel block
	Voxel borderValue; // Data value to be assumed outside of the voxel block's boundaries
	VoxelAlignment alignment; // Alignment of voxel values in voxel block
	int numCells[3]; // Number of cells in each direction
	int rowLength,imageHeight; // Extra size measures to allow subblocking
	int increments[3]; // Pointer increments for voxel block
	bool useNPOTDTextures; // Flag whether the renderer should use non-power-of-two-dimension textures
	int textureSize[3]; // Size of 2D/3D texture that can hold the complete data block
	
	/* Data block geometry description: */
	Point origin; // Position of data block's origin in model coordinates
	Size extent; // Data block's size in model coordinates
	int subOrigin[3],subSize[3]; // Origin and size of the currently selected subblock
	BoxCorner corners[8]; // Array of corners storing the box's geometry and connectivity
	Scalar minCellSize; // Smallest side length of a volume cell
	
	/* Rendering mode description: */
	RenderingMode renderingMode; // Rendering mode
	GLenum interpolationMode; // Interpolation mode
	GLenum textureFunction; // Texture function to be used for slice rendering (GL_REPLACE or GL_MODULATE)
	Point sliceCenter; // Center point for slice generation in model coordinates
	Scalar sliceFactor; // Slice distance in units of cell size
	Scalar sliceDistance; // Slice distance for view-perpendicular rendering
	bool autosaveGLState; // Flag to enable automatic saving of the GL state
	
	/* Caching of generated textures: */
	bool textureCachingEnabled; // Flag to completely disable caching
	unsigned int dataVersion,settingsVersion; // Counters to synchronize volume renderer and texture cache state
	
	/* Protected methods: */
	virtual void deletePrivateData(void); // Deletes all privately allocated memory
	virtual void uploadTexture2D(DataItem* dataItem,int axis,int index) const; // Uploads a slice of voxel values as a 2D texture
	virtual void uploadTexture3D(DataItem* dataItem) const; // Uploads the complete voxel block as a 3D texture
	virtual void prepareRenderAxisAligned(DataItem* dataItem) const; // Called right before the texture slices are rendered
	void renderAxisAligned(DataItem* dataItem,const Vector& viewDirection) const; // Renders the voxel block as a stack of axis-aligned slices using 2D textures
	virtual void prepareRenderViewPerpendicular(DataItem* dataItem) const; // Called right before the texture block is rendered
	void renderViewPerpendicular(DataItem* dataItem,const Vector& viewDirection) const; // Renders the voxel block as a stack of view-perpendicular slices using 3D textures
	void calcIncrements(void); // Calculates pointer increments to navigate the voxel data
	Voxel* createPrivateMemoryBlock(const int newSize[3],int newBorderSize); // Calculates "optimal" memory layout for the given voxel block specification (sometimes counter-intuitive)
	void initBoxStructure(void); // Initializes the box corner structure
	virtual void updateVoxelBlock(void); // Called after voxel data has been changed
	void calcBoxTexCoords(void); // Updates the texture coordinates of the voxel block
	void calcBoxGeometry(void); // Updates the origin/extent of the voxel block
	void calcSlicingParameters(void); // Updates the minimum cell size and the slice distance
	
	/* Constructors and destructors: */
	public:
	VolumeRenderer(void); // Creates an uninitialized volume renderer
	VolumeRenderer(const char* filename); // Loads a private voxel block from a volume file
	VolumeRenderer(const Voxel* sValues,const int sSize[3],int sBorderSize,VoxelAlignment sAlignment); // Sets the volume renderer to a voxel block
	virtual ~VolumeRenderer(void);
	
	/* Methods: */
	bool hasVoxelBlock(void) const // Checks if the volume renderer is already associated with a voxel block
		{
		return values!=0;
		}
	const Voxel* getVoxelBlock(void) const // Returns pointer to the first voxel inside the voxel block
		{
		return values;
		}
	const int* getSize(void) const // Returns the voxel block's size
		{
		return size;
		}
	int getSize(int dimension) const // Ditto
		{
		return size[dimension];
		}
	const int* getNumCells(void) const // Returns the number of cells in the voxel block
		{
		return numCells;
		}
	int getNumCells(int dimension) const // Ditto
		{
		return numCells[dimension];
		}
	int getBorderSize(void) const // Returns the voxel block's border size
		{
		return borderSize;
		}
	Voxel getVoxel(int i,int j,int k) const // Ditto
		{
		return values[i*increments[0]+j*increments[1]+k];
		}
	const Voxel* getVoxelPtr(int i,int j,int k) const // Returns pointer into voxel block
		{
		return values+(i*increments[0]+j*increments[1]+k);
		}
	Voxel getBorderValue(void) const // Returns the voxel value assumed for exterior regions
		{
		return borderValue;
		}
	VoxelAlignment getVoxelAlignment(void) const // Returns the alignment of voxels inside their cells
		{
		return alignment;
		}
	int getIncrement(int dimension) const // Returns increment value to navigate the voxel block
		{
		return increments[dimension];
		}
	bool getUseNPOTDTextures(void) const // Returns the non-power-of-two-dimension textures flag
		{
		return useNPOTDTextures;
		}
	void setUseNPOTDTextures(bool newUseNPOTDTextures); // Sets the non-power-of-two-dimension textures flag
	const Point& getOrigin(void) const // Returns the voxel block's origin in model coordinates
		{
		return origin;
		}
	const Size& getExtent(void) const // Returns the voxel block's extent in model coordinates
		{
		return extent;
		}
	Scalar getExtent(int dimension) const // Ditto
		{
		return extent[dimension];
		}
	Point getCenter(void) const // Returns the voxel block's centroid in model coordinates
		{
		Point result=origin;
		for(int i=0;i<3;++i)
			result[i]+=extent[i]*Scalar(0.5);
		return result;
		}
	Scalar getRadius(void) const // Returns the voxel block's bounding sphere radius
		{
		Scalar radius2(0);
		for(int i=0;i<3;++i)
			radius2+=Math::sqr(extent[i]*Scalar(0.5));
		return Math::sqrt(radius2);
		}
	RenderingMode getRenderingMode(void) const // Returns the current rendering mode
		{
		return renderingMode;
		}
	TextureFunction getTextureFunction(void) const // Returns the current texture function
		{
		if(textureFunction==GL_REPLACE)
			return REPLACE;
		else
			return MODULATE;
		}
	Scalar getSliceFactor(void) const
		{
		if(renderingMode==VIEW_PERPENDICULAR)
			return sliceFactor;
		else
			return Scalar(1);
		}
	bool getAutosaveGLState(void) const
		{
		return autosaveGLState;
		}
	void clearVoxelBlock(void); // Clears the currently assigned voxel block
	Voxel* createVoxelBlock(const int newSize[3],int newBorderSize,VoxelAlignment newAlignment,int blockIncrements[3]); // Creates a new private voxel block and returns its base pointer and memory layout
	void finishVoxelBlock(void); // Must be called after data has been loaded into the new private voxel block
	void updateVoxelBlockData(void); // Notifies the volume renderer that data in the voxel block has changed
	void setVoxelBlock(const Voxel* newValues,const int newSize[3],int newBorderSize,VoxelAlignment newAlignment); // Sets the volume renderer to a new voxel block
	void setVoxelBlock(const Voxel* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VoxelAlignment newAlignment); // Sets the volume renderer to a new voxel block
	template <class SourceVoxelType>
	void setVoxelBlock(const SourceVoxelType* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VoxelAlignment newAlignment,SourceVoxelType rangeMin,SourceVoxelType rangeMax); // Sets the volume renderer to a new voxel block with conversion
	virtual void setRowLength(int newRowLength); // Sets the special row length
	virtual void setImageHeight(int newImageHeight); // Sets the special image height
	virtual void setBorderValue(Voxel newBorderValue) // Sets the border value
		{
		borderValue=newBorderValue;
		}
	virtual void setVoxelAlignment(VoxelAlignment newAlignment) // Sets the alignment of voxels in their cells
		{
		alignment=newAlignment;
		updateVoxelBlock();
		}
	void setPosition(const Point& newOrigin,const Size& newExtent) // Sets the voxel block's origin and size in model coordinates
		{
		/* Copy the new position: */
		origin=newOrigin;
		extent=newExtent;
		calcBoxGeometry();
		calcSlicingParameters();
		}
	void selectSubBlock(const int newSubOrigin[3],const int newSubSize[3]) // Selects a subblock of the volume for rendering
		{
		/* Copy the new subblock selection: */
		for(int i=0;i<3;++i)
			{
			subOrigin[i]=newSubOrigin[i];
			subSize[i]=newSubSize[i];
			}
		calcBoxTexCoords();
		calcBoxGeometry();
		}
	void setRenderingMode(RenderingMode newRenderingMode) // Sets the rendering mode
		{
		/* Set the new rendering mode: */
		renderingMode=newRenderingMode;
		++dataVersion;
		++settingsVersion;
		}
	void setInterpolationMode(InterpolationMode newInterpolationMode) // Sets the interpolation mode
		{
		switch(newInterpolationMode)
			{
			case CONSTANT:
				interpolationMode=GL_NEAREST;
				break;
			
			case LINEAR:
				interpolationMode=GL_LINEAR;
				break;
			}
		++settingsVersion;
		}
	void setTextureFunction(TextureFunction newTextureFunction) // Sets the texture function
		{
		switch(newTextureFunction)
			{
			case REPLACE:
				textureFunction=GL_REPLACE;
				break;
			
			case MODULATE:
				textureFunction=GL_MODULATE;
				break;
			}
		++settingsVersion;
		}
	void setSliceCenter(const Point& newSliceCenter)
		{
		sliceCenter=newSliceCenter;
		}
	void setSliceFactor(Scalar newSliceFactor)
		{
		sliceFactor=newSliceFactor;
		sliceDistance=minCellSize*sliceFactor;
		}
	void setSlicingParameters(const Point& newSliceCenter,Scalar newSliceDistance)
		{
		sliceCenter=newSliceCenter;
		sliceDistance=newSliceDistance;
		}
	void setAutosaveGLState(bool newAutosaveGLState)
		{
		/* Set the autosave mode: */
		autosaveGLState=newAutosaveGLState;
		}
	void setTextureCaching(bool newTextureCachingEnabled) //  Enables/disables caching of textures
		{
		/* Set the new caching state: */
		textureCachingEnabled=newTextureCachingEnabled;
		}
	virtual void initContext(GLContextData& contextData) const;
	virtual void setGLState(GLContextData& contextData) const; // Prepares OpenGL for volume rendering
	virtual void resetGLState(GLContextData& contextData) const; // Reverts all state changes
	static Vector calcViewDirection(void); // Calculates viewing direction from current OpenGL context
	void renderBlock(GLContextData& contextData) const; // Renders the volume block using OpenGL's viewing direction
	virtual void renderBlock(GLContextData& contextData,const Vector& viewDirection) const; // Renders the volume block using a given viewing direction
	VolumeRenderer& loadVolumeFile(const char* filename); // Loads a private voxel block from a volume file
	};

#endif
