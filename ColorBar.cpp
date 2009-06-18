/***********************************************************************
ColorBar - A widget to display color bars with tick marks and numerical
values.
Copyright (c) 2008 Oliver Kreylos

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

#include "ColorBar.h"

#include <string.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLTexEnvTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLColorMap.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/***********************************
Methods of class ColorBar::DataItem:
***********************************/

ColorBar::DataItem::DataItem(int sNumTickMarks)
	:numTickMarks(sNumTickMarks),
	 textureObjectIds(new GLuint[numTickMarks]),
	 tickMarksVersion(0)
	{
	glGenTextures(numTickMarks,textureObjectIds);
	}

ColorBar::DataItem::~DataItem(void)
	{
	glDeleteTextures(numTickMarks,textureObjectIds);
	delete[] textureObjectIds;
	}

/*************************
Methods of class ColorBar:
*************************/

void ColorBar::updateTickMarks(void)
	{
	/* Calculate the size of all tick mark labels: */
	tickMarkLabelHeight=0.0f;
	for(int i=0;i<numTickMarks;++i)
		{
		/* Calculate the tick mark value: */
		double value=valueMin+(valueMax-valueMin)*double(i)/double(numTickMarks-1);
		
		/* Create the tick mark label: */
		char label[20];
		int labelLen=snprintf(label,sizeof(label),"%.*g",tickMarkLabelPrecision,value);
		delete[] tickMarks[i].label;
		tickMarks[i].label=new char[labelLen+1];
		memcpy(tickMarks[i].label,label,labelLen+1);
		
		/* Calculate the tick mark label's size and texture coordinates: */
		tickMarks[i].labelBox=font->calcStringBox(tickMarks[i].label);
		tickMarks[i].labelTexCoords=font->calcStringTexCoords(tickMarks[i].label);
		
		if(tickMarkLabelHeight<tickMarks[i].labelBox.size[1])
			tickMarkLabelHeight=tickMarks[i].labelBox.size[1];
		}
	
	++tickMarksVersion;
	}

void ColorBar::layout(void)
	{
	/* Get the interior size of the widget: */
	Box inner=getInterior();
	inner.inset(Vector(marginWidth,marginWidth,0.0f));
	
	/* Align the vertical boxes: */
	tickMarkLabelBox=inner;
	tickMarkLabelBox.size[1]=tickMarkLabelHeight;
	colorBarBox=inner;
	colorBarBox.origin[1]+=tickMarkLabelBox.size[1]+tickMarkHeight;
	colorBarBox.size[1]-=tickMarkLabelBox.size[1]+tickMarkHeight;
	
	/* Distribute the tick mark labels uniformly: */
	GLfloat totalTickMarkLabelWidth=0.0f;
	for(int i=0;i<numTickMarks;++i)
		{
		/* Center the tick mark label under its tick mark: */
		GLfloat x=tickMarkLabelBox.origin[0]+tickMarkLabelBox.size[0]*GLfloat(i)/GLfloat(numTickMarks-1);
		tickMarks[i].labelBox.origin[0]=x-tickMarks[i].labelBox.size[0]*0.5f;
		tickMarks[i].labelBox.origin[1]=tickMarkLabelBox.origin[1]+(tickMarkLabelHeight-tickMarks[i].labelBox.size[1])*0.5f;
		totalTickMarkLabelWidth+=tickMarks[i].labelBox.size[0];
		}
	
	/* Shift the tick mark labels until they fit into the alloted box and don't overlap: */
	GLfloat minSeparation=(tickMarkLabelBox.size[0]-totalTickMarkLabelWidth)/GLfloat(numTickMarks-1);
	if(minSeparation>tickMarkLabelSeparation)
		minSeparation=tickMarkLabelSeparation;
	GLfloat left=tickMarkLabelBox.origin[0];
	GLfloat right=tickMarkLabelBox.origin[0]+tickMarkLabelBox.size[0];
	for(int i=0;i<numTickMarks/2;++i)
		{
		/* Align i-th tick mark label from the left: */
		if(tickMarks[i].labelBox.origin[0]<left)
			tickMarks[i].labelBox.origin[0]=left;
		left=tickMarks[i].labelBox.origin[0]+tickMarks[i].labelBox.size[0]+minSeparation;
		
		/* Align i-th tick mark label from the right: */
		if(tickMarks[numTickMarks-i-1].labelBox.origin[0]>right-tickMarks[numTickMarks-i-1].labelBox.size[0])
			tickMarks[numTickMarks-i-1].labelBox.origin[0]=right-tickMarks[numTickMarks-i-1].labelBox.size[0];
		right=tickMarks[numTickMarks-i-1].labelBox.origin[0]-minSeparation;
		}
	}

ColorBar::ColorBar(const char* sName,Container* sParent,GLfloat sColorBarHeight,int sTickMarkLabelPrecision,int sNumTickMarks,bool sManageChild)
	:Widget(sName,sParent,false),
	 colorBarHeight(sColorBarHeight),
	 valueMin(0.0),valueMax(1.0),
	 colorMap(0),
	 font(0),
	 tickMarkLabelPrecision(sTickMarkLabelPrecision),
	 numTickMarks(sNumTickMarks),tickMarks(new TickMark[numTickMarks]),
	 tickMarksVersion(1)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Get the margin width: */
	marginWidth=ss->containerMarginWidth;
	
	/* Get the font: */
	font=ss->font;
	
	/* Get the tick mark height: */
	tickMarkHeight=font->getTextHeight();
	tickMarkWidth=tickMarkHeight*0.5f;
	
	/* Calculate the tick mark label separation: */
	tickMarkLabelSeparation=font->getCharacterWidth()*2.0f;
	
	/* Initialize the tick marks: */
	updateTickMarks();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ColorBar::~ColorBar(void)
	{
	delete[] tickMarks;
	}

Vector ColorBar::calcNaturalSize(void) const
	{
	Vector result;
	
	/* Calculate the natural width of the widget: */
	result[0]=GLfloat(numTickMarks)*font->getCharacterWidth()*GLfloat(tickMarkLabelPrecision+6)+GLfloat(numTickMarks-1)*tickMarkLabelSeparation+2.0f*marginWidth;
	result[1]=font->getTextHeight()+tickMarkHeight+colorBarHeight+2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

void ColorBar::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Lay out the widget: */
	layout();
	}

void ColorBar::draw(GLContextData& contextData) const
	{
	/* Retrieve the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Draw parent class decorations: */
	Widget::draw(contextData);
	
	/* Draw the widget margin: */
	glColor(backgroundColor);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(1));
	for(int i=numTickMarks-1;i>=0;--i)
		{
		glVertex(tickMarks[i].labelBox.getCorner(1));
		glVertex(tickMarks[i].labelBox.getCorner(0));
		}
	glVertex(tickMarks[0].labelBox.getCorner(2));
	glVertex(tickMarkLabelBox.getCorner(2));
	glVertex(colorBarBox.getCorner(0));
	glVertex(colorBarBox.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(3));
	glVertex(getInterior().getCorner(2));
	glVertex(colorBarBox.getCorner(2));
	glVertex(colorBarBox.getCorner(3));
	glVertex(colorBarBox.getCorner(1));
	glVertex(tickMarkLabelBox.getCorner(3));
	glVertex(tickMarks[numTickMarks-1].labelBox.getCorner(3));
	glVertex(tickMarks[numTickMarks-1].labelBox.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glEnd();
	
	/* Draw the spaces between the tick mark labels: */
	glBegin(GL_QUADS);
	for(int i=1;i<numTickMarks;++i)
		{
		glVertex(tickMarks[i-1].labelBox.getCorner(3));
		glVertex(tickMarks[i-1].labelBox.getCorner(1));
		glVertex(tickMarks[i].labelBox.getCorner(0));
		glVertex(tickMarks[i].labelBox.getCorner(2));
		}
	glEnd();
	
	/* Draw the border between tick marks and tick mark labels: */
	glBegin(GL_QUAD_STRIP);
	GLfloat tickBot=tickMarkLabelBox.origin[1]+tickMarkLabelBox.size[1];
	glVertex(tickMarkLabelBox.getCorner(2));
	glVertex(tickMarks[0].labelBox.getCorner(2));
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	glVertex(tickMarks[0].labelBox.getCorner(3));
	for(int i=1;i<numTickMarks-1;++i)
		{
		GLfloat x=colorBarBox.origin[0]+colorBarBox.size[0]*GLfloat(i)/GLfloat(numTickMarks-1);
		glVertex3f(x-tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		glVertex(tickMarks[i].labelBox.getCorner(2));
		glVertex3f(x+tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		glVertex(tickMarks[i].labelBox.getCorner(3));
		}
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkLabelBox.size[0]-tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	glVertex(tickMarks[numTickMarks-1].labelBox.getCorner(2));
	glVertex(tickMarkLabelBox.getCorner(3));
	glVertex(tickMarks[numTickMarks-1].labelBox.getCorner(3));
	glEnd();
	
	/* Draw the spaces between tick marks: */
	glBegin(GL_QUADS);
	GLfloat tickTop=colorBarBox.origin[1];
	glVertex(colorBarBox.getCorner(0));
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	for(int i=1;i<numTickMarks-1;++i)
		{
		GLfloat x=colorBarBox.origin[0]+colorBarBox.size[0]*GLfloat(i)/GLfloat(numTickMarks-1);
		glVertex3f(x-tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		glVertex3f(x,tickTop,colorBarBox.origin[2]);
		glVertex3f(x,tickTop,colorBarBox.origin[2]);
		glVertex3f(x+tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		}
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkLabelBox.size[0]-tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	glVertex(colorBarBox.getCorner(1));
	glEnd();
	
	/* Draw the tick marks: */
	glBegin(GL_TRIANGLES);
	glColor(foregroundColor);
	glVertex(tickMarkLabelBox.getCorner(2));
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	glVertex(colorBarBox.getCorner(0));
	for(int i=1;i<numTickMarks-1;++i)
		{
		GLfloat x=colorBarBox.origin[0]+colorBarBox.size[0]*GLfloat(i)/GLfloat(numTickMarks-1);
		glVertex3f(x-tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		glVertex3f(x+tickMarkWidth*0.5f,tickBot,tickMarkLabelBox.origin[2]);
		glVertex3f(x,tickTop,colorBarBox.origin[2]);
		}
	glVertex3f(tickMarkLabelBox.origin[0]+tickMarkLabelBox.size[0]-tickMarkWidth,tickBot,tickMarkLabelBox.origin[2]);
	glVertex(tickMarkLabelBox.getCorner(3));
	glVertex(colorBarBox.getCorner(1));
	glEnd();
	
	/* Set up OpenGL state for color bar rendering: */
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
	
	/* Upload the color map as a 1D texture: */
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA8,256,0,GL_RGBA,GL_FLOAT,colorMap->getColors());
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE,&matrixMode);
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glScaled(1.0/(valueMax-valueMin),1.0,1.0);
	glTranslated(-valueMin,0.0,0.0);
	
	/* Draw the color bar: */
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glTexCoord1d(valueMin);
	glVertex(colorBarBox.getCorner(2));
	glVertex(colorBarBox.getCorner(0));
	for(int i=1;i<numTickMarks-1;++i)
		{
		GLfloat x=colorBarBox.origin[0]+colorBarBox.size[0]*GLfloat(i)/GLfloat(numTickMarks-1);
		glTexCoord1d(valueMin+(valueMax-valueMin)*double(i)/double(numTickMarks-1));
		glVertex3f(x,tickTop,colorBarBox.origin[2]);
		}	
	glTexCoord1d(valueMax);
	glVertex(colorBarBox.getCorner(1));
	glVertex(colorBarBox.getCorner(3));
	glEnd();
	
	/* Reset OpenGL state: */
	glPopMatrix();
	if(matrixMode!=GL_TEXTURE)
		glMatrixMode(matrixMode);
	if(texture3DEnabled)
		glEnable(GL_TEXTURE_3D);
	if(texture2DEnabled)
		glEnable(GL_TEXTURE_2D);
	if(!texture1DEnabled)
		glDisable(GL_TEXTURE_1D);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	
	/* Draw the tick mark labels: */
	glPushAttrib(GL_TEXTURE_BIT);
	GLint lightModelColorControl;
	glGetIntegerv(GL_LIGHT_MODEL_COLOR_CONTROL,&lightModelColorControl);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	glEnable(GL_TEXTURE_2D);
	for(int i=0;i<numTickMarks;++i)
		{
		glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIds[i]);
		if(dataItem->tickMarksVersion!=tickMarksVersion) // Have the tick marks changed?
			{
			/* Upload the tick mark label string texture again: */
			font->uploadStringTexture(tickMarks[i].label,backgroundColor,foregroundColor);
			}
		glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,GLTexEnvEnums::MODULATE);
		glColor4f(1.0f,1.0f,1.0f,backgroundColor[3]);
		glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,1.0f);
		glTexCoord(tickMarks[i].labelTexCoords.getCorner(0));
		glVertex(tickMarks[i].labelBox.getCorner(0));
		glTexCoord(tickMarks[i].labelTexCoords.getCorner(1));
		glVertex(tickMarks[i].labelBox.getCorner(1));
		glTexCoord(tickMarks[i].labelTexCoords.getCorner(3));
		glVertex(tickMarks[i].labelBox.getCorner(3));
		glTexCoord(tickMarks[i].labelTexCoords.getCorner(2));
		glVertex(tickMarks[i].labelBox.getCorner(2));
		glEnd();
		}
	dataItem->tickMarksVersion=tickMarksVersion;
	glBindTexture(GL_TEXTURE_2D,0);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,lightModelColorControl);
	glPopAttrib();
	}

void ColorBar::initContext(GLContextData& contextData) const
	{
	/* Create a context data item: */
	DataItem* dataItem=new DataItem(numTickMarks);
	contextData.addDataItem(this,dataItem);
	}

void ColorBar::setColorMap(GLColorMap* newColorMap)
	{
	/* Set the color map: */
	colorMap=newColorMap;
	}

void ColorBar::setValueRange(double newValueMin,double newValueMax)
	{
	/* Set the new value range: */
	valueMin=newValueMin;
	valueMax=newValueMax;
	
	/* Update the tick marks: */
	updateTickMarks();
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new label: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
