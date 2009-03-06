/***********************************************************************
SeededIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
implementation.
Copyright (c) 2005-2009 Oliver Kreylos

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
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>
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
#include <Wrappers/ElementSizeLimit.h>
#include <Wrappers/AlarmTimerElement.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/SeededIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/******************************************************
Methods of class SeededIsosurfaceExtractor::Parameters:
******************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		scalarVariableIndex=dataSource.template read<int>();
	else
		scalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	maxNumTriangles=dataSource.template read<unsigned int>();
	smoothShading=dataSource.template read<int>()!=0;
	isovalue=dataSource.template read<VScalar>();
	dataSource.template read<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndex,variableManager);
	dataSink.template write<unsigned int>(maxNumTriangles);
	dataSink.template write<int>(smoothShading?1:0);
	dataSink.template write<VScalar>(isovalue);
	dataSink.template write<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Misc::File& file,
	bool ascii,
	Visualization::Abstract::VariableManager* variableManager)
	{
	if(ascii)
		{
		/* Parse the parameter section: */
		AsciiParameterFileSectionHash* hash=parseAsciiParameterFileSection<Misc::File>(file);
		
		/* Extract the parameters: */
		scalarVariableIndex=readScalarVariableNameAscii(hash,"scalarVariable",variableManager);
		maxNumTriangles=readParameterAscii<unsigned int>(hash,"maxNumTriangles",maxNumTriangles);
		smoothShading=readParameterAscii<int>(hash,"smoothShading",smoothShading)!=0;
		isovalue=readParameterAscii<VScalar>(hash,"isovalue",isovalue);
		seedPoint=readParameterAscii<Point>(hash,"seedPoint",seedPoint);
		
		/* Clean up: */
		deleteAsciiParameterFileSectionHash(hash);
		}
	else
		{
		/* Read from binary file: */
		readBinary(file,false,variableManager);
		}
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::ClusterPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read (and ignore) the parameter packet size from the cluster pipe: */
	pipe.read<unsigned int>();
	
	/* Read from cluster pipe: */
	readBinary(pipe,false,variableManager);
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeScalarVariableNameAscii<Misc::File>(file,"scalarVariable",scalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,unsigned int>(file,"maxNumTriangles",maxNumTriangles);
		writeParameterAscii<Misc::File,int>(file,"smoothShading",smoothShading?1:0);
		writeParameterAscii<Misc::File,VScalar>(file,"isovalue",isovalue);
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
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getScalarVariableNameLength(scalarVariableIndex,variableManager);
	packetSize+=sizeof(unsigned int)+sizeof(int)+sizeof(VScalar);
	packetSize+=sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/**************************************************
Static elements of class SeededIsosurfaceExtractor:
**************************************************/

template <class DataSetWrapperParam>
const char* SeededIsosurfaceExtractor<DataSetWrapperParam>::name="Seeded Isosurface";

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
	 parameters(sVariableManager->getCurrentScalarVariable()),
	 ise(getDs(sVariableManager->getDataSetByScalarVariable(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex))),
	 currentIsosurface(0),
	 maxNumTrianglesValue(0),maxNumTrianglesSlider(0),
	 extractionModeBox(0),currentValue(0)
	{
	/* Initialize parameters: */
	parameters.maxNumTriangles=500000;
	parameters.smoothShading=true;
	
	/* Set the templatized isosurface extractor's extraction mode: */
	ise.setExtractionMode(parameters.smoothShading?ISE::SMOOTH:ISE::FLAT);
	}

template <class DataSetWrapperParam>
inline
SeededIsosurfaceExtractor<DataSetWrapperParam>::~SeededIsosurfaceExtractor(
	void)
	{
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
	
	extractionModeBox->addToggle("Flat Shading");
	extractionModeBox->addToggle("Smooth Shading");
	
	extractionModeBox->setSelectedToggle(parameters.smoothShading?1:0);
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
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::setSeedLocator: Mismatching locator type");
	
	/* Calculate the seeding point: */
	parameters.seedPoint=seedLocator->getPosition();
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	
	if(parameters.locatorValid)
		{
		/* Calculate the isovalue: */
		parameters.isovalue=VScalar(parameters.dsl.calcValue(ise.getScalarExtractor()));
		}
	
	/* Update the GUI: */
	if(currentValue!=0)
		{
		if(parameters.locatorValid)
			currentValue->setValue(double(parameters.isovalue));
		else
			currentValue->setLabel("");
		}
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	
	/* Create a new isosurface visualization element: */
	Isosurface* result=new Isosurface(myParameters,myParameters->isovalue,getVariableManager()->getColorMap(svi),getPipe());
	
	/* Update the isosurface extractor: */
	ise.update(getDs(getVariableManager()->getDataSetByScalarVariable(svi)),getSe(getVariableManager()->getScalarExtractor(svi)));
	ise.setExtractionMode(myParameters->smoothShading?ISE::SMOOTH:ISE::FLAT);
	
	/* Extract the isosurface into the visualization element: */
	ise.startSeededIsosurface(myParameters->dsl,result->getSurface());
	ElementSizeLimit<Isosurface> esl(*result,myParameters->maxNumTriangles);
	ise.continueSeededIsosurface(esl);
	ise.finishSeededIsosurface();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededIsosurfaceExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	
	/* Create a new isosurface visualization element: */
	currentIsosurface=new Isosurface(myParameters,myParameters->isovalue,getVariableManager()->getColorMap(svi),getPipe());
	
	/* Update the isosurface extractor: */
	ise.update(getDs(getVariableManager()->getDataSetByScalarVariable(svi)),getSe(getVariableManager()->getScalarExtractor(svi)));
	ise.setExtractionMode(myParameters->smoothShading?ISE::SMOOTH:ISE::FLAT);
	
	/* Start extracting the isosurface into the visualization element: */
	ise.startSeededIsosurface(myParameters->dsl,currentIsosurface->getSurface());
	
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
	size_t maxNumTriangles=dynamic_cast<Parameters*>(currentIsosurface->getParameters())->maxNumTriangles;
	AlarmTimerElement<Isosurface> atcf(alarm,*currentIsosurface,maxNumTriangles);
	return ise.continueSeededIsosurface(atcf)||currentIsosurface->getElementSize()>=maxNumTriangles;
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
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("SeededIsosurfaceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededIsosurfaceExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new isosurface visualization element: */
	currentIsosurface=new Isosurface(myParameters,myParameters->isovalue,getVariableManager()->getColorMap(myParameters->scalarVariableIndex),getPipe());
	
	return currentIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededIsosurfaceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
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
			parameters.smoothShading=false;
			ise.setExtractionMode(ISE::FLAT);
			break;
		
		case 1:
			parameters.smoothShading=true;
			ise.setExtractionMode(ISE::SMOOTH);
			break;
		}
	}

}

}
