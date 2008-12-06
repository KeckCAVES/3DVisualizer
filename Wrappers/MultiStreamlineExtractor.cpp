/***********************************************************************
MultiStreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized multi-streamline
extractor implementation.
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

#define VISUALIZATION_WRAPPERS_MULTISTREAMLINEEXTRACTOR_IMPLEMENTATION

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Slider.h>

#include <Abstract/VariableManager.h>
#include <Templatized/MultiStreamlineExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/MultiStreamlineExtractor.h>

namespace Visualization {

namespace Wrappers {

/*****************************************
Methods of class MultiStreamlineExtractor:
*****************************************/

template <class DataSetWrapperParam>
inline
const typename MultiStreamlineExtractor<DataSetWrapperParam>::DS*
MultiStreamlineExtractor<DataSetWrapperParam>::getDs(
	Visualization::Abstract::VariableManager* variableManager,
	int vectorVariableIndex,
	int colorScalarVariableIndex)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("MultiStreamlineExtractor::MultiStreamlineExtractor: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("MultiStreamlineExtractor::MultiStreamlineExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename MultiStreamlineExtractor<DataSetWrapperParam>::VE&
MultiStreamlineExtractor<DataSetWrapperParam>::getVe(
	const Visualization::Abstract::VectorExtractor* sVectorExtractor)
	{
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(sVectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("MultiStreamlineExtractor::MultiStreamlineExtractor: Mismatching vector extractor type");
	
	return myVectorExtractor->getVe();
	}

template <class DataSetWrapperParam>
inline
const typename MultiStreamlineExtractor<DataSetWrapperParam>::SE&
MultiStreamlineExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("MultiStreamlineExtractor::MultiStreamlineExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
MultiStreamlineExtractor<DataSetWrapperParam>::MultiStreamlineExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentVectorVariable(),sVariableManager->getCurrentScalarVariable(),20000,0,8,0),
	 msle(getDs(sVariableManager,parameters.vectorVariableIndex,parameters.colorScalarVariableIndex),getVe(sVariableManager->getVectorExtractor(parameters.vectorVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.colorScalarVariableIndex))),
	 currentStreamline(0),
	 maxNumVerticesValue(0),maxNumVerticesSlider(0),
	 epsilonValue(0),epsilonSlider(0),
	 numStreamlinesValue(0),numStreamlinesSlider(0),
	 diskRadiusValue(0),diskRadiusSlider(0)
	{
	/* Get the default epsilon from the templatized multi-streamline extractor: */
	parameters.epsilon=DSScalar(msle.getEpsilon());
	
	/* Set the multi-streamline extractor's number of streamlines: */
	msle.setNumStreamlines(parameters.numStreamlines);
	
	/* Set the initial seed disk radius: */
	parameters.diskRadius=msle.getDataSet()->calcAverageCellSize();
	}

template <class DataSetWrapperParam>
inline
MultiStreamlineExtractor<DataSetWrapperParam>::~MultiStreamlineExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::setMaxNumVertices(
	size_t newMaxNumVertices)
	{
	parameters.maxNumVertices=newMaxNumVertices;
	}

template <class DataSetWrapperParam>
inline
bool
MultiStreamlineExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
MultiStreamlineExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
MultiStreamlineExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("StreamlineExtractorSettingsDialogPopup",widgetManager,"Multistreamline Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("settingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	new GLMotif::Label("MaxNumVerticesLabel",settingsDialog,"Maximum Number of Steps");
	
	maxNumVerticesValue=new GLMotif::TextField("MaxNumVerticesValue",settingsDialog,12);
	maxNumVerticesValue->setValue((unsigned int)(parameters.maxNumVertices));
	
	maxNumVerticesSlider=new GLMotif::Slider("MaxNumVerticesSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	maxNumVerticesSlider->setValueRange(3.0,7.0,0.1);
	maxNumVerticesSlider->setValue(Math::log10(double(parameters.maxNumVertices)));
	maxNumVerticesSlider->getValueChangedCallbacks().add(this,&MultiStreamlineExtractor::maxNumVerticesSliderCallback);
	
	new GLMotif::Label("EpsilonLabel",settingsDialog,"Error Threshold");
	
	epsilonValue=new GLMotif::TextField("EpsilonValue",settingsDialog,12);
	epsilonValue->setPrecision(6);
	epsilonValue->setValue(double(parameters.epsilon));
	
	epsilonSlider=new GLMotif::Slider("EpsilonSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	epsilonSlider->setValueRange(-16.0,-4.0,0.1);
	epsilonSlider->setValue(Math::log10(double(parameters.epsilon)));
	epsilonSlider->getValueChangedCallbacks().add(this,&MultiStreamlineExtractor::epsilonSliderCallback);
	
	new GLMotif::Label("NumStreamlinesLabel",settingsDialog,"Number Of Streamlines");
	
	numStreamlinesValue=new GLMotif::TextField("NumStreamlinesValue",settingsDialog,2);
	numStreamlinesValue->setValue(parameters.numStreamlines);
	
	numStreamlinesSlider=new GLMotif::Slider("NumStreamlinesSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	numStreamlinesSlider->setValueRange(3.0,32.0,1.0);
	numStreamlinesSlider->setValue(double(parameters.numStreamlines));
	numStreamlinesSlider->getValueChangedCallbacks().add(this,&MultiStreamlineExtractor::numStreamlinesSliderCallback);
	
	new GLMotif::Label("DiskRadiusLabel",settingsDialog,"Seed Disk Radius");
	
	diskRadiusValue=new GLMotif::TextField("DiskRadiusValue",settingsDialog,12);
	diskRadiusValue->setPrecision(6);
	diskRadiusValue->setValue(double(parameters.diskRadius));
	
	diskRadiusSlider=new GLMotif::Slider("DiskRadiusSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	double drl=Math::log10(double(parameters.diskRadius));
	diskRadiusSlider->setValueRange(drl-4.0,drl+4.0,0.1);
	diskRadiusSlider->setValue(drl);
	diskRadiusSlider->getValueChangedCallbacks().add(this,&MultiStreamlineExtractor::diskRadiusSliderCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("MultiStreamlineExtractor::createElement: Mismatching locator type");
	DSL dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and seed disk frame: */
	Parameters newParameters=parameters;
	newParameters.base=seedLocator->getPosition();
	dsl.locatePoint(newParameters.base);
	DSVector seedVector=dsl.calcValue(msle.getVectorExtractor());
	newParameters.frame[0]=Geometry::normal(seedVector);
	newParameters.frame[0].normalize();
	newParameters.frame[1]=Geometry::cross(seedVector,newParameters.frame[0]);
	newParameters.frame[1].normalize();
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new streamline visualization element: */
	MultiStreamline* result=new MultiStreamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	msle.setMultiStreamline(result->getMultiPolyline());
	
	/* Calculate all streamline's starting points: */
	for(unsigned int i=0;i<newParameters.numStreamlines;++i)
		{
		DSScalar angle=DSScalar(2)*Math::Constants<DSScalar>::pi*DSScalar(i)/DSScalar(newParameters.numStreamlines);
		DSPoint p=newParameters.base;
		p+=newParameters.frame[0]*(Math::cos(angle)*newParameters.diskRadius);
		p+=newParameters.frame[1]*(Math::sin(angle)*newParameters.diskRadius);
		msle.initializeStreamline(i,p,dsl,typename MSLE::Scalar(0.1));
		}
	
	/* Extract the streamline into the visualization element: */
	msle.extractStreamlines();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("MultiStreamlineExtractor::createElement: Mismatching locator type");
	DSL dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and seed disk frame: */
	Parameters newParameters=parameters;
	newParameters.base=seedLocator->getPosition();
	dsl.locatePoint(newParameters.base);
	DSVector seedVector=dsl.calcValue(msle.getVectorExtractor());
	newParameters.frame[0]=Geometry::normal(seedVector);
	newParameters.frame[0].normalize();
	newParameters.frame[1]=Geometry::cross(seedVector,newParameters.frame[0]);
	newParameters.frame[1].normalize();
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new streamline visualization element: */
	currentStreamline=new MultiStreamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	msle.setMultiStreamline(currentStreamline->getMultiPolyline());
	
	/* Calculate all streamline's starting points: */
	for(unsigned int i=0;i<newParameters.numStreamlines;++i)
		{
		DSScalar angle=DSScalar(2)*Math::Constants<DSScalar>::pi*DSScalar(i)/DSScalar(newParameters.numStreamlines);
		DSPoint p=newParameters.base;
		p+=newParameters.frame[0]*(Math::cos(angle)*newParameters.diskRadius);
		p+=newParameters.frame[1]*(Math::sin(angle)*newParameters.diskRadius);
		msle.initializeStreamline(i,p,dsl,typename MSLE::Scalar(0.1));
		}
	
	/* Start extracting the streamline into the visualization element: */
	msle.startStreamlines();
	
	/* Return the result: */
	return currentStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
MultiStreamlineExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the streamline into the visualization element: */
	AlarmTimerElement<MultiStreamline> atcf(alarm,*currentStreamline,parameters.maxNumVertices);
	return msle.continueStreamlines(atcf)||currentStreamline->getElementSize()>=parameters.maxNumVertices;
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	msle.finishStreamlines();
	currentStreamline=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("MultiStreamlineExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Create a new multi-streamline visualization element: */
	currentStreamline=new MultiStreamline(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	return currentStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("MultiStreamlineExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentStreamline->getMultiPolyline().receive();
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::maxNumVerticesSliderCallback(
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
MultiStreamlineExtractor<DataSetWrapperParam>::epsilonSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to step size: */
	double epsilon=Math::pow(10.0,double(cbData->value));
	parameters.epsilon=DSScalar(epsilon);
	
	/* Update the multi-streamline extractor's accuracy threshold: */
	msle.setEpsilon(typename MSLE::Scalar(epsilon));
	
	/* Update the text field: */
	epsilonValue->setValue(double(parameters.epsilon));
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::numStreamlinesSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to number of streamlines: */
	parameters.numStreamlines=(unsigned int)(Math::floor(double(cbData->value)+0.5));
	
	/* Update the multi-streamline extractor's number of streamlines: */
	msle.setNumStreamlines(parameters.numStreamlines);
	
	/* Update the text field: */
	numStreamlinesValue->setValue(parameters.numStreamlines);
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::diskRadiusSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to disk radius: */
	parameters.diskRadius=DSScalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	diskRadiusValue->setValue(double(parameters.diskRadius));
	}

}

}
