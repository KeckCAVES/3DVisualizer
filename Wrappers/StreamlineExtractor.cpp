/***********************************************************************
StreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized streamline extractor
implementation.
Copyright (c) 2006-2008 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_STREAMLINEEXTRACTOR_IMPLEMENTATION

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Slider.h>

#include <Abstract/VariableManager.h>
#include <Templatized/StreamlineExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/StreamlineExtractor.h>

namespace Visualization {

namespace Wrappers {

/************************************
Methods of class StreamlineExtractor:
************************************/

template <class DataSetWrapperParam>
inline
const typename StreamlineExtractor<DataSetWrapperParam>::DS*
StreamlineExtractor<DataSetWrapperParam>::getDs(
	Visualization::Abstract::VariableManager* variableManager,
	int vectorVariableIndex,
	int colorScalarVariableIndex)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("StreamlineExtractor::StreamlineExtractor: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("StreamlineExtractor::StreamlineExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename StreamlineExtractor<DataSetWrapperParam>::VE&
StreamlineExtractor<DataSetWrapperParam>::getVe(
	const Visualization::Abstract::VectorExtractor* sVectorExtractor)
	{
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(sVectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("StreamlineExtractor::StreamlineExtractor: Mismatching vector extractor type");
	
	return myVectorExtractor->getVe();
	}

template <class DataSetWrapperParam>
inline
const typename StreamlineExtractor<DataSetWrapperParam>::SE&
StreamlineExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("StreamlineExtractor::StreamlineExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
StreamlineExtractor<DataSetWrapperParam>::StreamlineExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentVectorVariable(),sVariableManager->getCurrentScalarVariable(),100000,0),
	 sle(getDs(sVariableManager,parameters.vectorVariableIndex,parameters.colorScalarVariableIndex),getVe(sVariableManager->getVectorExtractor(parameters.vectorVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.colorScalarVariableIndex))),
	 currentStreamline(0),
	 maxNumVerticesValue(0),maxNumVerticesSlider(0),
	 epsilonValue(0),epsilonSlider(0)
	{
	/* Get the default epsilon from the templatized streamline extractor: */
	parameters.epsilon=DSScalar(sle.getEpsilon());
	}

template <class DataSetWrapperParam>
inline
StreamlineExtractor<DataSetWrapperParam>::~StreamlineExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::setMaxNumVertices(
	size_t newMaxNumVertices)
	{
	parameters.maxNumVertices=newMaxNumVertices;
	}

template <class DataSetWrapperParam>
inline
bool
StreamlineExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
StreamlineExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
StreamlineExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("StreamlineExtractorSettingsDialogPopup",widgetManager,"Streamline Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("settingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	new GLMotif::Label("MaxNumVerticesLabel",settingsDialog,"Maximum Number of Steps");
	
	maxNumVerticesValue=new GLMotif::TextField("MaxNumVerticesValue",settingsDialog,12);
	maxNumVerticesValue->setValue((unsigned int)(parameters.maxNumVertices));
	
	maxNumVerticesSlider=new GLMotif::Slider("MaxNumVerticesSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	maxNumVerticesSlider->setValueRange(3.0,7.0,0.1);
	maxNumVerticesSlider->setValue(Math::log10(double(parameters.maxNumVertices)));
	maxNumVerticesSlider->getValueChangedCallbacks().add(this,&StreamlineExtractor::maxNumVerticesSliderCallback);
	
	new GLMotif::Label("EpsilonLabel",settingsDialog,"Error Threshold");
	
	epsilonValue=new GLMotif::TextField("EpsilonValue",settingsDialog,12);
	epsilonValue->setPrecision(6);
	epsilonValue->setValue(double(parameters.epsilon));
	
	epsilonSlider=new GLMotif::Slider("EpsilonSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	epsilonSlider->setValueRange(-16.0,-4.0,0.1);
	epsilonSlider->setValue(Math::log10(double(parameters.epsilon)));
	epsilonSlider->getValueChangedCallbacks().add(this,&StreamlineExtractor::epsilonSliderCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamlineExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("StreamlineExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new streamline visualization element: */
	Streamline* result=new Streamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	/* Extract the streamline into the visualization element: */
	sle.extractStreamline(seedLocator->getPosition(),dsl,typename SLE::Scalar(0.1),result->getPolyline());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamlineExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("StreamlineExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new streamline visualization element: */
	currentStreamline=new Streamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	/* Start extracting the streamline into the visualization element: */
	sle.startStreamline(seedLocator->getPosition(),dsl,typename SLE::Scalar(0.1),currentStreamline->getPolyline());
	
	/* Return the result: */
	return currentStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
StreamlineExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the streamline into the visualization element: */
	AlarmTimerElement<Streamline> atcf(alarm,*currentStreamline,parameters.maxNumVertices);
	return sle.continueStreamline(atcf)||currentStreamline->getElementSize()>=parameters.maxNumVertices;
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	sle.finishStreamline();
	currentStreamline=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamlineExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("StreamlineExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Create a new streamline visualization element: */
	currentStreamline=new Streamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	return currentStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("StreamlineExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentStreamline->getPolyline().receive();
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::maxNumVerticesSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to maximum number of vertices: */
	parameters.maxNumVertices=size_t(Math::floor(Math::pow(10.0,double(cbData->value))+0.5));
	
	/* Update the text field: */
	maxNumVerticesValue->setValue((unsigned int)(parameters.maxNumVertices));
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::epsilonSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to step size: */
	double epsilon=Math::pow(10.0,double(cbData->value));
	parameters.epsilon=DSScalar(epsilon);
	
	/* Update the streamline extractor's error threshold: */
	sle.setEpsilon(typename SLE::Scalar(epsilon));
	
	/* Update the text field: */
	epsilonValue->setValue(double(parameters.epsilon));
	}

}

}
