/***********************************************************************
SliceVolumeRendererCartesian - Specialized texture-based version of
SliceVolumeRenderer class for Cartesian data sets.
Copyright (c) 2006-2007 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERERCARTESIAN_IMPLEMENTATION

#include <GL/gl.h>
#include <GL/GLColorMap.h>
#include <VolumeRenderer.h>
#include <PaletteRenderer.h>

#include <Templatized/Cartesian.h>
#include <Templatized/SliceVolumeRenderer.h>

namespace Visualization {

namespace Templatized {

/************************************
Methods of class SliceVolumeRenderer:
************************************/

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::SliceVolumeRenderer(
	const typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::DataSet* sDataSet,
	const typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::ScalarExtractor& sScalarExtractor,
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
	
	/* Create a voxel block: */
	int size[3];
	for(int i=0;i<3;++i)
		size[i]=dataSet->getNumVertices()[i];
	PaletteRenderer::Voxel* voxels;
	int increments[3];
	voxels=renderer->createVoxelBlock(size,0,PaletteRenderer::VERTEX_CENTERED,increments);
	
	/* Upload the data set's scalar values into the voxel block: */
	for(typename DataSet::Index index(0);index[0]<dataSet->getNumVertices()[0];index.preInc(dataSet->getNumVertices()))
		{
		/* Get the vertex' scalar value: */
		VScalar value=scalarExtractor.getValue(dataSet->getVertexValue(index));
		
		/* Convert the value to unsigned char: */
		PaletteRenderer::Voxel& voxel=voxels[index[0]*increments[0]+index[1]*increments[1]+index[2]*increments[2]];
		voxel=PaletteRenderer::Voxel(Math::floor((value-minValue)*VScalar(255)/(maxValue-minValue)+VScalar(0.5)));
		}
	renderer->finishVoxelBlock();
	
	/* Set the renderer's model space position and size: */
	renderer->setPosition(dataSet->getDomainBox().getOrigin(),dataSet->getDomainBox().getSize());
	
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

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::~SliceVolumeRenderer(
	void)
	{
	/* Destroy the volume renderer: */
	delete renderer;
	}

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
size_t
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::getSize(
	void) const
	{
	/* Return number of cells in volume renderer: */
	return size_t(renderer->getNumCells(0))*size_t(renderer->getNumCells(1))*size_t(renderer->getNumCells(2));
	}

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::Scalar
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::getSliceFactor(
	void) const
	{
	/* Return slice factor from palette renderer: */
	return renderer->getSliceFactor();
	}

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::setSliceFactor(
	typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::Scalar newSliceFactor)
	{
	/* Set slice factor of palette renderer: */
	renderer->setSliceFactor(newSliceFactor);
	}

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::setTransparencyGamma(
	float newTransparencyGamma)
	{
	/* Set the transparency gamma correction factor: */
	transparencyGamma=newTransparencyGamma;
	}

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::renderVolume(
	const typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::Point& sliceCenter,
	const typename SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>::Vector& viewDirection,
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
