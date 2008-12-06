/***********************************************************************
SeededIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
implementation.
Copyright (c) 2005-2008 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_SEEDEDISOSURFACEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <GL/GLColorMap.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RowColumn.h>

#include <Abstract/VariableManager.h>
#include <Templatized/IsosurfaceExtractorIndexedTriangleSet.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/SeededIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/******************************************
Methods of class SeededIsosurfaceExtractor:
******************************************/

template <class DataSetWrapperParam>
inline
const typename SeededIsosurfaceExtractor<DataSetWrapperParam>::DS*
SeededIsosurfaceExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::SeededIsosurfaceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename SeededIsosurfaceExtractor<DataSetWrapperParam>::SE&
SeededIsosurfaceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::SeededIsosurfaceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
SeededIsosurfaceExtractor<DataSetWrapperParam>::SeededIsosurfaceExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable(),500000,true),
	 ise(getDs(sVariableManager->getDataSetByScalarVariable(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex))),
	 currentIsosurface(0),
	 maxNumTrianglesValue(0),maxNumTrianglesSlider(0),
	 extractionModeBox(0),currentValue(0)
	{
	/* Set the templatized isosurface extractor's extraction mode: */
	ise.setExtractionMode(parameters.smoothShaded?ISE::SMOOTH:ISE::FLAT);
	}

template <class DataSetWrapperParam>
inline
SeededIsosurfaceExtractor<DataSetWrapperParam>::~SeededIsosurfaceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::setMaxNumTriangles(
	size_t newMaxNumTriangles)
	{
	parameters.maxNumTriangles=newMaxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
bool
SeededIsosurfaceExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
SeededIsosurfaceExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
SeededIsosurfaceExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("SeededIsosurfaceExtractorSettingsDialogPopup",widgetManager,"Seeded Isosurface Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("SettingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("MaxNumTrianglesLabel",settingsDialog,"Maximum Number of Triangles");
	
	GLMotif::RowColumn* maxNumTrianglesBox=new GLMotif::RowColumn("MaxNumTrianglesBox",settingsDialog,false);
	maxNumTrianglesBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	
	maxNumTrianglesValue=new GLMotif::TextField("MaxNumTrianglesValue",maxNumTrianglesBox,12);
	maxNumTrianglesValue->setValue((unsigned int)(parameters.maxNumTriangles));
	
	maxNumTrianglesSlider=new GLMotif::Slider("MaxNumTrianglesSlider",maxNumTrianglesBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	maxNumTrianglesSlider->setValueRange(3.0,7.0,0.1);
	maxNumTrianglesSlider->setValue(Math::log10(double(parameters.maxNumTriangles)));
	maxNumTrianglesSlider->getValueChangedCallbacks().add(this,&SeededIsosurfaceExtractor::maxNumTrianglesSliderCallback);
	
	maxNumTrianglesBox->manageChild();
	
	new GLMotif::Label("ExtractionModeLabel",settingsDialog,"Extraction Mode");
	
	extractionModeBox=new GLMotif::RadioBox("ExtractionModeBox",settingsDialog,false);
	extractionModeBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	extractionModeBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	extractionModeBox->setAlignment(GLMotif::Alignment::LEFT);
	extractionModeBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	extractionModeBox->addToggle("Flat Shaded");
	extractionModeBox->addToggle("Smooth Shaded");
	
	extractionModeBox->setSelectedToggle(parameters.smoothShaded?1:0);
	extractionModeBox->getValueChangedCallbacks().add(this,&SeededIsosurfaceExtractor::extractionModeBoxCallback);
	
	extractionModeBox->manageChild();
	
	new GLMotif::Label("CurrentValueLabel",settingsDialog,"Current Isovalue");
	
	GLMotif::Margin* currentValueMargin=new GLMotif::Margin("CurrentValueMargin",settingsDialog,false);
	currentValueMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	currentValue=new GLMotif::TextField("CurrentValue",currentValueMargin,16);
	currentValue->setPrecision(10);
	currentValue->setLabel("");
	
	currentValueMargin->manageChild();
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and the isovalue: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	typename SE::Scalar isovalue=dsl.calcValue(ise.getScalarExtractor());
	newParameters.isovalue=isovalue;
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Update the current isovalue text field: */
	currentValue->setValue(double(isovalue));
	
	/* Create a new isosurface visualization element: */
	Isosurface* result=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	/* Extract the isosurface into the visualization element: */
	ise.extractSeededIsosurface(dsl,result->getSurface());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededIsosurfaceExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and the isovalue: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	typename SE::Scalar isovalue=dsl.calcValue(ise.getScalarExtractor());
	newParameters.isovalue=isovalue;
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Update the current isovalue text field: */
	currentValue->setValue(double(newParameters.isovalue));
	
	/* Create a new isosurface visualization element: */
	currentIsosurface=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	/* Start extracting the isosurface into the visualization element: */
	ise.startSeededIsosurface(dsl,currentIsosurface->getSurface());
	
	/* Return the result: */
	return currentIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
SeededIsosurfaceExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the isosurface into the visualization element: */
	AlarmTimerElement<Isosurface> atcf(alarm,*currentIsosurface,parameters.maxNumTriangles);
	return ise.continueSeededIsosurface(atcf)||currentIsosurface->getElementSize()>=parameters.maxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	ise.finishSeededIsosurface();
	currentIsosurface=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededIsosurfaceExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededIsosurfaceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Update the current isovalue text field: */
	currentValue->setValue(double(newParameters.isovalue));
	
	/* Create a new isosurface visualization element: */
	currentIsosurface=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	return currentIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededIsosurfaceExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentIsosurface->getSurface().receive();
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::maxNumTrianglesSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to maximum number of triangles: */
	parameters.maxNumTriangles=size_t(Math::floor(Math::pow(10.0,double(cbData->value))+0.5));
	
	/* Update the text field: */
	maxNumTrianglesValue->setValue((unsigned int)(parameters.maxNumTriangles));
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::extractionModeBoxCallback(
	GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(extractionModeBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			parameters.smoothShaded=false;
			ise.setExtractionMode(ISE::FLAT);
			break;
		
		case 1:
			parameters.smoothShaded=true;
			ise.setExtractionMode(ISE::SMOOTH);
			break;
		}
	}

}

}
