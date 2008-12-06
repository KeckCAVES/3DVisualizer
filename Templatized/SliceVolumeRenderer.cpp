/***********************************************************************
SliceVolumeRenderer - Generic class to render volumetric images of data
sets using incremental slices.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <GL/gl.h>
#include <GL/GLColorMap.h>

#include <Templatized/SliceVolumeRenderer.h>

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
	:se(sDataSet,sScalarExtractor),
	 sliceFactor(2.0),
	 transparencyGamma(1.0f),
	 colorMap(sColorMap)
	{
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::~SliceVolumeRenderer(
	void)
	{
	}

template <class DataSetParam,class ScalarExtractorParam>
inline
void
SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::setSliceFactor(
	typename SliceVolumeRenderer<DataSetParam,ScalarExtractorParam>::Scalar newSliceFactor)
	{
	/* Set the slice factor: */
	sliceFactor=newSliceFactor;
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
	/* Determine the extents of the data set along the view direction: */
	const DataSet* ds=se.getDataSet();
	Scalar min,max;
	typename DataSet::VertexIterator vIt=ds->beginVertices();
	min=max=(vIt->getPosition()-sliceCenter)*viewDirection;
	for(++vIt;vIt!=ds->endVertices();++vIt)
		{
		Scalar d=(vIt->getPosition()-sliceCenter)*viewDirection;
		if(min>d)
			min=d;
		else if(max<d)
			max=d;
		}
	
	/* Fudge the slice distance: */
	Scalar sliceDistance=(max-min)*sliceFactor/Scalar(200);
	
	/* Set up OpenGL state: */
	GLboolean cullFaceEnabled=glIsEnabled(GL_CULL_FACE);
	if(cullFaceEnabled)
		glDisable(GL_CULL_FACE);
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLboolean texture1DEnabled=glIsEnabled(GL_TEXTURE_1D);
	if(!texture1DEnabled)
		glEnable(GL_TEXTURE_1D);
	GLboolean texture2DEnabled=glIsEnabled(GL_TEXTURE_2D);
	if(texture2DEnabled)
		glDisable(GL_TEXTURE_2D);
	GLboolean texture3DEnabled=glIsEnabled(GL_TEXTURE_3D);
	if(texture3DEnabled)
		glDisable(GL_TEXTURE_3D);
	GLboolean blendEnabled=glIsEnabled(GL_BLEND);
	if(!blendEnabled)
		glEnable(GL_BLEND);
	GLint blendSrc,blendDst;
	glGetIntegerv(GL_BLEND_SRC,&blendSrc);
	glGetIntegerv(GL_BLEND_DST,&blendDst);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	GLboolean depthMaskEnabled;
	glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMaskEnabled);
	if(depthMaskEnabled)
		glDepthMask(GL_FALSE);
	
	/* Process the color map: */
	GLColorMap privateMap(*colorMap);
	privateMap.changeTransparency(float(sliceFactor)*transparencyGamma);
	privateMap.premultiplyAlpha();
	
	/* Upload the color map as a 1D texture: */
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA8,256,0,GL_RGBA,GL_FLOAT,privateMap.getColors());
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE,&matrixMode);
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	double mapMin=colorMap->getScalarRangeMin();
	double mapRange=colorMap->getScalarRangeMax()-mapMin;
	glScaled(1.0/mapRange,1.0,1.0);
	glTranslated(-mapMin,0.0,0.0);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	{
	/* Render all slices in back-to-front order: */
	TR tr;
	for(Scalar sliceD=Math::floor(max/sliceDistance)*sliceDistance;sliceD>min;sliceD-=sliceDistance)
		se.extractSlice(Plane(viewDirection,sliceCenter+viewDirection*sliceD),tr);
	}
	
	/* Reset OpenGL state: */
	glPopMatrix();
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(matrixMode);
	if(depthMaskEnabled)
		glDepthMask(GL_TRUE);
	glBlendFunc(blendSrc,blendDst);
	if(!blendEnabled)
		glDisable(GL_BLEND);
	if(texture3DEnabled)
		glEnable(GL_TEXTURE_3D);
	if(texture2DEnabled)
		glEnable(GL_TEXTURE_2D);
	if(!texture1DEnabled)
		glDisable(GL_TEXTURE_1D);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	if(cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	}

}

}
