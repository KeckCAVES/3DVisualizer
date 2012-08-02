/***********************************************************************
EvaluationLocator - Base class for locators evaluating properties of
data sets.
Copyright (c) 2006-2012 Oliver Kreylos

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

#include "EvaluationLocator.h"

#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSet.h>
#include <Abstract/DataSetRenderer.h>
#include <Abstract/CoordinateTransformer.h>

#include "GLRenderState.h"
#include "Visualizer.h"

/**********************************************
Methods of class Visualizer::EvaluationLocator:
**********************************************/

EvaluationLocator::EvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,const char* dialogWindowTitle)
	:BaseLocator(sLocatorTool,sApplication),
	 evaluationDialogPopup(0),
	 locator(application->dataSet->getLocator()),
	 dragging(false),hasPoint(false)
	{
	/* Create the evaluation dialog window: */
	evaluationDialogPopup=new GLMotif::PopupWindow("EvaluationDialogPopup",Vrui::getWidgetManager(),dialogWindowTitle);
	evaluationDialogPopup->setResizableFlags(true,false);
	
	evaluationDialog=new GLMotif::RowColumn("EvaluationDialog",evaluationDialogPopup,false);
	evaluationDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("PosLabel",evaluationDialog,"Position");
	
	GLMotif::RowColumn* posBox=new GLMotif::RowColumn("PosBox",evaluationDialog,false);
	posBox->setOrientation(GLMotif::RowColumn::VERTICAL);
	posBox->setNumMinorWidgets(3);
	posBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Component-%d",i+1);
		GLMotif::Label* posLabel=new GLMotif::Label(labelName,posBox,application->coordinateTransformer->getComponentName(i));
		posLabel->setHAlignment(GLFont::Center);
		}
	
	for(int i=0;i<3;++i)
		{
		char posName[40];
		snprintf(posName,sizeof(posName),"Pos-%d",i+1);
		pos[i]=new GLMotif::TextField(posName,posBox,12);
		pos[i]->setPrecision(6);
		}
	
	/* Size pos boxes evenly (simulate PACK_GRID layout horizontally): */
	for(int i=0;i<3;++i)
		posBox->setColumnWeight(i,1.0f);
	
	posBox->manageChild();
	}

EvaluationLocator::~EvaluationLocator(void)
	{
	/* Destroy the evaluation dialog: */
	delete evaluationDialogPopup;
	
	/* Destroy the locator: */
	delete locator;
	}

void EvaluationLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Update the locator: */
	locator->setPosition(cbData->currentTransformation.getOrigin());
	locator->setOrientation(cbData->currentTransformation.getRotation());
	
	if(dragging)
		{
		/* Get the current position of the locator in model coordinates: */
		point=locator->getPosition();
		hasPoint=true;
		
		/* Update the evaluation display dialog: */
		CoordinateTransformer::Point sourcePoint=application->coordinateTransformer->transformCoordinate(CoordinateTransformer::Point(point));
		for(int i=0;i<3;++i)
			pos[i]->setValue(double(sourcePoint[i]));
		}
	}

void EvaluationLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Create a new evaluation point and start dragging it: */
	dragging=true;
	}

void EvaluationLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Stop dragging the current evaluation point: */
	dragging=false;
	}

void EvaluationLocator::highlightLocator(GLRenderState& renderState) const
	{
	/* Highlight the locator: */
	if(locator->isValid())
		application->dataSetRenderer->highlightLocator(locator,renderState);
	
	/* Render the evaluation point: */
	if(hasPoint)
		{
		/* Set up OpenGL state: */
		renderState.setLineWidth(1.0f);
		renderState.setLighting(false);
		
		/* Calculate the marker size by querying the current navigation transformation: */
		Vrui::Scalar markerSize=(Vrui::Scalar(2)*Vrui::getUiSize())/Vrui::getNavigationTransformation().getScaling();
		
		/* Determine the marker color: */
		Vrui::Color bgColor=Vrui::getBackgroundColor();
		Vrui::Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		/* Draw the evaluation point position: */
		glColor(fgColor);
		glBegin(GL_LINES);
		glVertex(point[0]-markerSize,point[1],point[2]);
		glVertex(point[0]+markerSize,point[1],point[2]);
		glVertex(point[0],point[1]-markerSize,point[2]);
		glVertex(point[0],point[1]+markerSize,point[2]);
		glVertex(point[0],point[1],point[2]-markerSize);
		glVertex(point[0],point[1],point[2]+markerSize);
		glEnd();
		}
	}
