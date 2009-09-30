/***********************************************************************
TripleChannelTripleChannelVolumeRenderer - Wrapper class for triple-channel volume
renderers as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2009 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDERER_IMPLEMENTATION

#include <Wrappers/TripleChannelVolumeRenderer.h>

#include <Misc/ThrowStdErr.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>

#include <Templatized/VolumeRenderingSampler.h>
#include <Wrappers/TripleChannelVolumeRendererExtractor.h>

#include <TripleChannelRaycaster.h>

namespace Visualization {

namespace Wrappers {

/********************************************
Methods of class TripleChannelVolumeRenderer:
********************************************/

template <class DataSetWrapperParam>
inline
TripleChannelVolumeRenderer<DataSetWrapperParam>::TripleChannelVolumeRenderer(
	Visualization::Abstract::Algorithm* algorithm,
	Visualization::Abstract::Parameters* sParameters)
	:Visualization::Abstract::Element(sParameters),
	 raycaster(0),
	 sliceFactorValue(0),sliceFactorSlider(0)
	{
	for(int channel=0;channel<3;++channel)
		{
		channelEnabledToggles[channel]=0;
		transparencyGammaValues[channel]=0;
		transparencyGammaSliders[channel]=0;
		}
	
	/* Get proper pointers to the algorithm and parameter objects: */
	typedef TripleChannelVolumeRendererExtractor<DataSetWrapper> MyAlgorithm;
	typedef typename MyAlgorithm::Parameters MyParameters;
	MyAlgorithm* myAlgorithm=dynamic_cast<MyAlgorithm*>(algorithm);
	if(myAlgorithm==0)
		Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching algorithm object type");
	MyParameters* myParameters=dynamic_cast<MyParameters*>(getParameters());
	if(myParameters==0)
		Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching parameter object type");
	
	/* Check if all three scalar channels are from the same data set: */
	Visualization::Abstract::VariableManager* variableManager=algorithm->getVariableManager();
	const Visualization::Abstract::DataSet* dataSet=variableManager->getDataSetByScalarVariable(myParameters->scalarVariableIndices[0]);
	for(int channel=1;channel<3;++channel)
		if(variableManager->getDataSetByScalarVariable(myParameters->scalarVariableIndices[channel])!=dataSet)
			Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching scalar variables");
	
	/* Get a reference to the templatized data set: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(dataSet);
	if(myDataSet==0)
		Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching data set type");
	const DS& ds=myDataSet->getDs();
	
	/* Create a volume rendering sampler: */
	Visualization::Templatized::VolumeRenderingSampler<DS> sampler(ds);
	
	/* Initialize the raycaster: */
	raycaster=new TripleChannelRaycaster(sampler.getSamplerSize(),ds.getDomainBox());
	
	/* Sample the three scalar channels: */
	for(int channel=0;channel<3;++channel)
		{
		/* Get a scalar extractor for the channel: */
		int svi=myParameters->scalarVariableIndices[channel];
		const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(variableManager->getScalarExtractor(svi));
		if(myScalarExtractor==0)
			Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching scalar extractor type");
		const SE& se=myScalarExtractor->getSe();
		
		/* Sample the channel: */
		sampler.sample(se,raycaster->getData(channel),raycaster->getDataStrides(),algorithm->getPipe(),100.0f/3.0f,100.0f*float(channel)/3.0f,algorithm);
		
		/* Set the channel's parameters: */
		raycaster->setChannelEnabled(channel,myParameters->channelEnableds[channel]);
		raycaster->setColorMap(channel,variableManager->getColorMap(svi));
		raycaster->setTransparencyGamma(channel,myParameters->transparencyGammas[channel]);
		}
	raycaster->updateData();
	
	/* Set the raycaster's step size: */
	raycaster->setStepSize(myParameters->sliceFactor);
	}

template <class DataSetWrapperParam>
inline
TripleChannelVolumeRenderer<DataSetWrapperParam>::~TripleChannelVolumeRenderer(
	void)
	{
	delete raycaster;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
TripleChannelVolumeRenderer<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("TripleChannelVolumeRendererSettingsDialogPopup",widgetManager,"Triple-Channel Volume Renderer Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("SettingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(2);
	
	/* Create a slider/textfield combo to change the slice factor: */
	new GLMotif::Label("SliceFactorLabel",settingsDialog,"Slice Factor");
	
	GLMotif::RowColumn* sliceFactorBox=new GLMotif::RowColumn("SliceFactorBox",settingsDialog,false);
	sliceFactorBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	sliceFactorBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	sliceFactorValue=new GLMotif::TextField("SliceFactorValue",sliceFactorBox,5);
	sliceFactorValue->setPrecision(3);
	sliceFactorValue->setFloatFormat(GLMotif::TextField::FIXED);
	sliceFactorValue->setValue(raycaster->getStepSize());
	
	sliceFactorSlider=new GLMotif::Slider("SliceFactorSlider",sliceFactorBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	sliceFactorSlider->setValueRange(0.25,4.0,0.05);
	sliceFactorSlider->setValue(raycaster->getStepSize());
	sliceFactorSlider->getValueChangedCallbacks().add(this,&TripleChannelVolumeRenderer::sliderValueChangedCallback);
	
	sliceFactorBox->manageChild();
	
	for(int channel=0;channel<3;++channel)
		{
		/* Create a slider/textfield combo to change the transparency gamma factor: */
		char name[40],label[20];
		snprintf(name,sizeof(name),"ChannelLabel%d",channel);
		snprintf(label,sizeof(label),"Channel %d",channel+1);
		new GLMotif::Label(name,settingsDialog,label);
		
		snprintf(name,sizeof(name),"ChannelBox%d",channel);
		GLMotif::RowColumn* channelBox=new GLMotif::RowColumn(name,settingsDialog,false);
		channelBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
		channelBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
		
		channelEnabledToggles[channel]=new GLMotif::ToggleButton("ChannelEnabledToggle",channelBox,"Enabled");
		channelEnabledToggles[channel]->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
		channelEnabledToggles[channel]->setToggle(raycaster->getChannelEnabled(channel));
		channelEnabledToggles[channel]->getValueChangedCallbacks().add(this,&TripleChannelVolumeRenderer::toggleButtonValueChangedCallback);
		
		transparencyGammaValues[channel]=new GLMotif::TextField("TransparencyGammaValue",channelBox,5);
		transparencyGammaValues[channel]->setPrecision(3);
		transparencyGammaValues[channel]->setFloatFormat(GLMotif::TextField::FIXED);
		transparencyGammaValues[channel]->setValue(raycaster->getTransparencyGamma(channel));
		
		transparencyGammaSliders[channel]=new GLMotif::Slider("TransparencyGammaSlider",channelBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
		transparencyGammaSliders[channel]->setValueRange(0.125,8.0,0.025);
		transparencyGammaSliders[channel]->setValue(raycaster->getTransparencyGamma(channel));
		transparencyGammaSliders[channel]->getValueChangedCallbacks().add(this,&TripleChannelVolumeRenderer::sliderValueChangedCallback);
		
		channelBox->manageChild();
		}
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
std::string
TripleChannelVolumeRenderer<DataSetWrapperParam>::getName(
	void) const
	{
	return "Triple-Channel Volume Renderer";
	}

template <class DataSetWrapperParam>
inline
size_t
TripleChannelVolumeRenderer<DataSetWrapperParam>::getSize(
	void) const
	{
	return size_t(raycaster->getDataSize(0)-1)*size_t(raycaster->getDataSize(1)-1)*size_t(raycaster->getDataSize(2)-1);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRenderer<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Render the volume: */
	raycaster->glRenderAction(contextData);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRenderer<DataSetWrapperParam>::sliderValueChangedCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get proper pointer to the parameter object: */
	typename TripleChannelVolumeRendererExtractor<DataSetWrapper>::Parameters* myParameters=dynamic_cast<typename TripleChannelVolumeRendererExtractor<DataSetWrapper>::Parameters*>(getParameters());
	if(myParameters==0)
		Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching parameter object type");
	
	if(cbData->slider==sliceFactorSlider)
		{
		/* Change the slice factor: */
		Scalar sliceFactor=Scalar(cbData->value);
		myParameters->sliceFactor=sliceFactor;
		raycaster->setStepSize(sliceFactor);
		
		/* Update the slice factor value text field: */
		sliceFactorValue->setValue(sliceFactor);
		}
	else
		{
		for(int channel=0;channel<3;++channel)
			if(cbData->slider==transparencyGammaSliders[channel])
				{
				/* Change the transparency gamma factor: */
				float transparencyGamma=float(cbData->value);
				myParameters->transparencyGammas[channel]=transparencyGamma;
				raycaster->setTransparencyGamma(channel,transparencyGamma);
				
				/* Update the transparency gamma value label: */
				transparencyGammaValues[channel]->setValue(transparencyGamma);
				}
		}
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRenderer<DataSetWrapperParam>::toggleButtonValueChangedCallback(
	GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Get proper pointer to the parameter object: */
	typename TripleChannelVolumeRendererExtractor<DataSetWrapper>::Parameters* myParameters=dynamic_cast<typename TripleChannelVolumeRendererExtractor<DataSetWrapper>::Parameters*>(getParameters());
	if(myParameters==0)
		Misc::throwStdErr("TripleChannelVolumeRenderer: Mismatching parameter object type");
	
	for(int channel=0;channel<3;++channel)
		if(cbData->toggle==channelEnabledToggles[channel])
			{
			/* Enable or disable the channel: */
			raycaster->setChannelEnabled(channel,cbData->set);
			}
	}

}

}
