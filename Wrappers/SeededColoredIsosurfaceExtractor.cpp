/***********************************************************************
SeededColoredIsosurfaceExtractor - Wrapper class to map from the
abstract visualization algorithm interface to a templatized colored
isosurface extractor implementation.
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

#define VISUALIZATION_WRAPPERS_SEEDEDCOLOREDISOSURFACEEXTRACTOR_IMPLEMENTATION

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
#include <Templatized/ColoredIsosurfaceExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/SeededColoredIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/*************************************************
Methods of class SeededColoredIsosurfaceExtractor:
*************************************************/

template <class DataSetWrapperParam>
inline
const typename SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::DS*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::getDs(
	Visualization::Abstract::VariableManager* variableManager,
	int scalarVariableIndex,
	int colorScalarVariableIndex)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByScalarVariable(scalarVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::SeededColoredIsosurfaceExtractor: Incompatible scalar and color scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::SeededColoredIsosurfaceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::SE&
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::SeededColoredIsosurfaceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::SeededColoredIsosurfaceExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable(),sVariableManager->getCurrentScalarVariable(),500000,true,true),
	 ise(getDs(sVariableManager,parameters.scalarVariableIndex,parameters.colorScalarVariableIndex),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.colorScalarVariableIndex))),
	 currentIsosurface(0),
	 maxNumTrianglesValue(0),maxNumTrianglesSlider(0),
	 colorScalarVariableBox(0),
	 extractionModeBox(0),
	 currentValue(0)
	{
	/* Set the templatized colored isosurface extractor's extraction mode: */
	ise.setExtractionMode(parameters.smoothShaded?ISE::SMOOTH:ISE::FLAT);
	}

template <class DataSetWrapperParam>
inline
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::~SeededColoredIsosurfaceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::setMaxNumTriangles(
	size_t newMaxNumTriangles)
	{
	parameters.maxNumTriangles=newMaxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
bool
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("SeededColoredIsosurfaceExtractorSettingsDialogPopup",widgetManager,"Seeded Colored Isosurface Extractor Settings");
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
	maxNumTrianglesSlider->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::maxNumTrianglesSliderCallback);
	
	maxNumTrianglesBox->manageChild();
	
	new GLMotif::Label("ColorScalarVariableLabel",settingsDialog,"Color Scalar");
	
	GLMotif::Margin* colorScalarVariableMargin=new GLMotif::Margin("ColorScalarVariableMargin",settingsDialog,false);
	colorScalarVariableMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	std::vector<std::string> scalarVariables;
	for(int i=0;i<getVariableManager()->getNumScalarVariables();++i)
		scalarVariables.push_back(getVariableManager()->getScalarVariableName(i));
	colorScalarVariableBox=new GLMotif::DropdownBox("ColorScalarVariableBox",colorScalarVariableMargin,scalarVariables);
	colorScalarVariableBox->setSelectedItem(parameters.colorScalarVariableIndex);
	colorScalarVariableBox->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::colorScalarVariableBoxCallback);
	
	colorScalarVariableMargin->manageChild();
	
	new GLMotif::Label("ExtractionModeLabel",settingsDialog,"Extraction Mode");
	
	GLMotif::RowColumn* surfaceModeBox=new GLMotif::RowColumn("SurfaceModeBox",settingsDialog,false);
	surfaceModeBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	surfaceModeBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	surfaceModeBox->setAlignment(GLMotif::Alignment::LEFT);
	surfaceModeBox->setNumMinorWidgets(1);
	
	extractionModeBox=new GLMotif::RadioBox("ExtractionModeBox",surfaceModeBox,false);
	extractionModeBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	extractionModeBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	extractionModeBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	extractionModeBox->addToggle("Flat Shaded");
	extractionModeBox->addToggle("Smooth Shaded");
	
	extractionModeBox->setSelectedToggle(parameters.smoothShaded?1:0);
	extractionModeBox->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::extractionModeBoxCallback);
	
	extractionModeBox->manageChild();
	
	lightSurfacesToggle=new GLMotif::ToggleButton("LightSurfacesToggle",surfaceModeBox,"Light Surfaces");
	lightSurfacesToggle->setBorderWidth(0.0f);
	lightSurfacesToggle->setHAlignment(GLFont::Left);
	lightSurfacesToggle->setToggle(parameters.lightSurface);
	lightSurfacesToggle->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::lightSurfacesToggleCallback);
	
	surfaceModeBox->manageChild();
	
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
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::createElement: Mismatching locator type");
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
	Isosurface* result=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	/* Extract the isosurface into the visualization element: */
	ise.extractSeededIsosurface(dsl,result->getSurface());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::createElement: Mismatching locator type");
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
	currentIsosurface=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	/* Start extracting the isosurface into the visualization element: */
	ise.startSeededIsosurface(dsl,currentIsosurface->getSurface());
	
	/* Return the result: */
	return currentIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the isosurface into the visualization element: */
	AlarmTimerElement<Isosurface> atcf(alarm,*currentIsosurface,parameters.maxNumTriangles);
	return ise.continueSeededIsosurface(atcf)||currentIsosurface->getElementSize()>=parameters.maxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	ise.finishSeededIsosurface();
	currentIsosurface=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Update the current isovalue text field: */
	currentValue->setValue(double(newParameters.isovalue));
	
	/* Create a new isosurface visualization element: */
	currentIsosurface=new Isosurface(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	return currentIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentIsosurface->getSurface().receive();
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::maxNumTrianglesSliderCallback(
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
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::colorScalarVariableBoxCallback(
	GLMotif::DropdownBox::ValueChangedCallbackData* cbData)
	{
	/* Get the new selected item: */
	parameters.colorScalarVariableIndex=cbData->newSelectedItem;
	
	/* Set the color isosurface extractor's color scalar variable: */
	ise.setColorScalarExtractor(getSe(getVariableManager()->getScalarExtractor(parameters.colorScalarVariableIndex)));
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::extractionModeBoxCallback(
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

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::lightSurfacesToggleCallback(
	GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Set the surface lighting flag: */
	parameters.lightSurface=cbData->set;
	}

}

}
