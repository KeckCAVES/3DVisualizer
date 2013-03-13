/***********************************************************************
StreamsurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized stream surface
extractor implementation.
Copyright (c) 2006-2007 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_STREAMSURFACEEXTRACTOR_IMPLEMENTATION

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

#include <Templatized/StreamsurfaceExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/Streamsurface.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/StreamsurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/***************************************
Methods of class StreamsurfaceExtractor:
***************************************/

template <class DataSetWrapperParam>
inline
const typename StreamsurfaceExtractor<DataSetWrapperParam>::DS*
StreamsurfaceExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("StreamsurfaceExtractor::StreamsurfaceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename StreamsurfaceExtractor<DataSetWrapperParam>::VE&
StreamsurfaceExtractor<DataSetWrapperParam>::getVe(
	const Visualization::Abstract::VectorExtractor* sVectorExtractor)
	{
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(sVectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("StreamsurfaceExtractor::StreamsurfaceExtractor: Mismatching vector extractor type");
	
	return myVectorExtractor->getVe();
	}

template <class DataSetWrapperParam>
inline
const typename StreamsurfaceExtractor<DataSetWrapperParam>::SE&
StreamsurfaceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("StreamsurfaceExtractor::StreamsurfaceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
StreamsurfaceExtractor<DataSetWrapperParam>::StreamsurfaceExtractor(
	const GLColorMap* sColorMap,
	const Visualization::Abstract::DataSet* sDataSet,
	const Visualization::Abstract::VectorExtractor* sVectorExtractor,
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor,
	Visualization::Abstract::DataSet::Scalar sDiskRadius)
	:colorMap(sColorMap),
	 sse(getDs(sDataSet),getVe(sVectorExtractor),getSe(sScalarExtractor)),
	 diskRadius(sDiskRadius),
	 maxNumVertices(100000),
	 currentStreamsurface(0),
	 numStreamlinesValue(0),numStreamlinesSlider(0),
	 diskRadiusValue(0),diskRadiusSlider(0),
	 stepSizeValue(0),stepSizeSlider(0),
	 maxNumVerticesValue(0),maxNumVerticesSlider(0)
	{
	sse.setNumStreamlines(16);
	sse.setClosed(true);
	}

template <class DataSetWrapperParam>
inline
StreamsurfaceExtractor<DataSetWrapperParam>::~StreamsurfaceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::setMaxNumVertices(
	size_t newMaxNumVertices)
	{
	maxNumVertices=newMaxNumVertices;
	}

template <class DataSetWrapperParam>
inline
bool
StreamsurfaceExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
StreamsurfaceExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
StreamsurfaceExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("StreamlineExtractorSettingsDialogPopup",widgetManager,"Stream Surface Extractor Settings");
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("settingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	GLMotif::Label* numStreamlinesLabel=new GLMotif::Label("NumStreamlinesLabel",settingsDialog,"Number Of Streamlines");
	
	numStreamlinesValue=new GLMotif::TextField("NumStreamlinesValue",settingsDialog,2);
	numStreamlinesValue->setValue(sse.getNumStreamlines());
	
	numStreamlinesSlider=new GLMotif::Slider("NumStreamlinesSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	numStreamlinesSlider->setValueRange(3.0,32.0,1.0);
	numStreamlinesSlider->setValue(double(sse.getNumStreamlines()));
	numStreamlinesSlider->getValueChangedCallbacks().add(this,&StreamsurfaceExtractor::numStreamlinesSliderCallback);
	
	GLMotif::Label* diskRadiusLabel=new GLMotif::Label("DiskRadiusLabel",settingsDialog,"Seed Disk Radius");
	
	diskRadiusValue=new GLMotif::TextField("DiskRadiusValue",settingsDialog,12);
	diskRadiusValue->setPrecision(6);
	diskRadiusValue->setValue(double(diskRadius));
	
	diskRadiusSlider=new GLMotif::Slider("DiskRadiusSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	double drl=Math::log10(double(diskRadius));
	diskRadiusSlider->setValueRange(drl-4.0,drl+4.0,0.1);
	diskRadiusSlider->setValue(drl);
	diskRadiusSlider->getValueChangedCallbacks().add(this,&StreamsurfaceExtractor::diskRadiusSliderCallback);
	
	GLMotif::Label* stepSizeLabel=new GLMotif::Label("StepSizeLabel",settingsDialog,"Step Size");
	
	stepSizeValue=new GLMotif::TextField("StepSizeValue",settingsDialog,12);
	stepSizeValue->setPrecision(6);
	stepSizeValue->setValue(double(sse.getStepSize()));
	
	stepSizeSlider=new GLMotif::Slider("StepSizeSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	stepSizeSlider->setValueRange(-4.0,4.0,0.1);
	stepSizeSlider->setValue(Math::log10(double(sse.getStepSize())));
	stepSizeSlider->getValueChangedCallbacks().add(this,&StreamsurfaceExtractor::stepSizeSliderCallback);
	
	GLMotif::Label* maxNumVerticesLabel=new GLMotif::Label("MaxNumVerticesLabel",settingsDialog,"Maximum Number of Steps");
	
	maxNumVerticesValue=new GLMotif::TextField("MaxNumVerticesValue",settingsDialog,12);
	maxNumVerticesValue->setValue((unsigned int)(maxNumVertices));
	
	maxNumVerticesSlider=new GLMotif::Slider("MaxNumVerticesSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	maxNumVerticesSlider->setValueRange(3.0,7.0,0.1);
	maxNumVerticesSlider->setValue(Math::log10(double(maxNumVertices)));
	maxNumVerticesSlider->getValueChangedCallbacks().add(this,&StreamsurfaceExtractor::maxNumVerticesSliderCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamsurfaceExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("StreamsurfaceExtractor::createElement: Mismatching locator type");
	DSL dsl=myLocator->getDsl();
	
	/* Create a new stream surface visualization element: */
	Streamsurface* result=new Streamsurface(colorMap);
	
	/* Create the seed disk: */
	DSPoint seedPoint=seedLocator->getPosition();
	dsl.locatePoint(seedPoint);
	DSVector seedVector=dsl.calcValue(sse.getVectorExtractor());
	DSVector x=Geometry::normal(seedVector);
	x.normalize();
	DSVector y=Geometry::cross(seedVector,x);
	y.normalize();
	for(int i=0;i<sse.getNumStreamlines();++i)
		{
		DSScalar angle=DSScalar(2)*Math::Constants<DSScalar>::pi*DSScalar(i)/DSScalar(sse.getNumStreamlines());
		DSPoint p=seedPoint;
		p+=x*(Math::cos(angle)*diskRadius);
		p+=y*(Math::sin(angle)*diskRadius);
		sse.initializeStreamline(i,p,dsl);
		}
	
	/* Extract the streamline into the visualization element: */
	sse.extractStreamsurface(result->getSurface());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamsurfaceExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("StreamsurfaceExtractor::createElement: Mismatching locator type");
	DSL dsl=myLocator->getDsl();
	
	/* Create a new streamline visualization element: */
	currentStreamsurface=new Streamsurface(colorMap);
	
	/* Create the seed disk: */
	DSPoint seedPoint=seedLocator->getPosition();
	dsl.locatePoint(seedPoint);
	DSVector seedVector=dsl.calcValue(sse.getVectorExtractor());
	DSVector x=Geometry::normal(seedVector);
	x.normalize();
	DSVector y=Geometry::cross(seedVector,x);
	y.normalize();
	for(int i=0;i<sse.getNumStreamlines();++i)
		{
		DSScalar angle=DSScalar(2)*Math::Constants<DSScalar>::pi*DSScalar(i)/DSScalar(sse.getNumStreamlines());
		DSPoint p=seedPoint;
		p+=x*(Math::cos(angle)*diskRadius);
		p+=y*(Math::sin(angle)*diskRadius);
		sse.initializeStreamline(i,p,dsl);
		}
	
	/* Start extracting the streamline into the visualization element: */
	sse.startStreamsurface(currentStreamsurface->getSurface());
	
	/* Return the result: */
	return currentStreamsurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
StreamsurfaceExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the streamline into the visualization element: */
	AlarmTimerElement<Streamsurface> atcf(alarm,*currentStreamsurface,maxNumVertices);
	return sse.continueStreamsurface(atcf)||currentStreamsurface->getElementSize()>=maxNumVertices;
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	sse.finishStreamsurface();
	currentStreamsurface=0;
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::numStreamlinesSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to number of streamlines: */
	int numStreamlines=int(Math::floor(double(cbData->value)+0.5));
	
	/* Update the streamline extractor's number of streamlines: */
	sse.setNumStreamlines(numStreamlines);
	
	/* Update the text field: */
	numStreamlinesValue->setValue(numStreamlines);
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::diskRadiusSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to disk radius: */
	diskRadius=DSScalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	diskRadiusValue->setValue(double(diskRadius));
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::stepSizeSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to step size: */
	double stepSize=Math::pow(10.0,double(cbData->value));
	
	/* Update the streamline extractor's step size: */
	sse.setStepSize(typename SSE::Scalar(stepSize));
	
	/* Update the text field: */
	stepSizeValue->setValue(stepSize);
	}

template <class DataSetWrapperParam>
inline
void
StreamsurfaceExtractor<DataSetWrapperParam>::maxNumVerticesSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to maximum number of vertices: */
	maxNumVertices=size_t(Math::floor(Math::pow(10.0,double(cbData->value))+0.5));
	
	/* Update the text field: */
	maxNumVerticesValue->setValue((unsigned int)(maxNumVertices));
	}

}

}
