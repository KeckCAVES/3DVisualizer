/***********************************************************************
SeededColoredIsosurfaceExtractor - Wrapper class to map from the
abstract visualization algorithm interface to a templatized colored
isosurface extractor implementation.
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

#define VISUALIZATION_WRAPPERS_SEEDEDCOLOREDISOSURFACEEXTRACTOR_IMPLEMENTATION

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
#include <Templatized/ColoredIsosurfaceExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/ElementSizeLimit.h>
#include <Wrappers/AlarmTimerElement.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/SeededColoredIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/*************************************************************
Methods of class SeededColoredIsosurfaceExtractor::Parameters:
*************************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		scalarVariableIndex=dataSource.template read<int>();
	else
		scalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	if(raw)
		colorScalarVariableIndex=dataSource.template read<int>();
	else
		colorScalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	maxNumTriangles=dataSource.template read<unsigned int>();
	smoothShading=dataSource.template read<int>()!=0;
	lighting=dataSource.template read<int>()!=0;
	isovalue=dataSource.template read<VScalar>();
	dataSource.template read<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndex,variableManager);
	if(raw)
		dataSink.template write<int>(colorScalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,colorScalarVariableIndex,variableManager);
	dataSink.template write<unsigned int>(maxNumTriangles);
	dataSink.template write<int>(smoothShading?1:0);
	dataSink.template write<int>(lighting?1:0);
	dataSink.template write<VScalar>(isovalue);
	dataSink.template write<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
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
		colorScalarVariableIndex=readScalarVariableNameAscii(hash,"colorScalarVariable",variableManager);
		maxNumTriangles=readParameterAscii<unsigned int>(hash,"maxNumTriangles",maxNumTriangles);
		smoothShading=readParameterAscii<int>(hash,"smoothShading",smoothShading)!=0;
		lighting=readParameterAscii<int>(hash,"lighting",lighting)!=0;
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
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
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
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeScalarVariableNameAscii<Misc::File>(file,"scalarVariable",scalarVariableIndex,variableManager);
		writeScalarVariableNameAscii<Misc::File>(file,"colorScalarVariable",colorScalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,unsigned int>(file,"maxNumTriangles",maxNumTriangles);
		writeParameterAscii<Misc::File,int>(file,"smoothShading",smoothShading?1:0);
		writeParameterAscii<Misc::File,int>(file,"lighting",lighting?1:0);
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
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getScalarVariableNameLength(scalarVariableIndex,variableManager);
	packetSize+=getScalarVariableNameLength(colorScalarVariableIndex,variableManager);
	packetSize+=sizeof(unsigned int)+sizeof(int)+sizeof(int)+sizeof(VScalar);
	packetSize+=sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/*********************************************************
Static elements of class SeededColoredIsosurfaceExtractor:
*********************************************************/

template <class DataSetWrapperParam>
const char* SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::name="Seeded Colored Isosurface";

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
	 parameters(sVariableManager->getCurrentScalarVariable(),sVariableManager->getCurrentScalarVariable()),
	 cise(getDs(sVariableManager,parameters.scalarVariableIndex,parameters.colorScalarVariableIndex),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.colorScalarVariableIndex))),
	 currentColoredIsosurface(0),
	 maxNumTrianglesValue(0),maxNumTrianglesSlider(0),
	 colorScalarVariableBox(0),
	 extractionModeBox(0),
	 currentValue(0)
	{
	/* Initialize parameters: */
	parameters.maxNumTriangles=500000;
	parameters.smoothShading=true;
	parameters.lighting=true;
	
	/* Set the templatized colored isosurface extractor's extraction mode: */
	cise.setExtractionMode(parameters.smoothShading?CISE::SMOOTH:CISE::FLAT);
	}

template <class DataSetWrapperParam>
inline
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::~SeededColoredIsosurfaceExtractor(
	void)
	{
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
	
	extractionModeBox->addToggle("Flat Shading");
	extractionModeBox->addToggle("Smooth Shading");
	
	extractionModeBox->setSelectedToggle(parameters.smoothShading?1:0);
	extractionModeBox->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::extractionModeBoxCallback);
	
	extractionModeBox->manageChild();
	
	lightingToggle=new GLMotif::ToggleButton("LightingToggle",surfaceModeBox,"Lighting");
	lightingToggle->setBorderWidth(0.0f);
	lightingToggle->setHAlignment(GLFont::Left);
	lightingToggle->setToggle(parameters.lighting);
	lightingToggle->getValueChangedCallbacks().add(this,&SeededColoredIsosurfaceExtractor::lightingToggleCallback);
	
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
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::setSeedLocator: Mismatching locator type");
	
	/* Calculate the seeding point: */
	parameters.seedPoint=seedLocator->getPosition();
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	
	if(parameters.locatorValid)
		{
		/* Calculate the isovalue: */
		parameters.isovalue=VScalar(parameters.dsl.calcValue(cise.getScalarExtractor()));
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
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new colored isosurface visualization element: */
	ColoredIsosurface* result=new ColoredIsosurface(myParameters,myParameters->lighting,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Update the colored isosurface extractor: */
	cise.update(getDs(getVariableManager(),svi,csvi),getSe(getVariableManager()->getScalarExtractor(svi)));
	cise.setColorScalarExtractor(getSe(getVariableManager()->getScalarExtractor(csvi)));
	cise.setExtractionMode(myParameters->smoothShading?CISE::SMOOTH:CISE::FLAT);
	
	/* Extract the colored isosurface into the visualization element: */
	cise.startSeededIsosurface(myParameters->dsl,result->getSurface());
	ElementSizeLimit<ColoredIsosurface> esl(*result,myParameters->maxNumTriangles);
	cise.continueSeededIsosurface(esl);
	cise.finishSeededIsosurface();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	int csvi=myParameters->colorScalarVariableIndex;
	
	/* Create a new colored isosurface visualization element: */
	currentColoredIsosurface=new ColoredIsosurface(myParameters,myParameters->lighting,getVariableManager()->getColorMap(csvi),getPipe());
	
	/* Update the colored isosurface extractor: */
	cise.update(getDs(getVariableManager(),svi,csvi),getSe(getVariableManager()->getScalarExtractor(svi)));
	cise.setColorScalarExtractor(getSe(getVariableManager()->getScalarExtractor(csvi)));
	cise.setExtractionMode(myParameters->smoothShading?CISE::SMOOTH:CISE::FLAT);
	
	/* start extracting the colored isosurface into the visualization element: */
	cise.startSeededIsosurface(myParameters->dsl,currentColoredIsosurface->getSurface());
	
	/* Return the result: */
	return currentColoredIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the colored isosurface into the visualization element: */
	size_t maxNumTriangles=dynamic_cast<Parameters*>(currentColoredIsosurface->getParameters())->maxNumTriangles;
	AlarmTimerElement<ColoredIsosurface> atcf(alarm,*currentColoredIsosurface,maxNumTriangles);
	return cise.continueSeededIsosurface(atcf)||currentColoredIsosurface->getElementSize()>=maxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	cise.finishSeededIsosurface();
	currentColoredIsosurface=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new colored isosurface visualization element: */
	currentColoredIsosurface=new ColoredIsosurface(myParameters,myParameters->lighting,getVariableManager()->getColorMap(myParameters->colorScalarVariableIndex),getPipe());
	
	return currentColoredIsosurface.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
		Misc::throwStdErr("SeededColoredIsosurfaceExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentColoredIsosurface->getSurface().receive();
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
	cise.setColorScalarExtractor(getSe(getVariableManager()->getScalarExtractor(parameters.colorScalarVariableIndex)));
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
			parameters.smoothShading=false;
			cise.setExtractionMode(CISE::FLAT);
			break;
		
		case 1:
			parameters.smoothShading=true;
			cise.setExtractionMode(CISE::SMOOTH);
			break;
		}
	}

template <class DataSetWrapperParam>
inline
void
SeededColoredIsosurfaceExtractor<DataSetWrapperParam>::lightingToggleCallback(
	GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Set the surface lighting flag: */
	parameters.lighting=cbData->set;
	}

}

}
