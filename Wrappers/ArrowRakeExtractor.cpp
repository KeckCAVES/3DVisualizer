/***********************************************************************
ArrowRakeExtractor - Wrapper class extract rakes of arrows from vector
fields.
Copyright (c) 2008-2009 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_IMPLEMENTATION

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
#include <Vrui/Vrui.h>

#include <Abstract/VariableManager.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/ArrowRakeExtractor.h>

namespace Visualization {

namespace Wrappers {

/***********************************************
Methods of class ArrowRakeExtractor::Parameters:
***********************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::readBinary(
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
	dataSource.template read<int>(rakeSize.getComponents(),2);
	dataSource.template read<Scalar>(cellSize,2);
	lengthScale=dataSource.template read<Scalar>();
	shaftRadius=dataSource.template read<Scalar>();
	numArrowVertices=dataSource.template read<unsigned int>();
	dataSource.template read<Scalar>(base.getComponents(),dimension);
	for(int i=0;i<2;++i)
		dataSource.template read<Scalar>(frame[i].getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::writeBinary(
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
	dataSink.template write<int>(rakeSize.getComponents(),2);
	dataSink.template write<Scalar>(cellSize,2);
	dataSink.template write<Scalar>(lengthScale);
	dataSink.template write<Scalar>(shaftRadius);
	dataSink.template write<unsigned int>(numArrowVertices);
	dataSink.template write<Scalar>(base.getComponents(),dimension);
	for(int i=0;i<2;++i)
		dataSink.template write<Scalar>(frame[i].getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::Parameters(
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
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::read(
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
		readParameterAscii<int>(hash,"rakeSize",rakeSize.getComponents(),2);
		readParameterAscii<Scalar>(hash,"cellSize",cellSize,2);
		lengthScale=readParameterAscii<Scalar>(hash,"lengthScale",lengthScale);
		shaftRadius=readParameterAscii<Scalar>(hash,"shaftRadius",shaftRadius);
		numArrowVertices=readParameterAscii<unsigned int>(hash,"numArrowVertices",numArrowVertices);
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
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::read(
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
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::read(
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
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::write(
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
		writeParameterAscii<Misc::File,int>(file,"rakeSize",rakeSize.getComponents(),2);
		writeParameterAscii<Misc::File,Scalar>(file,"cellSize",cellSize,2);
		writeParameterAscii<Misc::File,Scalar>(file,"lengthScale",lengthScale);
		writeParameterAscii<Misc::File,Scalar>(file,"shaftRadius",shaftRadius);
		writeParameterAscii<Misc::File,unsigned int>(file,"numArrowVertices",numArrowVertices);
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
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getVectorVariableNameLength(vectorVariableIndex,variableManager);
	packetSize+=getScalarVariableNameLength(colorScalarVariableIndex,variableManager);
	packetSize+=sizeof(int)*2+sizeof(Scalar)*2;
	packetSize+=sizeof(Scalar)+sizeof(Scalar)+sizeof(unsigned int);
	packetSize+=sizeof(Scalar)*dimension+2*sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::Parameters::update(
	Visualization::Abstract::VariableManager* variableManager,
	bool track)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("ArrowRakeExtractor::Parameters::update: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("ArrowRakeExtractor::Parameters::update: Mismatching data set type");
	ds=&myDataSet->getDs();
	
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(variableManager->getVectorExtractor(vectorVariableIndex));
	if(myVectorExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::Parameters::update: Mismatching vector extractor type");
	ve=&myVectorExtractor->getVe();
	
	/* Get a pointer to the color scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(variableManager->getScalarExtractor(colorScalarVariableIndex));
	if(myScalarExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::Parameters::update: Mismatching scalar extractor type");
	cse=&myScalarExtractor->getSe();
	
	/* Get a templatized locator: */
	dsl=ds->getLocator();
	if(track)
		{
		/* Locate the rake base point: */
		locatorValid=dsl.locatePoint(base);
		}
	}

/*******************************************
Static elements of class ArrowRakeExtractor:
*******************************************/

template <class DataSetWrapperParam>
const char* ArrowRakeExtractor<DataSetWrapperParam>::name="Arrow Rake";

/***********************************
Methods of class ArrowRakeExtractor:
***********************************/

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::ArrowRakeExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager),
	 currentArrowRake(0),currentParameters(0)
	{
	/* Initialize parameters: */
	parameters.rakeSize=Index(5,5);
	baseCellSize=parameters.ds->calcAverageCellSize();
	for(int i=0;i<2;++i)
		parameters.cellSize[i]=baseCellSize;
	parameters.lengthScale=Scalar(1);
	parameters.shaftRadius=Math::div2(Scalar(Vrui::getUiSize()));
	parameters.numArrowVertices=16;
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::~ArrowRakeExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
ArrowRakeExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("ArrowRakeExtractorSettingsDialogPopup",widgetManager,"Arrow Rake Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("settingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	for(int i=0;i<2;++i)
		{
		new GLMotif::Label("RakeSizeLabel",settingsDialog,i==0?"Rake Width":"Rake Height");
		
		rakeSizeValues[i]=new GLMotif::TextField("RakeSizeValue",settingsDialog,6);
		rakeSizeValues[i]->setValue(parameters.rakeSize[i]);
		
		rakeSizeSliders[i]=new GLMotif::Slider("RakeSizeSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
		rakeSizeSliders[i]->setValueRange(1,100,1);
		rakeSizeSliders[i]->setValue(parameters.rakeSize[i]);
		rakeSizeSliders[i]->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::rakeSizeSliderCallback);
		}
	
	for(int i=0;i<2;++i)
		{
		new GLMotif::Label("CellSizeLabel",settingsDialog,i==0?"Cell Width":"Cell Height");
		
		cellSizeValues[i]=new GLMotif::TextField("CellSizeValue",settingsDialog,6);
		cellSizeValues[i]->setValue(double(parameters.cellSize[i]));
		
		cellSizeSliders[i]=new GLMotif::Slider("CellSizeSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
		cellSizeSliders[i]->setValueRange(-4.0,4.0,0.1);
		cellSizeSliders[i]->setValue(Math::log10(double(parameters.cellSize[i]/baseCellSize)));
		cellSizeSliders[i]->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::cellSizeSliderCallback);
		}
	
	new GLMotif::Label("LengthScaleLabel",settingsDialog,"Arrow Scale");
	
	lengthScaleValue=new GLMotif::TextField("LengthScaleValue",settingsDialog,12);
	lengthScaleValue->setPrecision(6);
	lengthScaleValue->setValue(double(parameters.lengthScale));
	
	lengthScaleSlider=new GLMotif::Slider("LengthScaleSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	lengthScaleSlider->setValueRange(-4.0,4.0,0.1);
	lengthScaleSlider->setValue(Math::log10(double(parameters.lengthScale)));
	lengthScaleSlider->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::lengthScaleSliderCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("ArrowRakeExtractor::setSeedLocator: Mismatching locator type");
	
	/* Create the rake frame: */
	parameters.base=Point(seedLocator->getPosition());
	for(int i=0;i<2;++i)
		{
		Vector dir=Vector(seedLocator->getOrientation().getDirection(i==0?0:2));
		parameters.frame[i]=dir;
		parameters.base-=dir*Math::div2(Scalar(parameters.rakeSize[i]))*parameters.cellSize[i];
		}
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("ArrowRakeExtractor::createElement: Mismatching parameter object type");
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new arrow rake visualization element: */
	ArrowRake* result=new ArrowRake(myParameters,myParameters->rakeSize,myParameters->lengthScale,myParameters->shaftRadius,myParameters->numArrowVertices,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Calculate the arrow base points and directions: */
	for(Index index(0);index[0]<myParameters->rakeSize[0];index.preInc(myParameters->rakeSize))
		{
		Arrow& arrow=result->getRake()(index);
		arrow.base=myParameters->base;
		for(int i=0;i<2;++i)
			arrow.base+=myParameters->frame[i]*(Scalar(index[i])*myParameters->cellSize[i]);
		
		if((arrow.valid=myParameters->dsl.locatePoint(arrow.base)))
			{
			arrow.direction=Vector(myParameters->dsl.calcValue(*myParameters->ve));
			arrow.scalarValue=Scalar(myParameters->dsl.calcValue(*myParameters->cse));
			}
		}
	result->update();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("ArrowRakeExtractor::startElement: Mismatching parameter object type");
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new arrow rake visualization element: */
	currentArrowRake=new ArrowRake(myParameters,myParameters->rakeSize,myParameters->lengthScale,myParameters->shaftRadius,myParameters->numArrowVertices,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Remember the parameter object: */
	currentParameters=myParameters;
	
	/* Return the result: */
	return currentArrowRake.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Calculate the arrow base points and directions: */
	for(Index index(0);index[0]<currentParameters->rakeSize[0];index.preInc(currentParameters->rakeSize))
		{
		Arrow& arrow=currentArrowRake->getRake()(index);
		arrow.base=currentParameters->base;
		for(int i=0;i<2;++i)
			arrow.base+=currentParameters->frame[i]*(Scalar(index[i])*currentParameters->cellSize[i]);
		
		if((arrow.valid=currentParameters->dsl.locatePoint(arrow.base)))
			{
			arrow.direction=Vector(currentParameters->dsl.calcValue(*currentParameters->ve));
			arrow.scalarValue=Scalar(currentParameters->dsl.calcValue(*currentParameters->cse));
			}
		}
	currentArrowRake->update();
	
	return true;
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	currentArrowRake=0;
	currentParameters=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("ArrowRakeExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("ArrowRakeExtractor::startElement: Mismatching parameter object type");
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new arrow rake visualization element: */
	currentArrowRake=new ArrowRake(myParameters,myParameters->rakeSize,myParameters->lengthScale,myParameters->shaftRadius,myParameters->numArrowVertices,getVariableManager()->getColorMap(csvi),getPipe());
	
	return currentArrowRake.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
		Misc::throwStdErr("ArrowRakeExtractor::continueSlaveElement: Cannot be called on master node");
	
	/* Receive the new state of the arrow rake from the master: */
	currentArrowRake->update();
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::rakeSizeSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	int dimension=cbData->slider==rakeSizeSliders[0]?0:1;
	
	/* Get the new slider value: */
	parameters.rakeSize[dimension]=int(Math::floor(double(cbData->value)+0.5));
	
	/* Update the text field: */
	rakeSizeValues[dimension]->setValue(parameters.rakeSize[dimension]);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::cellSizeSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	int dimension=cbData->slider==cellSizeSliders[0]?0:1;
	
	/* Get the new slider value and convert to cell size: */
	parameters.cellSize[dimension]=Scalar(Math::pow(10.0,double(cbData->value)))*baseCellSize;
	
	/* Update the text field: */
	cellSizeValues[dimension]->setValue(double(parameters.cellSize[dimension]));
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::lengthScaleSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to cell size: */
	parameters.lengthScale=Scalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	lengthScaleValue->setValue(double(parameters.lengthScale));
	}

}

}
