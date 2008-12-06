/***********************************************************************
SliceVolumeRendererSampling - Generic class to render volumetric images
of data sets by resampling to a Cartesian grid and using a texture-based
volume renderer.
Copyright (c) 2007 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERER_IMPLEMENTATION

#include <Misc/Utility.h>
#include <Misc/ArrayIndex.h>
#include <Comm/MulticastPipe.h>
#include <GL/gl.h>
#include <GL/GLColorMap.h>
#include <VolumeRenderer.h>
#include <PaletteRenderer.h>

#include <Templatized/SliceVolumeRendererSampling.h>

namespace Visualization {

namespace Templatized {

/************************************
Methods of class SliceVolumeRenderer:
************************************/

template <class DataSetParam,class ScalarExtractorParam>
inline
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::SliceVolumeRenderer(
	const typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::DataSet* sDataSet,
	const typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::ScalarExtractor& sScalarExtractor,
	const GLColorMap* sColorMap,
	Comm::MulticastPipe* sPipe)
	:dataSet(sDataSet),
	 scalarExtractor(sScalarExtractor),
	 colorMap(sColorMap),
	 renderer(0),
	 transparencyGamma(1.0f)
	{
	/* Determine the data set's value range: */
	VScalar minValue,maxValue;
	typename DataSet::VertexIterator vIt=dataSet->beginVertices();
	minValue=maxValue=vIt->getValue(scalarExtractor);
	for(++vIt;vIt!=dataSet->endVertices();++vIt)
		{
		VScalar value=vIt->getValue(scalarExtractor);
		if(minValue>value)
			minValue=value;
		else if(maxValue<value)
			maxValue=value;
		}
	
	/* Create a palette renderer: */
	renderer=new PaletteRenderer;
	
	/* Create a regular grid covering the data set's domain: */
	Point boxOrigin=dataSet->getDomainBox().getOrigin();
	typename DataSet::Box::Size boxSize=dataSet->getDomainBox().getSize();
	Scalar avgCellSize=dataSet->calcAverageCellSize();
	typename DataSet::Box::Size cellSize;
	Misc::ArrayIndex<3> numVertices;
	for(int i=0;i<3;++i)
		{
		/* Find a power-of-two grid size that approximates the data set's average cell size: */
		Scalar optSize=Scalar(2)*boxSize[i]/avgCellSize;
		for(numVertices[i]=2;numVertices[i]<512&&Scalar(numVertices[i])*Math::sqrt(Scalar(2))<optSize;numVertices[i]<<=1)
			;
		cellSize[i]=boxSize[i]/Scalar(numVertices[i]-1);
		}
	
	/* Create a voxel block: */
	PaletteRenderer::Voxel* voxels;
	int increments[3];
	voxels=renderer->createVoxelBlock(numVertices.getComponents(),0,PaletteRenderer::VERTEX_CENTERED,increments);
	
	/* Sort the voxel block's dimensions according to their stride values: */
	int dims[3];
	for(int i=0;i<3;++i)
		dims[i]=i;
	if(increments[dims[0]]<increments[dims[1]])
		Misc::swap(dims[0],dims[1]);
	if(increments[dims[1]]<increments[dims[2]])
		Misc::swap(dims[1],dims[2]);
	if(increments[dims[0]]<increments[dims[1]])
		Misc::swap(dims[0],dims[1]);
	
	if(sPipe==0||sPipe->isMaster())
		{
		/* Sample the data set's scalar values into the voxel block: */
		typename DataSet::Locator sampleLocator=dataSet->getLocator();
		bool sampleValid=false;
		Misc::ArrayIndex<3> index;
		Point gridPos;
		PaletteRenderer::Voxel* base0;
		for(index[dims[0]]=0,gridPos[dims[0]]=boxOrigin[dims[0]],base0=voxels;index[dims[0]]<numVertices[dims[0]];++index[dims[0]],gridPos[dims[0]]+=cellSize[dims[0]],base0+=increments[dims[0]])
			{
			PaletteRenderer::Voxel* base1;
			for(index[dims[1]]=0,gridPos[dims[1]]=boxOrigin[dims[1]],base1=base0;index[dims[1]]<numVertices[dims[1]];++index[dims[1]],gridPos[dims[1]]+=cellSize[dims[1]],base1+=increments[dims[1]])
				{
				PaletteRenderer::Voxel* base2;
				for(index[dims[2]]=0,gridPos[dims[2]]=boxOrigin[dims[2]],base2=base1;index[dims[2]]<numVertices[dims[2]];++index[dims[2]],gridPos[dims[2]]+=cellSize[dims[2]],base2+=increments[dims[2]])
					{
					/* Locate the grid point: */
					sampleValid=sampleLocator.locatePoint(gridPos,sampleValid);
					if(sampleValid)
						{
						/* Get the vertex' scalar value: */
						VScalar value=sampleLocator.calcValue(scalarExtractor);
						*base2=PaletteRenderer::Voxel(Math::floor((value-minValue)*VScalar(255)/(maxValue-minValue)+VScalar(0.5)));
						}
					else
						{
						/* Assign a default value: */
						*base2=PaletteRenderer::Voxel(0);
						}
					}
				
				if(sPipe!=0)
					{
					/* Write the most recent span of voxels to the pipe: */
					sPipe->write<PaletteRenderer::Voxel>(base1,numVertices[dims[2]]);
					}
				}
			}
		}
	else
		{
		/* Receive the resampled data set from the multicast pipe: */
		Misc::ArrayIndex<3> index;
		PaletteRenderer::Voxel* base0;
		for(index[dims[0]]=0,base0=voxels;index[dims[0]]<numVertices[dims[0]];++index[dims[0]],base0+=increments[dims[0]])
			{
			PaletteRenderer::Voxel* base1;
			for(index[dims[1]]=0,base1=base0;index[dims[1]]<numVertices[dims[1]];++index[dims[1]],base1+=increments[dims[1]])
				{
				/* Read a span of voxels: */
				sPipe->read<PaletteRenderer::Voxel>(base1,numVertices[dims[2]]);
				}
			}
		}
	renderer->finishVoxelBlock();
	
	/* Set the renderer's model space position and size: */
	renderer->setPosition(boxOrigin,boxSize);
	
	/* Initialize volume renderer settings: */
	// renderer->setUseNPOTDTextures(true);
	renderer->setRenderingMode(PaletteRenderer::VIEW_PERPENDICULAR);
	renderer->setInterpolationMode(PaletteRenderer::LINEAR);
	renderer->setTextureFunction(PaletteRenderer::REPLACE);
	renderer->setSliceFactor(Scalar(2.0));
	renderer->setAutosaveGLState(true);
	renderer->setTextureCaching(true);
	renderer->setSharePalette(false);
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::~SliceVolumeRenderer(
	void)
	{
	/* Destroy the volume renderer: */
	delete renderer;
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
size_t
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::getSize(
	void) const
	{
	/* Return number of cells in volume renderer: */
	return size_t(renderer->getNumCells(0))*size_t(renderer->getNumCells(1))*size_t(renderer->getNumCells(2));
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::Scalar
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::getSliceFactor(
	void) const
	{
	/* Return slice factor from palette renderer: */
	return renderer->getSliceFactor();
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::setSliceFactor(
	typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::Scalar newSliceFactor)
	{
	/* Set slice factor of palette renderer: */
	renderer->setSliceFactor(newSliceFactor);
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::setTransparencyGamma(
	float newTransparencyGamma)
	{
	/* Set the transparency gamma correction factor: */
	transparencyGamma=newTransparencyGamma;
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::renderVolume(
	const typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::Point& sliceCenter,
	const typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::Vector& viewDirection,
	GLContextData& contextData) const
	{
	/* Set up OpenGL state: */
	GLboolean alphaTestEnabled=glIsEnabled(GL_ALPHA_TEST);
	if(!alphaTestEnabled)
		glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.0f);
	
	/* Process the color map: */
	GLColorMap privateMap(*colorMap);
	privateMap.changeTransparency(float(renderer->getSliceFactor())*transparencyGamma);
	privateMap.premultiplyAlpha();
	
	/* Render the volume: */
	renderer->setSliceCenter(sliceCenter);
	renderer->setColorMap(&privateMap);
	renderer->renderBlock(contextData,viewDirection);
	
	/* Reset OpenGL state: */
	if(!alphaTestEnabled)
		glDisable(GL_ALPHA_TEST);
	}

}

}
