/***********************************************************************
DataSetRenderer - Wrapper class to map from the abstract data set
renderer interface to its templatized data set renderer implementation.
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

#define VISUALIZATION_WRAPPERS_DATASETRENDERER_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>

#include <Wrappers/DataSetRenderer.h>

namespace Visualization {

namespace Wrappers {

/******************************************
Methods of class DataSetRenderer::DataItem:
******************************************/

template <class DataSetWrapperParam>
inline
DataSetRenderer<DataSetWrapperParam>::DataItem::DataItem(
	void)
	:displayListId(glGenLists(1)),
	 displayVersion(0)
	{
	}

template <class DataSetWrapperParam>
inline
DataSetRenderer<DataSetWrapperParam>::DataItem::~DataItem(
	void)
	{
	glDeleteLists(displayListId,1);
	}

/********************************
Methods of class DataSetRenderer:
********************************/

template <class DataSetWrapperParam>
inline
const typename DataSetRenderer<DataSetWrapperParam>::DS*
DataSetRenderer<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("DataSetRenderer::DataSetRenderer: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
DataSetRenderer<DataSetWrapperParam>::DataSetRenderer(
	const Visualization::Abstract::DataSet* sDataSet)
	:dsr(getDs(sDataSet)),
	 displayVersion(1)
	{
	}

template <class DataSetWrapperParam>
inline
DataSetRenderer<DataSetWrapperParam>::DataSetRenderer(
	const DataSetRenderer<DataSetWrapperParam>& source)
	:Base(source),
	 dsr(source.dsr),
	 displayVersion(1)
	{
	}

template <class DataSetWrapperParam>
inline
DataSetRenderer<DataSetWrapperParam>::~DataSetRenderer(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
DataSetRenderer<DataSetWrapperParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the initial data set rendering into the display list: */
	glNewList(dataItem->displayListId,GL_COMPILE_AND_EXECUTE);
	dsr.glRenderAction(contextData);
	glEndList();
	dataItem->displayVersion=displayVersion;
	}

template <class DataSetWrapperParam>
inline
typename DataSetRenderer<DataSetWrapperParam>::Base*
DataSetRenderer<DataSetWrapperParam>::clone(
	void) const
	{
	return new DataSetRenderer(*this);
	}

template <class DataSetWrapperParam>
inline
int
DataSetRenderer<DataSetWrapperParam>::getNumRenderingModes(
	void) const
	{
	return DSR::getNumRenderingModes();
	}

template <class DataSetWrapperParam>
inline
const char*
DataSetRenderer<DataSetWrapperParam>::getRenderingModeName(
	int renderingModeIndex) const
	{
	return DSR::getRenderingModeName(renderingModeIndex);
	}

template <class DataSetWrapperParam>
inline
int
DataSetRenderer<DataSetWrapperParam>::getRenderingMode(
	void) const
	{
	return dsr.getRenderingMode();
	}

template <class DataSetWrapperParam>
inline
void
DataSetRenderer<DataSetWrapperParam>::setRenderingMode(
	int renderingModeIndex)
	{
	dsr.setRenderingMode(renderingModeIndex);
	++displayVersion;
	}

template <class DataSetWrapperParam>
inline
void
DataSetRenderer<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Retrieve the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLboolean blendingEnabled=glIsEnabled(GL_BLEND);
	if(!blendingEnabled)
		glEnable(GL_BLEND);
	GLint blendSrc,blendDst;
	glGetIntegerv(GL_BLEND_SRC,&blendSrc);
	glGetIntegerv(GL_BLEND_DST,&blendDst);
	if(blendSrc!=GL_SRC_ALPHA||blendDst!=GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	GLboolean depthMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);
	if(depthMask)
		glDepthMask(GL_FALSE);
	
	if(displayVersion!=dataItem->displayVersion)
		{
		/* Upload the new data set rendering into the display list: */
		glNewList(dataItem->displayListId,GL_COMPILE_AND_EXECUTE);
		dsr.glRenderAction(contextData);
		glEndList();
		dataItem->displayVersion=displayVersion;
		}
	else
		{
		/* Render the display list: */
		glCallList(dataItem->displayListId);
		}
	
	/* Reset OpenGL state: */
	if(depthMask)
		glDepthMask(GL_TRUE);
	if(blendSrc!=GL_ONE||blendDst!=GL_ONE)
		glBlendFunc(blendSrc,blendDst);
	if(!blendingEnabled)
		glDisable(GL_BLEND);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}

template <class DataSetWrapperParam>
inline
void
DataSetRenderer<DataSetWrapperParam>::highlightLocator(
	const Visualization::Abstract::DataSet::Locator* locator,
	GLContextData& contextData) const
	{
	const typename DataSetWrapper::Locator* myLocator=dynamic_cast<const typename DataSetWrapper::Locator*>(locator);
	if(myLocator==0)
		Misc::throwStdErr("DataSetRenderer::highlightLocator: Mismatching locator type");
	
	/* Set OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	if(lineWidth!=1.0f)
		glLineWidth(1.0f);
	glColor3f(0.0f,1.0f,0.0f);
	
	/* Highlight the locator's cell in the data set: */
	dsr.renderCell(myLocator->getDsl().getCellID(),contextData);
	
	/* Reset OpenGL state: */
	if(lineWidth!=1.0f)
		glLineWidth(lineWidth);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}

}

}
