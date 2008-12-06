/***********************************************************************
ArrowRakeExtractor - Wrapper class extract rakes of arrows from vector
fields.
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

#define VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
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
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/ArrowRakeExtractor.h>

namespace Visualization {

namespace Wrappers {

/***********************************
Methods of class ArrowRakeExtractor:
***********************************/

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::DS*
ArrowRakeExtractor<DataSetWrapperParam>::getDs(
	Visualization::Abstract::VariableManager* variableManager,
	int vectorVariableIndex,
	int colorScalarVariableIndex)
	{
	/* Get the abstract data set pointer: */
	const Visualization::Abstract::DataSet* ds1=variableManager->getDataSetByVectorVariable(vectorVariableIndex);
	const Visualization::Abstract::DataSet* ds2=variableManager->getDataSetByScalarVariable(colorScalarVariableIndex);
	if(ds1!=ds2)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Incompatible vector and scalar variables");
	
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(ds1);
	if(myDataSet==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::VE&
ArrowRakeExtractor<DataSetWrapperParam>::getVe(
	const Visualization::Abstract::VectorExtractor* sVectorExtractor)
	{
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(sVectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching vector extractor type");
	
	return myVectorExtractor->getVe();
	}

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::SE&
ArrowRakeExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::ArrowRakeExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentVectorVariable(),sVariableManager->getCurrentScalarVariable(),Index(5,5),0,1,Math::div2(Scalar(Vrui::getUiSize())),16),
	 dataSet(getDs(sVariableManager,parameters.vectorVariableIndex,parameters.colorScalarVariableIndex)),
	 vectorExtractor(getVe(sVariableManager->getVectorExtractor(parameters.vectorVariableIndex))),
	 colorScalarExtractor(getSe(sVariableManager->getScalarExtractor(parameters.colorScalarVariableIndex))),
	 currentArrowRake(0)
	{
	/* Initialize the rake cell size: */
	baseCellSize=dataSet->calcAverageCellSize();
	for(int i=0;i<2;++i)
		parameters.cellSize[i]=baseCellSize;
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::~ArrowRakeExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
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
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("ArrowRakeExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Create the rake frame: */
	Parameters newParameters=parameters;
	newParameters.base=Point(seedLocator->getPosition());
	for(int i=0;i<2;++i)
		{
		Vector dir=Vector(seedLocator->getOrientation().getDirection(i==0?0:2));
		newParameters.direction[i]=dir;
		newParameters.base-=dir*Math::div2(Scalar(newParameters.rakeSize[i]))*newParameters.cellSize[i];
		}
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new arrow rake visualization element: */
	ArrowRake* result=new ArrowRake(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	/* Calculate the arrow base points and directions: */
	for(Index index(0);index[0]<newParameters.rakeSize[0];index.preInc(newParameters.rakeSize))
		{
		Arrow& arrow=result->getRake()(index);
		arrow.base=newParameters.base;
		for(int i=0;i<2;++i)
			arrow.base+=newParameters.direction[i]*(Scalar(index[i])*newParameters.cellSize[i]);
		
		DSL locator=dsl;
		if((arrow.valid=locator.locatePoint(arrow.base)))
			{
			arrow.direction=Vector(locator.calcValue(vectorExtractor))*newParameters.lengthScale;
			arrow.scalarValue=Scalar(locator.calcValue(colorScalarExtractor));
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
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("ArrowRakeExtractor::createElement: Mismatching locator type");
	currentDsl=myLocator->getDsl();
	
	/* Create the rake frame: */
	parameters.base=Point(seedLocator->getPosition());
	for(int i=0;i<2;++i)
		{
		Vector dir=Vector(seedLocator->getOrientation().getDirection(i==0?0:2));
		parameters.direction[i]=dir;
		parameters.base-=dir*Math::div2(Scalar(parameters.rakeSize[i]))*parameters.cellSize[i];
		}
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		parameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new slice visualization element: */
	currentArrowRake=new ArrowRake(parameters,getVariableManager()->getColorMap(parameters.colorScalarVariableIndex),getPipe());
	
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
	for(Index index(0);index[0]<parameters.rakeSize[0];index.preInc(parameters.rakeSize))
		{
		Arrow& arrow=currentArrowRake->getRake()(index);
		arrow.base=parameters.base;
		for(int i=0;i<2;++i)
			arrow.base+=parameters.direction[i]*(Scalar(index[i])*parameters.cellSize[i]);
		DSL locator=currentDsl;
		if((arrow.valid=locator.locatePoint(arrow.base)))
			{
			arrow.direction=Vector(locator.calcValue(vectorExtractor))*parameters.lengthScale;
			arrow.scalarValue=Scalar(locator.calcValue(colorScalarExtractor));
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
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("ArrowRakeExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Create a new arrow rake visualization element: */
	currentArrowRake=new ArrowRake(newParameters,getVariableManager()->getColorMap(newParameters.colorScalarVariableIndex),getPipe());
	
	return currentArrowRake.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
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
