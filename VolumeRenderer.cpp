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

#include <Misc/PriorityHeap.h>
#include <Misc/File.h>
#include <Math/Math.h>
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <GL/Extensions/GLEXTTexture3D.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLTextures.h>

#include "VolumeRenderer.h"

int numPolygons;

/*****************************************
Methods of class VolumeRenderer::DataItem:
*****************************************/

VolumeRenderer::DataItem::DataItem(void)
	:has3DTextures(GLEXTTexture3D::isSupported()),
	 hasNPOTDTextures(GLARBTextureNonPowerOfTwo::isSupported()),
	 dataVersion(0),settingsVersion(0),
	 numTextureObjects(0),textureObjectIDs(0),
	 setParameters(true),uploadData(true)
	{
	/* Initialize relevant OpenGL extensions: */
	if(has3DTextures)
		GLEXTTexture3D::initExtension();
	if(hasNPOTDTextures)
		GLARBTextureNonPowerOfTwo::initExtension();
	}

VolumeRenderer::DataItem::~DataItem(void)
	{
	if(numTextureObjects!=0)
		{
		glDeleteTextures(numTextureObjects,textureObjectIDs);
		delete[] textureObjectIDs;
		}
	}

void VolumeRenderer::DataItem::updateTextureCache(const VolumeRenderer* renderer,int majorAxis)
	{
	setParameters=false;
	uploadData=false;
	if(dataVersion!=renderer->dataVersion||majorAxis!=cachedAxis)
		{
		/* Calculate the number of required texture objects: */
		int requiredNumTextures=1;
		if(!has3DTextures||renderer->renderingMode==VolumeRenderer::AXIS_ALIGNED)
			{
			for(int i=0;i<3;++i)
				if(requiredNumTextures<renderer->size[i])
					requiredNumTextures=renderer->size[i];
			}
		
		/* Reallocate the texture cache if necessary: */
		if(numTextureObjects!=requiredNumTextures)
			{
			if(numTextureObjects!=0)
				{
				glDeleteTextures(numTextureObjects,textureObjectIDs);
				delete[] textureObjectIDs;
				}
			numTextureObjects=requiredNumTextures;
			textureObjectIDs=new GLuint[numTextureObjects];
			glGenTextures(numTextureObjects,textureObjectIDs);
			}
		
		/* Invalidate the texture cache: */
		dataVersion=renderer->dataVersion;
		cachedAxis=majorAxis;
		textureCacheValid=false;
		setParameters=true;
		uploadData=true;
		}
	
	if(settingsVersion!=renderer->settingsVersion)
		{
		/* Invalidate the texture cache: */
		settingsVersion=renderer->settingsVersion;
		textureCacheValid=false;
		setParameters=true;
		}
	}

void VolumeRenderer::DataItem::deleteTextureCache(void)
	{
	if(numTextureObjects!=0)
		{
		/* Delete all textures: */
		glDeleteTextures(numTextureObjects,textureObjectIDs);

		/* Delete the texture cache: */
		numTextureObjects=0;
		delete[] textureObjectIDs;
		textureObjectIDs=0;
		textureCacheValid=false;
		cachedAxis=-1;
		}
	setParameters=true;
	uploadData=true;
	}

/*******************************
Methods of class VolumeRenderer:
*******************************/

void VolumeRenderer::deletePrivateData(void)
	{
	if(privateData)
		{
		/* Calculate the base address of the voxel block: */
		const Voxel* valueBase=values;
		for(int i=0;i<3;++i)
			valueBase-=borderSize*increments[i];
		
		/* Delete the allocated voxel block: */
		delete[] valueBase;
		}
	
	/* Reset elements: */
	privateData=false;
	values=0;
	}

void VolumeRenderer::uploadTexture2D(VolumeRenderer::DataItem* dataItem,int axis,int index) const
	{
	if(dataItem->setParameters)
		{
		/* Set the OpenGL texturing parameters: */
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolationMode);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolationMode);
		}
	
	if(dataItem->uploadData)
		{
		/* Upload a texture slice: */
		const Voxel* slicePtr=values+index*increments[axis];
		switch(axis)
			{
			case 0:
				glTexImage2D(GL_TEXTURE_2D,0,GL_INTENSITY8,textureSize[2],textureSize[1],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[2],size[1],increments[2],increments[1],GL_LUMINANCE,GL_UNSIGNED_BYTE,slicePtr);
				break;
			case 1:
				glTexImage2D(GL_TEXTURE_2D,0,GL_INTENSITY8,textureSize[2],textureSize[0],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[2],size[0],increments[2],increments[0],GL_LUMINANCE,GL_UNSIGNED_BYTE,slicePtr);
				break;
			case 2:
				glTexImage2D(GL_TEXTURE_2D,0,GL_INTENSITY8,textureSize[1],textureSize[0],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size[1],size[0],increments[1],increments[0],GL_LUMINANCE,GL_UNSIGNED_BYTE,slicePtr);
				break;
			}
		}
	}

void VolumeRenderer::uploadTexture3D(VolumeRenderer::DataItem* dataItem) const
	{
	if(dataItem->setParameters)
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
	
	if(dataItem->uploadData)
		{
		/* Upload the texture block: */
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0); // increments[1]); // Seems to be a bug in OpenGL - consistent across SGI/nVidia platforms
		glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,0); // increments[0]);
		glPixelStorei(GL_UNPACK_SKIP_IMAGES,0);
		glTexImage3DEXT(GL_TEXTURE_3D,0,GL_INTENSITY8,textureSize[2],textureSize[1],textureSize[0],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
		glTexSubImage3DEXT(GL_TEXTURE_3D,0,0,0,0,size[2],size[1],size[0],GL_LUMINANCE,GL_UNSIGNED_BYTE,values);
		}
	}

void VolumeRenderer::prepareRenderAxisAligned(VolumeRenderer::DataItem* dataItem) const
	{
	}

void VolumeRenderer::renderAxisAligned(VolumeRenderer::DataItem* dataItem,const Vector& viewDirection) const
	{
	/* Identify the major rendering axis and the stacking direction: */
	int majorAxis;
	int textureAxis[2];
	Vector vda;
	for(int i=0;i<3;++i)
		vda[i]=Math::abs(viewDirection[i]);
	if(vda[0]>=vda[1]&&vda[0]>=vda[2])
		{
		/* Major axis is x: */
		majorAxis=0;
		textureAxis[0]=2;
		textureAxis[1]=1;
		}
	else if(vda[1]>=vda[2])
		{
		/* Major axis is y: */
		majorAxis=1;
		textureAxis[0]=2;
		textureAxis[1]=0;
		}
	else
		{
		/* Major axis is z: */
		majorAxis=2;
		textureAxis[0]=1;
		textureAxis[1]=0;
		}
	
	/* Determine stacking order and calculate the slices' corner positions and texture coordinates: */
	static int cornerIndices[3][2][4]={{{0,2,6,4},{1,5,7,3}},{{0,4,5,1},{2,3,7,6}},{{0,1,3,2},{4,6,7,5}}};
	int stackingOrder;
	int sliceIndex,sliceIncrement,lastSlice;
	Scalar quadCornerIncrement;
	Point quadCorner[4];
	Scalar quadTexCoord[4][2];
	if(viewDirection[majorAxis]<Scalar(0))
		{
		/* Stacking order is upwards: */
		stackingOrder=0;
		sliceIndex=subOrigin[majorAxis];
		lastSlice=subOrigin[majorAxis]+subSize[majorAxis];
		if(alignment==VERTEX_CENTERED)
			++lastSlice;
		sliceIncrement=1;
		quadCornerIncrement=corners[7].position[majorAxis]-corners[0].position[majorAxis];
		}
	else
		{
		/* Stacking order is downwards: */
		stackingOrder=1;
		sliceIndex=subOrigin[majorAxis]+subSize[majorAxis]-1;
		if(alignment==VERTEX_CENTERED)
			++sliceIndex;
		lastSlice=subOrigin[majorAxis]-1;
		sliceIncrement=-1;
		quadCornerIncrement=corners[0].position[majorAxis]-corners[7].position[majorAxis];
		}
	quadCornerIncrement/=Scalar(subSize[majorAxis]);
	
	/* Copy positions and texture coordinates from the box structure: */
	for(int i=0;i<4;++i)
		{
		const BoxCorner& c=corners[cornerIndices[majorAxis][stackingOrder][i]];
		quadCorner[i]=c.position;
		for(int j=0;j<2;++j)
			quadTexCoord[i][j]=c.texture[2-textureAxis[j]];
		}
	
	/* Adjust for cell-centered voxels (texture slices are aligned with cell centers): */
	if(alignment==CELL_CENTERED)
		for(int i=0;i<4;++i)
			quadCorner[i][majorAxis]+=quadCornerIncrement*Scalar(0.5);
	
	/* Create/delete the texture cache if necessary: */
	if(textureCachingEnabled)
		dataItem->updateTextureCache(this,majorAxis);
	else
		dataItem->deleteTextureCache();
	
	/* Prepare rendering: */
	prepareRenderAxisAligned(dataItem);
	
	/* Render each slice as a textured quadrilateral: */
	for(;sliceIndex!=lastSlice;sliceIndex+=sliceIncrement)
		{
		#if 1
		/* Upload the slice texture: */
		if(textureCachingEnabled)
			{
			glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIDs[sliceIndex]);
			if(!dataItem->textureCacheValid)
				uploadTexture2D(dataItem,majorAxis,sliceIndex);
			}
		else
			uploadTexture2D(dataItem,majorAxis,sliceIndex);
		#endif
		
		/* Render a quadrilateral: */
		glBegin(GL_QUADS);
		for(int i=0;i<4;++i)
			{
			glTexCoord<2>(quadTexCoord[i]);
			glVertex(quadCorner[i]);
			quadCorner[i][majorAxis]+=quadCornerIncrement;
			}
		glEnd();
		++numPolygons;
		}
	
	#if 1
	if(textureCachingEnabled)
		{
		/* Unbind the last texture to prevent someone else from tampering with it: */
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Validate the texture cache: */
		dataItem->textureCacheValid=true;
		}
	#endif
	}

void VolumeRenderer::prepareRenderViewPerpendicular(VolumeRenderer::DataItem* dataItem) const
	{
	}

void VolumeRenderer::renderViewPerpendicular(VolumeRenderer::DataItem* dataItem,const VolumeRenderer::Vector& viewDirection) const
	{
	/* Calculate the corners' parameters along the viewing direction: */
	Scalar cornerD[8];
	for(int i=0;i<8;++i)
		cornerD[i]=corners[i].position*viewDirection;
	
	/* Find the box's distance range and the farthest away corner: */
	int maxCorner=0;
	Scalar minD=cornerD[0];
	Scalar maxD=cornerD[0];
	for(int i=1;i<8;++i)
		{
		if(minD>cornerD[i])
			minD=cornerD[i];
		else if(maxD<cornerD[i])
			{
			maxD=cornerD[i];
			maxCorner=i;
			}
		}
	
	/* Calculate the distance of the farthest slice: */
	Scalar sliceOffset=sliceCenter*viewDirection;
	Scalar sliceD=Math::floor((maxD-sliceOffset)/sliceDistance)*sliceDistance+sliceOffset;
	
	/* Initialize the list of active edges: */
	ActiveEdge edges[12];
	ActiveEdge* firstEdge=&edges[0];
	ActiveEdge* nextEdge=&edges[0];
	Misc::PriorityHeap<EdgeExpiration,EdgeExpiration> expirations(6);
	for(int i=0;i<3;++i,++nextEdge)
		{
		/* Initialize the edge: */
		nextEdge->expired=false;
		nextEdge->startIndex=maxCorner;
		int endCorner=corners[maxCorner].neighbours[i];
		nextEdge->endIndex=endCorner;
		Scalar rangeD=cornerD[endCorner]-cornerD[maxCorner];
		if(rangeD!=Scalar(0))
			{
			nextEdge->dPoint=(corners[endCorner].position-corners[maxCorner].position)/rangeD;
			nextEdge->point=corners[maxCorner].position+nextEdge->dPoint*(sliceD-cornerD[maxCorner]);
			nextEdge->dPoint*=sliceDistance;
			nextEdge->dTexture=(corners[endCorner].texture-corners[maxCorner].texture)/rangeD;
			nextEdge->texture=corners[maxCorner].texture+nextEdge->dTexture*(sliceD-cornerD[maxCorner]);
			nextEdge->dTexture*=sliceDistance;
			}
		nextEdge->pred=&edges[(i+2)%3];
		nextEdge->succ=&edges[(i+1)%3];
		
		/* Store its expiration distance: */
		expirations.insert(EdgeExpiration(cornerD[endCorner],nextEdge));
		}
	
	/* Create/delete the texture cache if necessary: */
	if(textureCachingEnabled)
		dataItem->updateTextureCache(this,-1);
	else
		dataItem->deleteTextureCache();
	
	/* Set up OpenGL texturing parameters: */
	prepareRenderViewPerpendicular(dataItem);
	
	#if 1
	/* Upload the block texture: */
	if(textureCachingEnabled)
		{
		glBindTexture(GL_TEXTURE_3D,dataItem->textureObjectIDs[0]);
		if(!dataItem->textureCacheValid)
			uploadTexture3D(dataItem);
		}
	else
		uploadTexture3D(dataItem);
	#endif
	
	/* Generate slices while updating the active edge list: */
	while(sliceD>minD)
		{
		/* Process all expired edges: */
		while(expirations.getSmallest().endD>=sliceD)
			{
			/* Distinguish the four expiration cases: */
			ActiveEdge* edge=expirations.getSmallest().edge;
			int startIndex=edge->endIndex;
			if(edge->expired)
				{
				/* Edge has already expired; just remove it from the expiration queue: */
				expirations.removeSmallest();
				}
			else if(startIndex!=edge->pred->endIndex&&startIndex!=edge->succ->endIndex)
				{
				/* Split the edge: */
				edge->expired=true;
				
				/* Create the two new edges: */
				nextEdge->expired=false;
				nextEdge->startIndex=startIndex;
				int endIndex1=corners[startIndex].incomingEdgeSuccessors[edge->startIndex];
				nextEdge->endIndex=endIndex1;
				Scalar rangeD1=cornerD[endIndex1]-cornerD[startIndex];
				if(rangeD1!=Scalar(0))
					{
					nextEdge->dPoint=(corners[endIndex1].position-corners[startIndex].position)/rangeD1;
					nextEdge->point=corners[startIndex].position+nextEdge->dPoint*(sliceD-cornerD[startIndex]);
					nextEdge->dPoint*=sliceDistance;
					nextEdge->dTexture=(corners[endIndex1].texture-corners[startIndex].texture)/rangeD1;
					nextEdge->texture=corners[startIndex].texture+nextEdge->dTexture*(sliceD-cornerD[startIndex]);
					nextEdge->dTexture*=sliceDistance;
					}
				nextEdge->pred=edge->pred;
				nextEdge->pred->succ=nextEdge;
				nextEdge->succ=nextEdge+1;
				expirations.getSmallest().endD=cornerD[endIndex1];
				expirations.getSmallest().edge=nextEdge;
				expirations.reinsertSmallest();
				++nextEdge;
				nextEdge->expired=false;
				nextEdge->startIndex=startIndex;
				int endIndex2=corners[startIndex].incomingEdgeSuccessors[endIndex1];
				nextEdge->endIndex=endIndex2;
				Scalar rangeD2=cornerD[endIndex2]-cornerD[startIndex];
				if(rangeD2!=Scalar(0))
					{
					nextEdge->dPoint=(corners[endIndex2].position-corners[startIndex].position)/rangeD2;
					nextEdge->point=corners[startIndex].position+nextEdge->dPoint*(sliceD-cornerD[startIndex]);
					nextEdge->dPoint*=sliceDistance;
					nextEdge->dTexture=(corners[endIndex2].texture-corners[startIndex].texture)/rangeD2;
					nextEdge->texture=corners[startIndex].texture+nextEdge->dTexture*(sliceD-cornerD[startIndex]);
					nextEdge->dTexture*=sliceDistance;
					}
				nextEdge->pred=nextEdge-1;
				nextEdge->succ=edge->succ;
				nextEdge->succ->pred=nextEdge;
				firstEdge=nextEdge;
				expirations.insert(EdgeExpiration(cornerD[endIndex2],nextEdge));
				++nextEdge;
				}
			else
				{
				/* Merge the edge with one of its neighbours: */
				ActiveEdge* pred;
				ActiveEdge* succ;
				if(startIndex==edge->pred->endIndex)
					{
					/* Merge with the clockwise neighbour: */
					pred=edge->pred;
					succ=edge;
					}
				else
					{
					/* Merge with the counter-clockwise neighbour: */
					pred=edge;
					succ=edge->succ;
					}
				pred->expired=true;
				succ->expired=true;
				
				/* Create the new edge: */
				nextEdge->expired=false;
				nextEdge->startIndex=startIndex;
				int endIndex=corners[startIndex].incomingEdgeSuccessors[pred->startIndex];
				nextEdge->endIndex=endIndex;
				Scalar rangeD=cornerD[endIndex]-cornerD[startIndex];
				if(rangeD!=Scalar(0))
					{
					nextEdge->dPoint=(corners[endIndex].position-corners[startIndex].position)/rangeD;
					nextEdge->point=corners[startIndex].position+nextEdge->dPoint*(sliceD-cornerD[startIndex]);
					nextEdge->dPoint*=sliceDistance;
					nextEdge->dTexture=(corners[endIndex].texture-corners[startIndex].texture)/rangeD;
					nextEdge->texture=corners[startIndex].texture+nextEdge->dTexture*(sliceD-cornerD[startIndex]);
					nextEdge->dTexture*=sliceDistance;
					}
				nextEdge->pred=pred->pred;
				nextEdge->pred->succ=nextEdge;
				nextEdge->succ=succ->succ;
				nextEdge->succ->pred=nextEdge;
				firstEdge=nextEdge;
				expirations.getSmallest().endD=cornerD[endIndex];
				expirations.getSmallest().edge=nextEdge;
				expirations.reinsertSmallest();
				++nextEdge;
				}
			}
		
		/* Generate the current polygon: */
		#if 1
		glBegin(GL_POLYGON);
		ActiveEdge* ePtr=firstEdge;
		do
			{
			glTexCoord(ePtr->texture);
			ePtr->texture-=ePtr->dTexture;
			glVertex(ePtr->point);
			ePtr->point-=ePtr->dPoint;
			ePtr=ePtr->succ;
			}
		while(ePtr!=firstEdge);
		glEnd();
		#else
		ActiveEdge* ePtr=firstEdge;
		do
			{
			ePtr->texture-=ePtr->dTexture;
			ePtr->point-=ePtr->dPoint;
			ePtr=ePtr->succ;
			}
		while(ePtr!=firstEdge);
		#endif
		++numPolygons;
		
		/* Go to the next slice: */
		sliceD-=sliceDistance;
		}
	
	#if 1
	if(textureCachingEnabled)
		{
		/* Unbind the texture to prevent someone else from tampering with it: */
		glBindTexture(GL_TEXTURE_3D,0);
		
		/* Validate the texture cache: */
		dataItem->textureCacheValid=true;
		}
	#endif
	}

void VolumeRenderer::calcIncrements(void)
	{
	increments[2]=1;
	if(rowLength==0)
		increments[1]=increments[2]*(size[2]+borderSize+borderSize);
	else
		increments[1]=rowLength;
	if(imageHeight==0)
		increments[0]=increments[1]*(size[1]+borderSize+borderSize);
	else
		increments[0]=imageHeight;
	}

VolumeRenderer::Voxel* VolumeRenderer::createPrivateMemoryBlock(const int newSize[3],int newBorderSize)
	{
	/* Calculate the private block's specification: */
	privateData=true;
	for(int i=0;i<3;++i)
		size[i]=newSize[i];
	borderSize=newBorderSize;
	rowLength=0;
	imageHeight=0;
	
	int numVoxels=1;
	#ifdef __SGI_IRIX__
	/**************************************************************
	The most fucking ugly workaround ever committed by mankind:
	Calculate the texture size OpenGL will be using ahead of time,
	and allocate a user memory array of that size. Only fill in the
	used part of the array though, and set up "fake" increments to
	fool the other parts of this class into working correctly. If
	this ain't gonna win the golden raspberry, I don't know what.
	--
	In retrospect, this actually works incredibly well.
	**************************************************************/
	
	/* Calculate the fake image size: */
	int imageSize[3];
	for(int i=0;i<3;++i)
		{
		int bSize=size[i]+borderSize+borderSize;
		for(imageSize[i]=1;imageSize[i]<bSize;imageSize[i]+=imageSize[i])
			;
		numVoxels*=imageSize[i];
		}
	increments[2]=1;
	increments[1]=imageSize[2];
	increments[0]=imageSize[1]*imageSize[2];
	#else
	calcIncrements();
	for(int i=0;i<3;++i)
		{
		int bSize=size[i]+borderSize+borderSize;
		numVoxels*=bSize;
		}
	#endif
	Voxel* result=new Voxel[numVoxels];
	values=result;
	return result;
	}

void VolumeRenderer::initBoxStructure(void)
	{
	/* Construct the box's connectivity: */
	static const int cornerNeighbours[8][3]={{1,2,4},{0,5,3},{0,3,6},{1,7,2},{0,6,5},{1,4,7},{2,7,4},{3,5,6}};
	for(int i=0;i<8;++i)
		{
		for(int j=0;j<3;++j)
			{
			corners[i].neighbours[j]=cornerNeighbours[i][j];
			corners[i].incomingEdgeSuccessors[cornerNeighbours[i][j]]=cornerNeighbours[i][(j+1)%3];
			}
		}
	}

void VolumeRenderer::updateVoxelBlock(void)
	{
	for(int i=0;i<3;++i)
		{
		/* Calculate the number of cells: */
		numCells[i]=size[i];
		if(alignment==VERTEX_CENTERED)
			--numCells[i];
		
		/* Reset the subblock selection: */
		subOrigin[i]=0;
		subSize[i]=numCells[i];
		
		/* Calculate the texture image size: */
		if(useNPOTDTextures)
			{
			/* Just use the data size as texture image size: */
			textureSize[i]=size[i];
			}
		else
			{
			/* Adjust texture image size to the next power of two: */
			for(textureSize[i]=1;textureSize[i]<size[i];textureSize[i]+=textureSize[i])
				;
			}
		}
	
	/* Update other settings depending on the voxel block size: */
	calcBoxTexCoords();
	calcBoxGeometry();
	calcSlicingParameters();
	
	/* Update the data version counter: */
	++dataVersion;
	}

void VolumeRenderer::calcBoxTexCoords(void)
	{
	int iMask=0x1;
	for(int i=0;i<3;++i,iMask+=iMask)
		{
		Scalar texMin,texMax;
		if(alignment==CELL_CENTERED)
			{
			texMin=Scalar(subOrigin[i])/Scalar(textureSize[i]);
			texMax=Scalar(subOrigin[i]+subSize[i])/Scalar(textureSize[i]);
			}
		else
			{
			texMin=(Scalar(subOrigin[i])+Scalar(0.5))/Scalar(textureSize[i]);
			texMax=(Scalar(subOrigin[i]+subSize[i])+Scalar(0.5))/Scalar(textureSize[i]);
			}
		
		/* Update the box's texture coordinates: */
		for(int j=0;j<8;++j)
			corners[j].texture[2-i]=j&iMask?texMax:texMin;
		}
	}

void VolumeRenderer::calcBoxGeometry(void)
	{
	/* Calculate the corner positions in model coordinates: */
	int iMask=0x1;
	for(int i=0;i<3;++i,iMask+=iMask)
		{
		Scalar coordMin=origin[i]+Scalar(subOrigin[i])*extent[i]/Scalar(numCells[i]);
		Scalar coordMax=origin[i]+Scalar(subOrigin[i]+subSize[i])*extent[i]/Scalar(numCells[i]);
		
		/* Update the box's corner coordinates: */
		for(int j=0;j<8;++j)
			corners[j].position[i]=j&iMask?coordMax:coordMin;
		}
	}

void VolumeRenderer::calcSlicingParameters(void)
	{
	/* Calculate the minimal cell side length: */
	minCellSize=Math::abs(extent[0]/Scalar(numCells[0]));
	for(int i=1;i<3;++i)
		{
		Scalar cellSize=Math::abs(extent[i]/Scalar(numCells[i]));
		if(minCellSize>cellSize)
			minCellSize=cellSize;
		}
	
	/* Calculate the slicing distance for view-perpendicular rendering: */
	sliceDistance=minCellSize*sliceFactor;
	}

VolumeRenderer::VolumeRenderer(void)
	:privateData(false),values(0),rowLength(0),imageHeight(0),
	 useNPOTDTextures(false),renderingMode(AXIS_ALIGNED),
	 interpolationMode(GL_NEAREST),textureFunction(GL_REPLACE),sliceFactor(Scalar(0.5)),
	 autosaveGLState(true),textureCachingEnabled(false),
	 dataVersion(0),settingsVersion(0)
	{
	initBoxStructure();
	}

VolumeRenderer::VolumeRenderer(const char* filename)
	:privateData(false),values(0),rowLength(0),imageHeight(0),
	 useNPOTDTextures(false),renderingMode(AXIS_ALIGNED),
	 interpolationMode(GL_NEAREST),textureFunction(GL_REPLACE),sliceFactor(Scalar(0.5)),
	 autosaveGLState(true),textureCachingEnabled(false),
	 dataVersion(0),settingsVersion(0)
	{
	initBoxStructure();
	loadVolumeFile(filename);
	}

VolumeRenderer::VolumeRenderer(const VolumeRenderer::Voxel* sValues,const int sSize[3],int sBorderSize,VolumeRenderer::VoxelAlignment sAlignment)
	:privateData(false),values(0),rowLength(0),imageHeight(0),
	 useNPOTDTextures(false),renderingMode(AXIS_ALIGNED),
	 interpolationMode(GL_NEAREST),textureFunction(GL_REPLACE),sliceFactor(Scalar(0.5)),
	 autosaveGLState(true),textureCachingEnabled(false),
	 dataVersion(0),settingsVersion(0)
	{
	initBoxStructure();
	setVoxelBlock(sValues,sSize,sBorderSize,sAlignment);
	}

VolumeRenderer::~VolumeRenderer(void)
	{
	deletePrivateData();
	}

void VolumeRenderer::setUseNPOTDTextures(bool newUseNPOTDTextures)
	{
	/* Set the non-power-of-two-dimension texture flag: */
	useNPOTDTextures=newUseNPOTDTextures;
	
	/* Re-calculate the voxel block layout: */
	updateVoxelBlock();
	}

void VolumeRenderer::clearVoxelBlock(void)
	{
	deletePrivateData();
	}

VolumeRenderer::Voxel* VolumeRenderer::createVoxelBlock(const int newSize[3],int newBorderSize,VoxelAlignment newAlignment,int blockIncrements[3])
	{
	deletePrivateData();
	
	/* Create the private memory block: */
	Voxel* result=createPrivateMemoryBlock(newSize,newBorderSize);
	
	/* Set the private block's specification: */
	alignment=newAlignment;
	
	/* Return the resulting block: */
	for(int i=0;i<3;++i)
		blockIncrements[i]=increments[i];
	return result;
	}

void VolumeRenderer::finishVoxelBlock(void)
	{
	/* Update other data depending on the block specification: */
	updateVoxelBlock();
	}

void VolumeRenderer::updateVoxelBlockData(void)
	{
	/* Update the data version counter: */
	++dataVersion;
	}

void VolumeRenderer::setVoxelBlock(const VolumeRenderer::Voxel* newValues,const int newSize[3],int newBorderSize,VolumeRenderer::VoxelAlignment newAlignment)
	{
	deletePrivateData();
	
	/* Use the given array as non-private data: */
	privateData=false;
	values=newValues;
	
	/* Copy the given specifications: */
	for(int i=0;i<3;++i)
		size[i]=newSize[i];
	borderSize=newBorderSize;
	alignment=newAlignment;
	calcIncrements();
	
	/* Update other data depending on the block specification: */
	updateVoxelBlock();
	}

void VolumeRenderer::setVoxelBlock(const VolumeRenderer::Voxel* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VolumeRenderer::VoxelAlignment newAlignment)
	{
	deletePrivateData();
	
	/* Create the private memory block: */
	Voxel* ownValues=createPrivateMemoryBlock(newSize,newBorderSize);
	
	/* Set the private block's specification: */
	alignment=newAlignment;
	
	/* Copy all source values: */
	int bSize[3];
	for(int i=0;i<3;++i)
		bSize[i]=size[i]+borderSize+borderSize;
	const Voxel* svPlane=newValues;
	for(int x=0;x<bSize[0];++x,svPlane+=newIncrements[0])
		{
		const Voxel* svRow=svPlane;
		for(int y=0;y<bSize[1];++y,svRow+=newIncrements[1])
			{
			const Voxel* svPtr=svRow;
			for(int z=0;z<bSize[2];++z,svPtr+=newIncrements[2])
				ownValues[x*increments[0]+y*increments[1]+z*increments[2]]=*svPtr;
			}
		}
	
	/* Update other data depending on the block specification: */
	updateVoxelBlock();
	}

template <class SourceVoxelType>
void VolumeRenderer::setVoxelBlock(const SourceVoxelType* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VolumeRenderer::VoxelAlignment newAlignment,SourceVoxelType rangeMin,SourceVoxelType rangeMax)
	{
	deletePrivateData();
	
	/* Create the private memory block: */
	Voxel* ownValues=createPrivateMemoryBlock(newSize,newBorderSize);
	
	/* Set the private block's specification: */
	alignment=newAlignment;
	
	/* Copy all source values: */
	double rangeScale=256.0/double(rangeMax-rangeMin);
	double rm=double(rangeMin);
	int bSize[3];
	for(int i=0;i<3;++i)
		bSize[i]=size[i]+borderSize+borderSize;
	
	/* Modify the iteration sequence to access the source data as consecutively as possible: */
	if(newIncrements[0]>newIncrements[2])
		{
		const SourceVoxelType* svPlane=newValues;
		for(int x=0;x<bSize[0];++x,svPlane+=newIncrements[0])
			{
			const SourceVoxelType* svRow=svPlane;
			for(int y=0;y<bSize[1];++y,svRow+=newIncrements[1])
				{
				const SourceVoxelType* svPtr=svRow;
				for(int z=0;z<bSize[2];++z,svPtr+=newIncrements[2])
					{
					Voxel* vPtr=ownValues+(x*increments[0]+y*increments[1]+z*increments[2]);
					if(*svPtr<rangeMin)
						*vPtr=Voxel(0);
					else if(*svPtr>=rangeMax)
						*vPtr=Voxel(255);
					else
						*vPtr=Voxel((double(*svPtr)-rm)*rangeScale);
					}
				}
			}
		}
	else
		{
		const SourceVoxelType* svPlane=newValues;
		for(int z=0;z<bSize[2];++z,svPlane+=newIncrements[2])
			{
			const SourceVoxelType* svRow=svPlane;
			for(int y=0;y<bSize[1];++y,svRow+=newIncrements[1])
				{
				const SourceVoxelType* svPtr=svRow;
				for(int x=0;x<bSize[0];++x,svPtr+=newIncrements[0])
					{
					Voxel* vPtr=ownValues+(x*increments[0]+y*increments[1]+z*increments[2]);
					if(*svPtr<rangeMin)
						*vPtr=Voxel(0);
					else if(*svPtr>=rangeMax)
						*vPtr=Voxel(255);
					else
						*vPtr=Voxel((double(*svPtr)-rm)*rangeScale);
					}
				}
			}
		}
	
	/* Update other data depending on the block specification: */
	updateVoxelBlock();
	}

void VolumeRenderer::setRowLength(int newRowLength)
	{
	rowLength=newRowLength;
	calcIncrements();
	}

void VolumeRenderer::setImageHeight(int newImageHeight)
	{
	imageHeight=newImageHeight;
	calcIncrements();
	}

void VolumeRenderer::initContext(GLContextData& contextData) const
	{
	contextData.addDataItem(this,new DataItem);
	}

void VolumeRenderer::setGLState(GLContextData& contextData) const
	{
	/* Get a pointer to the context data: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up the OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_POLYGON_BIT|GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	if(dataItem->has3DTextures&&renderingMode==VIEW_PERPENDICULAR)
		glEnable(GL_TEXTURE_3D);
	else
		glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,textureFunction);
	}

void VolumeRenderer::resetGLState(GLContextData& contextData) const
	{
	/* Restore the OpenGL state: */
	glPopAttrib();
	}

VolumeRenderer::Vector VolumeRenderer::calcViewDirection(void)
	{
	typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
	typedef Geometry::HVector<Scalar,3> HVector;
	
	/* Retrieve the viewing direction in model coordinates: */
	PTransform pmv=glGetMatrix<Scalar>(GLMatrixEnums::PROJECTION);
	pmv*=glGetMatrix<Scalar>(GLMatrixEnums::MODELVIEW);
	#if 1
	HVector x=pmv.inverseTransform(HVector(1,0,0,0));
	HVector y=pmv.inverseTransform(HVector(0,1,0,0));
	Vector viewDirection=Geometry::cross(y.toVector(),x.toVector());
	#else
	HVector eye=pmv.inverseTransform(HVector(0,0,1,0));
	Vector viewDirection;
	if(eye[3]!=Scalar(0))
		{
		/* Perspective projection, the eye is located at an affine point: */
		viewDirection=(origin+Vector(extent)*Scalar(0.5))-eye.toPoint();
		}
	else
		{
		/* Parallel projection, the eye is at infinity: */
		viewDirection=eye.toVector();
		}
	#endif
	viewDirection.normalize();
	
	return viewDirection;
	}

void VolumeRenderer::renderBlock(GLContextData& contextData) const
	{
	/* Render the block with automatically calculated viewing direction: */
	renderBlock(contextData,calcViewDirection());
	}

void VolumeRenderer::renderBlock(GLContextData& contextData,const VolumeRenderer::Vector& viewDirection) const
	{
	/* Get a pointer to the context data: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Render the voxel block using the current rendering mode: */
	if(autosaveGLState)
		setGLState(contextData);
	if(dataItem->has3DTextures&&renderingMode==VIEW_PERPENDICULAR)
		renderViewPerpendicular(dataItem,viewDirection);
	else
		renderAxisAligned(dataItem,viewDirection);
	if(autosaveGLState)
		resetGLState(contextData);
	}

VolumeRenderer& VolumeRenderer::loadVolumeFile(const char* filename)
	{
	/* Open the volume file: */
	Misc::File volFile(filename,"rb",Misc::File::BigEndian);
	
	/* Read the volume file header: */
	int newSize[3];
	volFile.read(newSize,3);
	int newBorderSize=volFile.read<int>();
	
	/* Set the voxel block's position and size, and the slice center to the block's center: */
	origin=Point::origin;
	for(int i=0;i<3;++i)
		{
		extent[i]=Scalar(volFile.read<float>());
		sliceCenter[i]=origin[i]+extent[i]*Scalar(0.5);
		}
	
	/* Create a voxel array: */
	int numVoxels=(newSize[0]+2*newBorderSize)*(newSize[1]+2*newBorderSize)*(newSize[2]+2*newBorderSize);
	Voxel* newValueBase=new Voxel[numVoxels];
	
	/* Determine the data type stored in the volume file: */
	const char* extPtr=0;
	for(const char* cPtr=filename;*cPtr!='\0';++cPtr)
		if(*cPtr=='.')
			extPtr=cPtr;
	if(strcasecmp(extPtr,".vol")==0)
		{
		/* Read the unsigned char voxel values from file: */
		volFile.read(newValueBase,numVoxels);
		}
	else if(strcasecmp(extPtr,".fvol")==0)
		{
		/* Read the float voxel values from file: */
		float* floatValueBase=new float[numVoxels];
		volFile.read(floatValueBase,numVoxels);
		
		/* Determine the voxel data's value range: */
		float minValue,maxValue;
		minValue=maxValue=floatValueBase[0];
		for(int i=1;i<numVoxels;++i)
			{
			if(minValue>floatValueBase[i])
				minValue=floatValueBase[i];
			else if(maxValue<floatValueBase[i])
				maxValue=floatValueBase[i];
			}
		
		/* Convert the float data to unsigned char: */
		for(int i=0;i<numVoxels;++i)
			newValueBase[i]=(unsigned char)Math::floor((floatValueBase[i]-minValue)*255.0f/(maxValue-minValue)+0.5f);
		
		delete[] floatValueBase;
		}
	
	/* Calculate the address of the voxel block: */
	Voxel* newValues=newValueBase+newBorderSize;
	int newIncrements[3];
	newIncrements[2]=1;
	for(int i=2;i>0;--i)
		{
		newIncrements[i-1]=newIncrements[i]*(newSize[i]+2*newBorderSize);
		newValues+=newBorderSize*newIncrements[i-1];
		}

	/* Set the voxel block as private data: */
	setVoxelBlock(newValues,newSize,newBorderSize,VERTEX_CENTERED);
	borderValue=0;
	privateData=true;
	
	return *this;
	}

/*******************************
Template instantiation requests:
*******************************/

template void VolumeRenderer::setVoxelBlock<float>(const float* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VoxelAlignment newAlignment,float rangeMin,float rangeMax);
template void VolumeRenderer::setVoxelBlock<double>(const double* newValues,const int newSize[3],int newBorderSize,const int newIncrements[3],VoxelAlignment newAlignment,double rangeMin,double rangeMax);
