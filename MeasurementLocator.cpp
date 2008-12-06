/***********************************************************************
MeasurementLocator - Class for locators measuring spatial properties of
data sets.
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

#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

#include "Visualizer.h"

/***********************************************
Methods of class Visualizer::MeasurementLocator:
***********************************************/

Visualizer::MeasurementLocator::MeasurementLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication)
	:BaseLocator(sLocatorTool,sApplication),
	 measurementDialogPopup(0),
	 numPoints(0),dragging(false)
	{
	/* Create the measurement dialog window: */
	measurementDialogPopup=new GLMotif::PopupWindow("MeasurementDialogPopup",Vrui::getWidgetManager(),"Measurement Dialog");
	
	GLMotif::RowColumn* measurementDialog=new GLMotif::RowColumn("MeasurementDialog",measurementDialogPopup,false);
	measurementDialog->setNumMinorWidgets(2);
	
	GLMotif::Label* pos1Label=new GLMotif::Label("Pos1Label",measurementDialog,"Position 1");
	
	GLMotif::RowColumn* pos1Box=new GLMotif::RowColumn("Pos1Box",measurementDialog,false);
	pos1Box->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	pos1Box->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Pos1-%d",i+1);
		pos1[i]=new GLMotif::TextField(labelName,pos1Box,8);
		pos1[i]->setFieldWidth(8);
		pos1[i]->setPrecision(4);
		}
	
	pos1Box->manageChild();
	
	GLMotif::Label* pos2Label=new GLMotif::Label("Pos2Label",measurementDialog,"Position 2");
	
	GLMotif::RowColumn* pos2Box=new GLMotif::RowColumn("Pos2Box",measurementDialog,false);
	pos2Box->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	pos2Box->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Pos2-%d",i+1);
		pos2[i]=new GLMotif::TextField(labelName,pos2Box,8);
		pos2[i]->setFieldWidth(8);
		pos2[i]->setPrecision(4);
		}
	
	pos2Box->manageChild();
	
	GLMotif::Label* distLabel=new GLMotif::Label("DistLabel",measurementDialog,"Distance");
	
	dist=new GLMotif::TextField("Dist",measurementDialog,8);
	dist->setFieldWidth(8);
	dist->setPrecision(4);
	
	measurementDialog->manageChild();
	
	/* Pop up the measurement dialog: */
	Vrui::popupPrimaryWidget(measurementDialogPopup,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	}

Visualizer::MeasurementLocator::~MeasurementLocator(void)
	{
	/* Pop down the measurement dialog: */
	Vrui::popdownPrimaryWidget(measurementDialogPopup);
	
	/* Destroy the measurement dialog: */
	delete measurementDialogPopup;
	}

void Visualizer::MeasurementLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	if(dragging)
		{
		/* Get the current position of the locator in model coordinates: */
		points[numPoints-1]=cbData->currentTransformation.getOrigin();
		
		/* Update the measurement display dialog: */
		if(numPoints>=1)
			for(int i=0;i<3;++i)
				pos1[i]->setValue(double(points[0][i]));
		if(numPoints>=2)
			{
			for(int i=0;i<3;++i)
				pos2[i]->setValue(double(points[1][i]));
			
			/* Calculate the distance between the two measurement points: */
			Vrui::Scalar distance=Geometry::dist(points[0],points[1]);
			dist->setValue(double(distance));
			}
		}
	}

void Visualizer::MeasurementLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Reset the last measurement if it was completed: */
	if(numPoints==2)
		{
		numPoints=0;
		for(int i=0;i<3;++i)
			pos2[i]->setLabel("");
		dist->setLabel("");
		}
	
	/* Create a new measurement point and start dragging it: */
	++numPoints;
	dragging=true;
	}

void Visualizer::MeasurementLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Stop dragging the current measurement point: */
	dragging=false;
	}

void Visualizer::MeasurementLocator::highlightLocator(GLContextData& contextData) const
	{
	/* Set up and save OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	glLineWidth(1.0f);
	
	/* Calculate the marker size by querying the current navigation transformation: */
	Vrui::Scalar markerSize=Vrui::getUiSize()/Vrui::getNavigationTransformation().getScaling();
	
	/* Draw the measurement point positions: */
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_LINES);
	for(int i=0;i<numPoints;++i)
		{
		glVertex(points[i][0]-markerSize,points[i][1],points[i][2]);
		glVertex(points[i][0]+markerSize,points[i][1],points[i][2]);
		glVertex(points[i][0],points[i][1]-markerSize,points[i][2]);
		glVertex(points[i][0],points[i][1]+markerSize,points[i][2]);
		glVertex(points[i][0],points[i][1],points[i][2]-markerSize);
		glVertex(points[i][0],points[i][1],points[i][2]+markerSize);
		}
	if(numPoints==2)
		{
		glVertex(points[0]);
		glVertex(points[1]);
		}
	glEnd();
	
	/* Restore OpenGL state: */
	glLineWidth(lineWidth);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}
