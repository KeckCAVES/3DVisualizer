/***********************************************************************
MultiStreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized multi-streamline
extractor implementation.
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

#define VISUALIZATION_WRAPPERS_MULTISTREAMLINEEXTRACTOR_IMPLEMENTATION

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
#include <Templatized/MultiStreamlineExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/ElementSizeLimit.h>
#include <Wrappers/AlarmTimerElement.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/MultiStreamlineExtractor.h>

namespace Visualization {

namespace Wrappers {

/*****************************************************
Methods of class MultiStreamlineExtractor::Parameters:
*****************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::readBinary(
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
	numStreamlines=dataSource.template read<unsigned int>();
	diskRadius=dataSource.template read<Scalar>();
	dataSource.template read<Scalar>(base.getComponents(),dimension);
	for(int i=0;i<2;++i)
		dataSource.template read<Scalar>(frame[i].getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::writeBinary(
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
	dataSink.template write<unsigned int>(numStreamlines);
	dataSink.template write<Scalar>(diskRadius);
	dataSink.template write<Scalar>(base.getComponents(),dimension);
	for(int i=0;i<2;++i)
		dataSink.template write<Scalar>(frame[i].getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::Parameters(
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
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::read(
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
		numStreamlines=readParameterAscii<unsigned int>(hash,"numStreamlines",numStreamlines);
		diskRadius=readParameterAscii<Scalar>(hash,"diskRadius",diskRadius);
		base=readParameterAscii<Point>(hash,"base",base);
		readParameterAscii<Vector>(hash,"frame",frame,2);
		
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
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::read(
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
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::read(
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
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::write(
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
		writeParameterAscii<Misc::File,unsigned int>(file,"numStreamlines",numStreamlines);
		writeParameterAscii<Misc::File,Scalar>(file,"diskRadius",diskRadius);
		writeParameterAscii<Misc::File,Point>(file,"base",base);
		writeParameterAscii<Misc::File,Vector>(file,"frame",frame,2);
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
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getVectorVariableNameLength(vectorVariableIndex,variableManager);
	packetSize+=getScalarVariableNameLength(colorScalarVariableIndex,variableManager);
	packetSize+=sizeof(unsigned int)+sizeof(Scalar)+sizeof(unsigned int)+sizeof(Scalar);
	packetSize+=sizeof(Scalar)*dimension+2*sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::Parameters::update(
	Visualization::Abstract::VariableManager* variableManager,
	bool track)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("MultiStreamlineExtractor::Parameters::update: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("MultiStreamlineExtractor::Parameters::update: Mismatching data set type");
	ds=&myDataSet->getDs();
	
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(variableManager->getVectorExtractor(vectorVariableIndex));
	if(myVectorExtractor==0)
		Misc::throwStdErr("MultiStreamlineExtractor::Parameters::update: Mismatching vector extractor type");
	ve=&myVectorExtractor->getVe();
	
	/* Get a pointer to the color scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(variableManager->getScalarExtractor(colorScalarVariableIndex));
	if(myScalarExtractor==0)
		Misc::throwStdErr("MultiStreamlineExtractor::Parameters::update: Mismatching scalar extractor type");
	cse=&myScalarExtractor->getSe();
	
	/* Get a templatized locator: */
	dsl=ds->getLocator();
	if(track)
		{
		/* Locate the base point: */
		locatorValid=dsl.locatePoint(base);
		}
	}

/*************************************************
Static elements of class MultiStreamlineExtractor:
*************************************************/

template <class DataSetWrapperParam>
const char* MultiStreamlineExtractor<DataSetWrapperParam>::name="Streamline Bundle";

/*****************************************
Methods of class MultiStreamlineExtractor:
*****************************************/

template <class DataSetWrapperParam>
inline
MultiStreamlineExtractor<DataSetWrapperParam>::MultiStreamlineExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager),
	 msle(parameters.ds,*parameters.ve,*parameters.cse),
	 currentMultiStreamline(0),
	 maxNumVerticesValue(0),maxNumVerticesSlider(0),
	 epsilonValue(0),epsilonSlider(0),
	 numStreamlinesValue(0),numStreamlinesSlider(0),
	 diskRadiusValue(0),diskRadiusSlider(0)
	{
	/* Initialize parameters: */
	parameters.epsilon=Scalar(msle.getEpsilon());
	parameters.maxNumVertices=20000;
	parameters.numStreamlines=8;
	parameters.diskRadius=parameters.ds->calcAverageCellSize();
	
	/* Set the multi-streamline extractor's number of streamlines: */
	msle.setNumStreamlines(parameters.numStreamlines);
	}

template <class DataSetWrapperParam>
inline
MultiStreamlineExtractor<DataSetWrapperParam>::~MultiStreamlineExtractor(
	void)
	{
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
void
MultiStreamlineExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("MultiStreamlineExtractor::setSeedLocator: Mismatching locator type");
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	
	/* Calculate the seeding point and seed disk frame: */
	parameters.base=Point(seedLocator->getPosition());
	Vector seedVector=parameters.dsl.calcValue(*parameters.ve);
	parameters.frame[0]=Geometry::normal(seedVector);
	parameters.frame[0].normalize();
	parameters.frame[1]=Geometry::cross(seedVector,parameters.frame[0]);
	parameters.frame[1].normalize();
	
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("MultiStreamlineExtractor::createElement: Mismatching parameter object type");
	
	/* Create a new multi-streamline visualization element: */
	MultiStreamline* result=new MultiStreamline(myParameters,myParameters->numStreamlines,getVariableManager()->getColorMap(myParameters->colorScalarVariableIndex),getPipe());
	
	/* Update the multi-streamline extractor: */
	msle.update(myParameters->ds,*myParameters->ve,*myParameters->cse);
	msle.setMultiStreamline(result->getMultiPolyline());
	
	/* Calculate all streamlines' starting points: */
	for(unsigned int i=0;i<myParameters->numStreamlines;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(myParameters->numStreamlines);
		Point p=myParameters->base;
		p+=myParameters->frame[0]*(Math::cos(angle)*myParameters->diskRadius);
		p+=myParameters->frame[1]*(Math::sin(angle)*myParameters->diskRadius);
		msle.initializeStreamline(i,p,myParameters->dsl,typename MSLE::Scalar(0.1));
		}
	
	/* Extract the streamline into the visualization element: */
	msle.startStreamlines();
	ElementSizeLimit<MultiStreamline> esl(*result,myParameters->maxNumVertices);
	msle.continueStreamlines(esl);
	msle.finishStreamlines();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("MultiStreamlineExtractor::startElement: Mismatching parameter object type");
	
	/* Create a new multi-streamline visualization element: */
	currentMultiStreamline=new MultiStreamline(myParameters,myParameters->numStreamlines,getVariableManager()->getColorMap(myParameters->colorScalarVariableIndex),getPipe());
	
	/* Update the multi-streamline extractor: */
	msle.update(myParameters->ds,*myParameters->ve,*myParameters->cse);
	msle.setMultiStreamline(currentMultiStreamline->getMultiPolyline());
	
	/* Calculate all streamlines' starting points: */
	for(unsigned int i=0;i<myParameters->numStreamlines;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(myParameters->numStreamlines);
		Point p=myParameters->base;
		p+=myParameters->frame[0]*(Math::cos(angle)*myParameters->diskRadius);
		p+=myParameters->frame[1]*(Math::sin(angle)*myParameters->diskRadius);
		msle.initializeStreamline(i,p,myParameters->dsl,typename MSLE::Scalar(0.1));
		}
	
	/* Extract the streamline into the visualization element: */
	msle.startStreamlines();
	
	/* Return the result: */
	return currentMultiStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
MultiStreamlineExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the multi-streamline into the visualization element: */
	size_t maxNumVertices=dynamic_cast<Parameters*>(currentMultiStreamline->getParameters())->maxNumVertices;
	AlarmTimerElement<MultiStreamline> atcf(alarm,*currentMultiStreamline,maxNumVertices);
	return msle.continueStreamlines(atcf)||currentMultiStreamline->getElementSize()>=maxNumVertices;
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	msle.finishStreamlines();
	currentMultiStreamline=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
MultiStreamlineExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("MultiStreamlineExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("MultiStreamlineExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new multi-streamline visualization element: */
	currentMultiStreamline=new MultiStreamline(myParameters,myParameters->numStreamlines,getVariableManager()->getColorMap(myParameters->colorScalarVariableIndex),getPipe());
	
	return currentMultiStreamline.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
MultiStreamlineExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
		Misc::throwStdErr("MultiStreamlineExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentMultiStreamline->getMultiPolyline().receive();
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
	parameters.epsilon=Scalar(epsilon);
	
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
	parameters.diskRadius=Scalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	diskRadiusValue->setValue(double(parameters.diskRadius));
	}

}

}
