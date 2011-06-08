/***********************************************************************
PaletteEditor - Class to represent a GLMotif popup window to edit
one-dimensional transfer functions with RGB color and opacity.
Copyright (c) 2005-2010 Oliver Kreylos

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

#include "PaletteEditor.h"

#include <stdio.h>
#include <Misc/File.h>
#include <GL/GLColorMap.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Slider.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

/******************************
Methods of class PaletteEditor:
******************************/

void PaletteEditor::selectedControlPointChangedCallback(Misc::CallbackData* cbData)
	{
	GLMotif::ColorMap::SelectedControlPointChangedCallbackData* cbData2=static_cast<GLMotif::ColorMap::SelectedControlPointChangedCallbackData*>(cbData);
	
	if(cbData2->newSelectedControlPoint!=0)
		{
		/* Copy the selected control point's data and color value to the color editor: */
		controlPointValue->setValue(colorMap->getSelectedControlPointValue());
		controlPointValue->setEditable(true);
		if(controlPointValue->hasFocus())
			controlPointValue->setSelection(0,0);
		GLMotif::ColorMap::ColorMapValue colorValue=colorMap->getSelectedControlPointColorValue();
		colorPanel->setBackgroundColor(colorValue);
		for(int i=0;i<4;++i)
			colorSliders[i]->setValue(colorValue[i]);
		}
	else
		{
		controlPointValue->setString("");
		controlPointValue->setEditable(false);
		colorPanel->setBackgroundColor(GLMotif::Color(0.5f,0.5f,0.5f));
		for(int i=0;i<4;++i)
			colorSliders[i]->setValue(0.5);
		}
	}

void PaletteEditor::colorMapChangedCallback(Misc::CallbackData* cbData)
	{
	if(colorMap->hasSelectedControlPoint())
		{
		/* Copy the updated value of the selected control point to the color editor: */
		controlPointValue->setValue(colorMap->getSelectedControlPointValue());
		if(controlPointValue->hasFocus())
			controlPointValue->setSelection(0,0);
		colorSliders[3]->setValue(colorMap->getSelectedControlPointColorValue()[3]);
		}
	}

void PaletteEditor::controlPointValueChangedCallback(Misc::CallbackData* cbData)
	{
	if(colorMap->hasSelectedControlPoint())
		{
		/* Update the selected control point's value: */
		colorMap->setSelectedControlPointValue(atof(controlPointValue->getString()));
		controlPointValue->setValue(colorMap->getSelectedControlPointValue());
		}
	}

void PaletteEditor::colorSliderValueChangedCallback(Misc::CallbackData* cbData)
	{
	/* Calculate the new selected control point color: */
	GLMotif::ColorMap::ColorMapValue newColor;
	for(int i=0;i<4;++i)
		newColor[i]=colorSliders[i]->getValue();
	
	/* Copy the new color value to the color panel and the selected control point: */
	colorPanel->setBackgroundColor(newColor);
	colorMap->setSelectedControlPointColorValue(newColor);
	}

void PaletteEditor::removeControlPointCallback(Misc::CallbackData* cbData)
	{
	/* Remove the currently selected control point: */
	colorMap->deleteSelectedControlPoint();
	}

void PaletteEditor::savePaletteCallback(Misc::CallbackData* cbData)
	{
	/* Delegate the actual saving to someone else: */
	CallbackData myCbData(this);
	savePaletteCallbacks.call(&myCbData);
	}

PaletteEditor::PaletteEditor(void)
	:GLMotif::PopupWindow("PaletteEditorPopup",Vrui::getWidgetManager(),"Palette Editor"),
	 colorMap(0),controlPointValue(0),colorPanel(0)
	{
	for(int i=0;i<4;++i)
		colorSliders[i]=0;
	
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create the palette editor GUI: */
	GLMotif::RowColumn* colorMapDialog=new GLMotif::RowColumn("ColorMapDialog",this,false);
	
	colorMap=new GLMotif::ColorMap("ColorMap",colorMapDialog);
	colorMap->setBorderWidth(ss.size*0.5f);
	colorMap->setBorderType(GLMotif::Widget::LOWERED);
	colorMap->setForegroundColor(GLMotif::Color(0.0f,1.0f,0.0f));
	colorMap->setMarginWidth(ss.size);
	colorMap->setPreferredSize(GLMotif::Vector(ss.fontHeight*20.0,ss.fontHeight*10.0,0.0f));
	colorMap->setControlPointSize(ss.size);
	colorMap->setSelectedControlPointColor(GLMotif::Color(1.0f,0.0f,0.0f));
	colorMap->getSelectedControlPointChangedCallbacks().add(this,&PaletteEditor::selectedControlPointChangedCallback);
	colorMap->getColorMapChangedCallbacks().add(this,&PaletteEditor::colorMapChangedCallback);
	
	/* Create the RGB color editor: */
	GLMotif::RowColumn* colorEditor=new GLMotif::RowColumn("ColorEditor",colorMapDialog,false);
	colorEditor->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	colorEditor->setAlignment(GLMotif::Alignment::LEFT);
	
	GLMotif::RowColumn* controlPointData=new GLMotif::RowColumn("ControlPointData",colorEditor,false);
	controlPointData->setOrientation(GLMotif::RowColumn::VERTICAL);
	controlPointData->setNumMinorWidgets(2);
	
	new GLMotif::Label("ControlPointValueLabel",controlPointData,"Control Point Value");
	
	controlPointValue=new GLMotif::TextField("ControlPointValue",controlPointData,12);
	controlPointValue->setPrecision(6);
	controlPointValue->setString("");
	controlPointValue->getValueChangedCallbacks().add(this,&PaletteEditor::controlPointValueChangedCallback);
	
	new GLMotif::Label("ColorEditorLabel",controlPointData,"Control Point Color");
	
	colorPanel=new GLMotif::Blind("ColorPanel",controlPointData);
	colorPanel->setBorderWidth(ss.size*0.5f);
	colorPanel->setBorderType(GLMotif::Widget::LOWERED);
	colorPanel->setBackgroundColor(GLMotif::Color(0.5f,0.5f,0.5f));
	colorPanel->setPreferredSize(GLMotif::Vector(ss.fontHeight*2.5f,ss.fontHeight*2.5f,0.0f));
	
	controlPointData->manageChild();
	
	GLMotif::RowColumn* colorSlidersBox=new GLMotif::RowColumn("ColorSliders",colorEditor,false);
	colorSlidersBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	colorSlidersBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	colorSliders[0]=new GLMotif::Slider("RedSlider",colorSlidersBox,GLMotif::Slider::VERTICAL,ss.fontHeight*5.0f);
	colorSliders[0]->setSliderColor(GLMotif::Color(1.0f,0.0f,0.0f));
	colorSliders[0]->setValueRange(0.0,1.0,0.01);
	colorSliders[0]->setValue(0.5);
	colorSliders[0]->getValueChangedCallbacks().add(this,&PaletteEditor::colorSliderValueChangedCallback);
	
	colorSliders[1]=new GLMotif::Slider("GreenSlider",colorSlidersBox,GLMotif::Slider::VERTICAL,ss.fontHeight*5.0f);
	colorSliders[1]->setSliderColor(GLMotif::Color(0.0f,1.0f,0.0f));
	colorSliders[1]->setValueRange(0.0,1.0,0.01);
	colorSliders[1]->setValue(0.5);
	colorSliders[1]->getValueChangedCallbacks().add(this,&PaletteEditor::colorSliderValueChangedCallback);
	
	colorSliders[2]=new GLMotif::Slider("BlueSlider",colorSlidersBox,GLMotif::Slider::VERTICAL,ss.fontHeight*5.0f);
	colorSliders[2]->setSliderColor(GLMotif::Color(0.0f,0.0f,1.0f));
	colorSliders[2]->setValueRange(0.0,1.0,0.01);
	colorSliders[2]->setValue(0.5);
	colorSliders[2]->getValueChangedCallbacks().add(this,&PaletteEditor::colorSliderValueChangedCallback);
	
	colorSliders[3]=new GLMotif::Slider("AlphaSlider",colorSlidersBox,GLMotif::Slider::VERTICAL,ss.fontHeight*5.0f);
	colorSliders[3]->setValueRange(0.0,1.0,0.01);
	colorSliders[3]->setValue(0.5);
	colorSliders[3]->getValueChangedCallbacks().add(this,&PaletteEditor::colorSliderValueChangedCallback);
	
	colorSlidersBox->manageChild();
	
	colorEditor->manageChild();
	
	/* Create the button box: */
	GLMotif::RowColumn* buttonBox=new GLMotif::RowColumn("ButtonBox",colorMapDialog,false);
	buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	buttonBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	buttonBox->setAlignment(GLMotif::Alignment::RIGHT);
	
	GLMotif::Button* removeControlPointButton=new GLMotif::Button("RemoveControlPointButton",buttonBox,"Remove Control Point");
	removeControlPointButton->getSelectCallbacks().add(this,&PaletteEditor::removeControlPointCallback);
	
	GLMotif::Button* savePaletteButton=new GLMotif::Button("SavePaletteButton",buttonBox,"Save Palette");
	savePaletteButton->getSelectCallbacks().add(this,&PaletteEditor::savePaletteCallback);
	
	buttonBox->manageChild();
	
	/* Let the color map widget eat any size increases: */
	colorMapDialog->setRowWeight(0,1.0f);
	
	colorMapDialog->manageChild();
	}

PaletteEditor::Storage* PaletteEditor::getPalette(void) const
	{
	return colorMap->getColorMap();
	}

void PaletteEditor::setPalette(const PaletteEditor::Storage* newPalette)
	{
	colorMap->setColorMap(newPalette);
	}

void PaletteEditor::createPalette(PaletteEditor::ColorMapCreationType colorMapType,const PaletteEditor::ValueRange& newValueRange)
	{
	colorMap->createColorMap(colorMapType,newValueRange);
	}

void PaletteEditor::createPalette(const std::vector<GLMotif::ColorMap::ControlPoint>& controlPoints)
	{
	colorMap->createColorMap(controlPoints);
	}

void PaletteEditor::loadPalette(const char* paletteFileName,const PaletteEditor::ValueRange& newValueRange)
	{
	colorMap->loadColorMap(paletteFileName,newValueRange);
	}

void PaletteEditor::savePalette(const char* paletteFileName) const
	{
	colorMap->saveColorMap(paletteFileName);
	}

void PaletteEditor::exportColorMap(GLColorMap& glColorMap) const
	{
	/* Update the color map's colors: */
	colorMap->exportColorMap(glColorMap);
	
	/* Update the color map's value range: */
	glColorMap.setScalarRange(colorMap->getValueRange().first,colorMap->getValueRange().second);
	}
