/***********************************************************************
StreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized streamline extractor
implementation.
Copyright (c) 2006-2009 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>
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
#include <Wrappers/ElementSizeLimit.h>
#include <Wrappers/AlarmTimerElement.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/StreamlineExtractor.h>

namespace Visualization {

namespace Wrappers {

/************************************************
Methods of class StreamlineExtractor::Parameters:
************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		vectorVariableIndex=dataSource.template read<int>();
	else
		vectorVariableIndex=readVectorVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	if(raw)
		colorScalarVariableIndex=dataSource.template read<int>();
	else
		colorScalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	maxNumVertices=dataSource.template read<unsigned int>();
	epsilon=dataSource.template read<Scalar>();
	dataSource.template read<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(vectorVariableIndex);
	else
		writeVectorVariableNameBinary<DataSinkParam>(dataSink,vectorVariableIndex,variableManager);
	if(raw)
		dataSink.template write<int>(colorScalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,colorScalarVariableIndex,variableManager);
	dataSink.template write<unsigned int>(maxNumVertices);
	dataSink.template write<Scalar>(epsilon);
	dataSink.template write<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
StreamlineExtractor<DataSetWrapperParam>::Parameters::Parameters(
	Visualization::Abstract::VariableManager* variableManager)
	:vectorVariableIndex(variableManager->getCurrentVectorVariable()),
	 colorScalarVariableIndex(variableManager->getCurrentScalarVariable()),
	 locatorValid(false)
	{
	update(variableManager,false);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::read(
	Misc::File& file,
	bool ascii,
	Visualization::Abstract::VariableManager* variableManager)
	{
	if(ascii)
		{
		/* Parse the parameter section: */
		AsciiParameterFileSectionHash* hash=parseAsciiParameterFileSection<Misc::File>(file);
		
		/* Extract the parameters: */
		vectorVariableIndex=readVectorVariableNameAscii(hash,"vectorVariable",variableManager);
		colorScalarVariableIndex=readScalarVariableNameAscii(hash,"colorScalarVariable",variableManager);
		maxNumVertices=readParameterAscii<unsigned int>(hash,"maxNumVertices",maxNumVertices);
		epsilon=readParameterAscii<Scalar>(hash,"epsilon",epsilon);
		seedPoint=readParameterAscii<Point>(hash,"seedPoint",seedPoint);
		
		/* Clean up: */
		deleteAsciiParameterFileSectionHash(hash);
		}
	else
		{
		/* Read from binary file: */
		readBinary(file,false,variableManager);
		}
	
	/* Update derived parameters: */
	update(variableManager,true);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	
	/* Update derived parameters: */
	update(variableManager,true);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::ClusterPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read (and ignore) the parameter packet size from the cluster pipe: */
	pipe.read<unsigned int>();
	
	/* Read from cluster pipe: */
	readBinary(pipe,false,variableManager);
	
	/* Update derived parameters: */
	update(variableManager,true);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeVectorVariableNameAscii<Misc::File>(file,"vectorVariable",vectorVariableIndex,variableManager);
		writeScalarVariableNameAscii<Misc::File>(file,"colorScalarVariable",colorScalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,unsigned int>(file,"maxNumVertices",maxNumVertices);
		writeParameterAscii<Misc::File,Scalar>(file,"epsilon",epsilon);
		writeParameterAscii<Misc::File,Point>(file,"seedPoint",seedPoint);
		file.write("}\n",2);
		}
	else
		{
		/* Write to binary file: */
		writeBinary(file,false,variableManager);
		}
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getVectorVariableNameLength(vectorVariableIndex,variableManager);
	packetSize+=getScalarVariableNameLength(colorScalarVariableIndex,variableManager);
	packetSize+=sizeof(unsigned int)+sizeof(Scalar);
	packetSize+=sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::Parameters::update(
	Visualization::Abstract::VariableManager* variableManager,
	bool track)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("StreamlineExtractor::Parameters::update: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("StreamlineExtractor::Parameters::update: Mismatching data set type");
	ds=&myDataSet->getDs();
	
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(variableManager->getVectorExtractor(vectorVariableIndex));
	if(myVectorExtractor==0)
		Misc::throwStdErr("StreamlineExtractor::Parameters::update: Mismatching vector extractor type");
	ve=&myVectorExtractor->getVe();
	
	/* Get a pointer to the color scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(variableManager->getScalarExtractor(colorScalarVariableIndex));
	if(myScalarExtractor==0)
		Misc::throwStdErr("StreamlineExtractor::Parameters::update: Mismatching scalar extractor type");
	cse=&myScalarExtractor->getSe();
	
	/* Get a templatized locator: */
	dsl=ds->getLocator();
	if(track)
		{
		/* Locate the seed point: */
		locatorValid=dsl.locatePoint(seedPoint);
		}
	}

/********************************************
Static elements of class StreamlineExtractor:
********************************************/

template <class DataSetWrapperParam>
const char* StreamlineExtractor<DataSetWrapperParam>::name="Streamline";

/************************************
Methods of class StreamlineExtractor:
************************************/

template <class DataSetWrapperParam>
inline
StreamlineExtractor<DataSetWrapperParam>::StreamlineExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager),
	 sle(parameters.ds,*parameters.ve,*parameters.cse),
	 currentStreamline(0),
	 maxNumVerticesValue(0),maxNumVerticesSlider(0),
	 epsilonValue(0),epsilonSlider(0)
	{
	/* Initialize parameters: */
	parameters.maxNumVertices=100000;
	parameters.epsilon=Scalar(sle.getEpsilon());
	}

template <class DataSetWrapperParam>
inline
StreamlineExtractor<DataSetWrapperParam>::~StreamlineExtractor(
	void)
	{
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
void
StreamlineExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("StreamlineExtractor::setSeedLocator: Mismatching locator type");
	
	/* Update the seed point: */
	parameters.seedPoint=Point(seedLocator->getPosition());
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamlineExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("StreamlineExtractor::createElement: Mismatching parameter object type");
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new streamline visualization element: */
	Streamline* result=new Streamline(myParameters,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Update the streamline extractor: */
	sle.update(myParameters->ds,*myParameters->ve,*myParameters->cse);
	
	/* Extract the streamline into the visualization element: */
	sle.startStreamline(myParameters->seedPoint,myParameters->dsl,typename SLE::Scalar(0.1),result->getPolyline());
	ElementSizeLimit<Streamline> esl(*result,myParameters->maxNumVertices);
	sle.continueStreamline(esl);
	sle.finishStreamline();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
StreamlineExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("StreamlineExtractor::createElement: Mismatching parameter object type");
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new streamline visualization element: */
	currentStreamline=new Streamline(myParameters,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Update the streamline extractor: */
	sle.update(myParameters->ds,*myParameters->ve,*myParameters->cse);
	
	/* Extract the streamline into the visualization element: */
	sle.startStreamline(myParameters->seedPoint,myParameters->dsl,typename SLE::Scalar(0.1),currentStreamline->getPolyline());
	
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
	size_t maxNumVertices=dynamic_cast<Parameters*>(currentStreamline->getParameters())->maxNumVertices;
	AlarmTimerElement<Streamline> atcf(alarm,*currentStreamline,maxNumVertices);
	return sle.continueStreamline(atcf)||currentStreamline->getElementSize()>=maxNumVertices;
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
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("StreamlineExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("StreamlineExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new streamline visualization element: */
	currentStreamline=new Streamline(myParameters,getVariableManager()->getColorMap(myParameters->colorScalarVariableIndex),getPipe());
	
	return currentStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
StreamlineExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
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
	parameters.epsilon=Scalar(epsilon);
	
	/* Update the streamline extractor's error threshold: */
	sle.setEpsilon(typename SLE::Scalar(epsilon));
	
	/* Update the text field: */
	epsilonValue->setValue(double(parameters.epsilon));
	}

}

}
