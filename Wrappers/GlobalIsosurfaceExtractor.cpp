/***********************************************************************
GlobalIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
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

#define VISUALIZATION_WRAPPERS_GLOBALISOSURFACEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>
#include <Math/Math.h>
#include <Math/Constants.h>
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
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/GlobalIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/******************************************************
Methods of class GlobalIsosurfaceExtractor::Parameters:
******************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		scalarVariableIndex=dataSource.template read<int>();
	else
		scalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	smoothShading=dataSource.template read<int>()!=0;
	isovalue=dataSource.template read<VScalar>();
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndex,variableManager);
	dataSink.template write<int>(smoothShading?1:0);
	dataSink.template write<VScalar>(isovalue);
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
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
		smoothShading=readParameterAscii<int>(hash,"smoothShading",smoothShading)!=0;
		isovalue=readParameterAscii<VScalar>(hash,"isovalue",isovalue);
		
		/* Clean up: */
		deleteAsciiParameterFileSectionHash(hash);
		}
	else
		{
		/* Read from binary file: */
		readBinary(file,false,variableManager);
		}
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::ClusterPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read (and ignore) the parameter packet size from the cluster pipe: */
	pipe.read<unsigned int>();
	
	/* Read from cluster pipe: */
	readBinary(pipe,false,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeScalarVariableNameAscii<Misc::File>(file,"scalarVariable",scalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,int>(file,"smoothShading",smoothShading?1:0);
		writeParameterAscii<Misc::File,VScalar>(file,"isovalue",isovalue);
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
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getScalarVariableNameLength(scalarVariableIndex,variableManager);
	packetSize+=sizeof(int)+sizeof(VScalar);
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/**************************************************
Static elements of class GlobalIsosurfaceExtractor:
**************************************************/

template <class DataSetWrapperParam>
const char* GlobalIsosurfaceExtractor<DataSetWrapperParam>::name="Global Isosurface";

/******************************************
Methods of class GlobalIsosurfaceExtractor:
******************************************/

template <class DataSetWrapperParam>
inline
const typename GlobalIsosurfaceExtractor<DataSetWrapperParam>::DS*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::GlobalIsosurfaceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename GlobalIsosurfaceExtractor<DataSetWrapperParam>::SE&
GlobalIsosurfaceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::GlobalIsosurfaceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
GlobalIsosurfaceExtractor<DataSetWrapperParam>::GlobalIsosurfaceExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable()),
	 ise(getDs(sVariableManager->getDataSetByScalarVariable(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex))),
	 valueRange(sVariableManager->getScalarValueRange(parameters.scalarVariableIndex)),
	 extractionModeBox(0),isovalueValue(0),isovalueSlider(0)
	{
	/* Initialize parameters: */
	parameters.smoothShading=true;
	parameters.isovalue=Math::mid(valueRange.first,valueRange.second);
	
	/* Set the templatized isosurface extractor's extraction mode: */
	ise.setExtractionMode(parameters.smoothShading?ISE::SMOOTH:ISE::FLAT);
	}

template <class DataSetWrapperParam>
inline
GlobalIsosurfaceExtractor<DataSetWrapperParam>::~GlobalIsosurfaceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("GlobalIsosurfaceExtractorSettingsDialogPopup",widgetManager,"Global Isosurface Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("SettingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("ExtractionModeLabel",settingsDialog,"Extraction Mode");
	
	extractionModeBox=new GLMotif::RadioBox("ExtractionModeBox",settingsDialog,false);
	extractionModeBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	extractionModeBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	extractionModeBox->setAlignment(GLMotif::Alignment::LEFT);
	extractionModeBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	extractionModeBox->addToggle("Flat Shading");
	extractionModeBox->addToggle("Smooth Shading");
	
	extractionModeBox->setSelectedToggle(parameters.smoothShading?1:0);
	extractionModeBox->getValueChangedCallbacks().add(this,&GlobalIsosurfaceExtractor::extractionModeBoxCallback);
	
	extractionModeBox->manageChild();
	
	new GLMotif::Label("IsovalueLabel",settingsDialog,"Isovalue");
	
	GLMotif::RowColumn* isovalueBox=new GLMotif::RowColumn("IsovalueBox",settingsDialog,false);
	isovalueBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	
	isovalueValue=new GLMotif::TextField("IsovalueValue",isovalueBox,12);
	isovalueValue->setValue(parameters.isovalue);
	
	isovalueSlider=new GLMotif::Slider("IsovalueSlider",isovalueBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*20.0f);
	isovalueSlider->setValueRange(valueRange.first,valueRange.second,0.0);
	isovalueSlider->setValue(parameters.isovalue);
	isovalueSlider->getValueChangedCallbacks().add(this,&GlobalIsosurfaceExtractor::isovalueSliderCallback);
	
	isovalueBox->manageChild();
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	
	/* Create a new isosurface visualization element: */
	Isosurface* result=new Isosurface(myParameters,myParameters->isovalue,getVariableManager()->getColorMap(svi),getPipe());
	
	/* Update the isosurface extractor: */
	ise.update(getDs(getVariableManager()->getDataSetByScalarVariable(svi)),getSe(getVariableManager()->getScalarExtractor(svi)));
	
	/* Set the templatized isosurface extractor's extraction mode: */
	ise.setExtractionMode(myParameters->smoothShading?ISE::SMOOTH:ISE::FLAT);
	
	/* Extract the isosurface into the visualization element: */
	ise.extractIsosurface(myParameters->isovalue,result->getSurface());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("GlobalIsosurfaceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new isosurface visualization element: */
	Isosurface* result=new Isosurface(myParameters,myParameters->isovalue,getVariableManager()->getColorMap(myParameters->scalarVariableIndex),getPipe());
	
	/* Receive the isosurface from the master: */
	result->getSurface().receive();
	
	return result;
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::extractionModeBoxCallback(
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

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::isovalueSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new isovalue: */
	parameters.isovalue=VScalar(cbData->value);
	
	/* Update the text field: */
	isovalueValue->setValue(parameters.isovalue);
	}

}

}
