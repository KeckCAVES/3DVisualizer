/***********************************************************************
VolumeRenderer - Wrapper class for volume renderers as visualization
elements.
Part of the wrapper layer of the templatized visualization
components.
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

#define VISUALIZATION_WRAPPERS_VOLUMERENDERER_IMPLEMENTATION

#include <Geometry/Vector.h>
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>

#include <Wrappers/VolumeRendererExtractor.h>

#include <Wrappers/VolumeRenderer.h>

namespace Visualization {

namespace Wrappers {

/*******************************
Methods of class VolumeRenderer:
*******************************/

template <class DataSetWrapperParam>
inline
VolumeRenderer<DataSetWrapperParam>::VolumeRenderer(
	Visualization::Abstract::Parameters* sParameters,
	typename VolumeRenderer<DataSetWrapperParam>::Scalar sSliceFactor,
	float sTransparencyGamma,
	const typename VolumeRenderer<DataSetWrapperParam>::DS* sDs,
	const typename VolumeRenderer<DataSetWrapperParam>::SE& sSe,
	const GLColorMap* sColorMap,
	Comm::MulticastPipe* pipe)
	:Visualization::Abstract::Element(sParameters),
	 sliceFactor(sSliceFactor),transparencyGamma(sTransparencyGamma),
	 svr(sDs,sSe,sColorMap,pipe),
	 sliceFactorValue(0),sliceFactorSlider(0),
	 transparencyGammaValue(0),transparencyGammaSlider(0)
	{
	/* Set the templatized volume renderer's parameters: */
	svr.setSliceFactor(sliceFactor);
	svr.setTransparencyGamma(transparencyGamma);
	}

template <class DataSetWrapperParam>
inline
VolumeRenderer<DataSetWrapperParam>::~VolumeRenderer(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
VolumeRenderer<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("VolumeRendererSettingsDialogPopup",widgetManager,"Volume Renderer Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("SettingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	/* Create a slider/textfield combo to change the slice factor: */
	new GLMotif::Label("SliceFactorLabel",settingsDialog,"Slice Factor");
	
	sliceFactorValue=new GLMotif::TextField("SliceFactorValue",settingsDialog,5);
	sliceFactorValue->setPrecision(3);
	sliceFactorValue->setFloatFormat(GLMotif::TextField::FIXED);
	sliceFactorValue->setValue(double(sliceFactor));
	
	sliceFactorSlider=new GLMotif::Slider("SliceFactorSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	sliceFactorSlider->setValueRange(0.25,4.0,0.05);
	sliceFactorSlider->setValue(double(sliceFactor));
	sliceFactorSlider->getValueChangedCallbacks().add(this,&VolumeRenderer::sliderValueChangedCallback);
	
	/* Create a slider/textfield combo to change the transparency gamma factor: */
	new GLMotif::Label("TransparencyGammaLabel",settingsDialog,"Transparency Gamma");
	
	transparencyGammaValue=new GLMotif::TextField("TransparencyGammaValue",settingsDialog,5);
	transparencyGammaValue->setPrecision(3);
	transparencyGammaValue->setFloatFormat(GLMotif::TextField::FIXED);
	transparencyGammaValue->setValue(double(transparencyGamma));
	
	transparencyGammaSlider=new GLMotif::Slider("TransparencyGammaSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	transparencyGammaSlider->setValueRange(0.125,8.0,0.025);
	transparencyGammaSlider->setValue(double(transparencyGamma));
	transparencyGammaSlider->getValueChangedCallbacks().add(this,&VolumeRenderer::sliderValueChangedCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
std::string
VolumeRenderer<DataSetWrapperParam>::getName(
	void) const
	{
	return "Volume Renderer";
	}

template <class DataSetWrapperParam>
inline
size_t
VolumeRenderer<DataSetWrapperParam>::getSize(
	void) const
	{
	return svr.getSize();
	}

template <class DataSetWrapperParam>
inline
void
VolumeRenderer<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Calculate the view direction: */
	typedef typename SVR::Scalar Scalar;
	typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
	typedef Geometry::HVector<Scalar,3> HVector;
	
	/* Retrieve the viewing direction in model coordinates: */
	PTransform pmv=glGetMatrix<Scalar>(GLMatrixEnums::PROJECTION);
	pmv*=glGetMatrix<Scalar>(GLMatrixEnums::MODELVIEW);
	HVector x=pmv.inverseTransform(HVector(1,0,0,0));
	HVector y=pmv.inverseTransform(HVector(0,1,0,0));
	typename SVR::Vector viewDirection=Geometry::cross(y.toVector(),x.toVector());
	viewDirection.normalize();
	
	/* Render the volume: */
	svr.renderVolume(SVR::Point::origin,viewDirection,contextData);
	}

template <class DataSetWrapperParam>
inline
void
VolumeRenderer<DataSetWrapperParam>::sliderValueChangedCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get proper pointer to the parameter object: */
	typename VolumeRendererExtractor<DataSetWrapper>::Parameters* myParameters=dynamic_cast<typename VolumeRendererExtractor<DataSetWrapper>::Parameters*>(getParameters());
	if(myParameters==0)
		Misc::throwStdErr("VolumeRenderer: Mismatching parameter object type");
	
	if(cbData->slider==sliceFactorSlider)
		{
		/* Change the slice factor: */
		sliceFactor=Scalar(cbData->value);
		myParameters->sliceFactor=sliceFactor;
		svr.setSliceFactor(sliceFactor);
		
		/* Update the slice factor value text field: */
		sliceFactorValue->setValue(double(sliceFactor));
		}
	else if(cbData->slider==transparencyGammaSlider)
		{
		/* Change the transparency gamma factor: */
		transparencyGamma=float(cbData->value);
		myParameters->transparencyGamma=transparencyGamma;
		svr.setTransparencyGamma(transparencyGamma);
		
		/* Update the transparency gamma value label: */
		transparencyGammaValue->setValue(double(transparencyGamma));
		}
	}

}

}
