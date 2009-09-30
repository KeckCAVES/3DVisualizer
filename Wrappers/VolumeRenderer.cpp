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

#include <Wrappers/VolumeRenderer.h>

#include <Misc/ThrowStdErr.h>
#ifndef VISUALIZATION_USE_SHADERS
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>
#endif
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>

#include <Templatized/VolumeRenderingSampler.h>
#include <Wrappers/VolumeRendererExtractor.h>

#ifdef VISUALIZATION_USE_SHADERS
#include <SingleChannelRaycaster.h>
#else
#include <PaletteRenderer.h>
#endif

namespace Visualization {

namespace Wrappers {

/*******************************
Methods of class VolumeRenderer:
*******************************/

template <class DataSetWrapperParam>
inline
VolumeRenderer<DataSetWrapperParam>::VolumeRenderer(
	Visualization::Abstract::Algorithm* algorithm,
	Visualization::Abstract::Parameters* sParameters)
	:Visualization::Abstract::Element(sParameters),
	 #ifndef VISUALIZATION_USE_SHADERS
	 colorMap(0),
	 #endif
	 renderer(0),
	 sliceFactorValue(0),sliceFactorSlider(0),transparencyGammaValue(0),transparencyGammaSlider(0)
	{
	/* Get proper pointers to the algorithm and parameter objects: */
	typedef VolumeRendererExtractor<DataSetWrapper> MyAlgorithm;
	typedef typename MyAlgorithm::Parameters MyParameters;
	MyAlgorithm* myAlgorithm=dynamic_cast<MyAlgorithm*>(algorithm);
	if(myAlgorithm==0)
		Misc::throwStdErr("VolumeRenderer: Mismatching algorithm object type");
	MyParameters* myParameters=dynamic_cast<MyParameters*>(getParameters());
	if(myParameters==0)
		Misc::throwStdErr("VolumeRenderer: Mismatching parameter object type");
	
	/* Get a reference to the templatized data set: */
	Visualization::Abstract::VariableManager* variableManager=algorithm->getVariableManager();
	const Visualization::Abstract::DataSet* dataSet=variableManager->getDataSetByScalarVariable(myParameters->scalarVariableIndex);
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(dataSet);
	if(myDataSet==0)
		Misc::throwStdErr("VolumeRenderer: Mismatching data set type");
	const DS& ds=myDataSet->getDs();
	
	/* Get a scalar extractor for the scalar variable: */
	int svi=myParameters->scalarVariableIndex;
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(variableManager->getScalarExtractor(svi));
	if(myScalarExtractor==0)
		Misc::throwStdErr("VolumeRenderer: Mismatching scalar extractor type");
	const SE& se=myScalarExtractor->getSe();

	/* Create a volume rendering sampler: */
	Visualization::Templatized::VolumeRenderingSampler<DS> sampler(ds);
	
	#ifdef VISUALIZATION_USE_SHADERS
	
	/* Initialize the raycaster: */
	renderer=new SingleChannelRaycaster(sampler.getSamplerSize(),ds.getDomainBox());
	
	/* Sample the scalar variable: */
	sampler.sample(se,renderer->getData(),renderer->getDataStrides(),algorithm->getPipe(),100.0f,0.0f,algorithm);
	
	renderer->updateData();
	
	/* Set the raycaster's parameters: */
	renderer->setColorMap(variableManager->getColorMap(svi));
	renderer->setTransparencyGamma(myParameters->transparencyGamma);
	renderer->setStepSize(myParameters->sliceFactor);
	
	#else
	
	/* Initialize the slice-based volume renderer: */
	renderer=new PaletteRenderer;
	
	/* Create a voxel block: */
	int samplerSize[3];
	for(int i=0;i<3;++i)
		samplerSize[i]=int(sampler.getSamplerSize()[i]);
	PaletteRenderer::Voxel* voxels;
	int increments[3];
	voxels=renderer->createVoxelBlock(samplerSize,0,PaletteRenderer::VERTEX_CENTERED,increments);
	
	/* Upload the data set's scalar values into the raycaster: */
	ptrdiff_t dataStrides[3];
	for(int i=0;i<3;++i)
		dataStrides[i]=increments[i];
	sampler.sample(se,voxels,dataStrides,algorithm->getPipe(),100.0f,0.0f,algorithm);
	renderer->finishVoxelBlock();
	
	/* Set the renderer's model space position and size: */
	renderer->setPosition(ds.getDomainBox().getOrigin(),ds.getDomainBox().getSize());
	
	/* Initialize volume renderer settings: */
	// renderer->setUseNPOTDTextures(true);
	renderer->setRenderingMode(PaletteRenderer::VIEW_PERPENDICULAR);
	renderer->setInterpolationMode(PaletteRenderer::LINEAR);
	renderer->setTextureFunction(PaletteRenderer::REPLACE);
	renderer->setSliceFactor(myParameters->sliceFactor);
	renderer->setAutosaveGLState(true);
	renderer->setTextureCaching(true);
	renderer->setSharePalette(false);
	#endif
	}

template <class DataSetWrapperParam>
inline
VolumeRenderer<DataSetWrapperParam>::~VolumeRenderer(
	void)
	{
	/* Destroy the volume renderer: */
	delete renderer;
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
	#ifdef VISUALIZATION_USE_SHADERS
	
	/* Return the number of cells in the raycaster: */
	return size_t(renderer->getDataSize(0)-1)*size_t(renderer->getDataSize(1)-1)*size_t(renderer->getDataSize(2)-1);
	
	#else
	
	/* Return number of cells in volume renderer: */
	return size_t(renderer->getNumCells(0))*size_t(renderer->getNumCells(1))*size_t(renderer->getNumCells(2));
	
	#endif
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
	
	#ifdef VISUALIZATION_USE_SHADERS
	double sliceFactor=renderer->getStepSize();
	#else
	double sliceFactor=renderer->getSliceFactor();
	#endif
	
	sliceFactorValue=new GLMotif::TextField("SliceFactorValue",settingsDialog,5);
	sliceFactorValue->setPrecision(3);
	sliceFactorValue->setFloatFormat(GLMotif::TextField::FIXED);
	sliceFactorValue->setValue(sliceFactor);
	
	sliceFactorSlider=new GLMotif::Slider("SliceFactorSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	sliceFactorSlider->setValueRange(0.25,4.0,0.05);
	sliceFactorSlider->setValue(sliceFactor);
	sliceFactorSlider->getValueChangedCallbacks().add(this,&VolumeRenderer::sliderValueChangedCallback);
	
	/* Create a slider/textfield combo to change the transparency gamma factor: */
	new GLMotif::Label("TransparencyGammaLabel",settingsDialog,"Transparency Gamma");
	
	#ifdef VISUALIZATION_USE_SHADERS
	float transparencyGamma=renderer->getTransparencyGamma();
	#endif
	
	transparencyGammaValue=new GLMotif::TextField("TransparencyGammaValue",settingsDialog,5);
	transparencyGammaValue->setPrecision(3);
	transparencyGammaValue->setFloatFormat(GLMotif::TextField::FIXED);
	transparencyGammaValue->setValue(transparencyGamma);
	
	transparencyGammaSlider=new GLMotif::Slider("TransparencyGammaSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	transparencyGammaSlider->setValueRange(0.125,8.0,0.025);
	transparencyGammaSlider->setValue(transparencyGamma);
	transparencyGammaSlider->getValueChangedCallbacks().add(this,&VolumeRenderer::sliderValueChangedCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
void
VolumeRenderer<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	#ifdef VISUALIZATION_USE_SHADERS
	
	/* Render the raycaster: */
	renderer->glRenderAction(contextData);
	
	#else
	
	/* Calculate the view direction: */
	typedef typename PaletteRenderer::Scalar PRScalar;
	typedef Geometry::ProjectiveTransformation<PRScalar,3> PTransform;
	typedef Geometry::HVector<PRScalar,3> HVector;
	
	/* Retrieve the viewing direction in model coordinates: */
	PTransform pmv=glGetMatrix<PRScalar>(GLMatrixEnums::PROJECTION);
	pmv*=glGetMatrix<PRScalar>(GLMatrixEnums::MODELVIEW);
	HVector x=pmv.inverseTransform(HVector(1,0,0,0));
	HVector y=pmv.inverseTransform(HVector(0,1,0,0));
	typename PaletteRenderer::Vector viewDirection=Geometry::cross(y.toVector(),x.toVector());
	viewDirection.normalize();
	
	/* Set up OpenGL state: */
	GLboolean alphaTestEnabled=glIsEnabled(GL_ALPHA_TEST);
	if(!alphaTestEnabled)
		glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.0f);
	
	/* Process the color map: */
	GLColorMap privateColorMap(*colorMap);
	privateColorMap.changeTransparency(float(renderer->getSliceFactor())*transparencyGamma);
	privateColorMap.premultiplyAlpha();
	
	/* Render the volume: */
	renderer->setSliceCenter(PaletteRenderer::Point::origin);
	renderer->setColorMap(&privateColorMap);
	renderer->renderBlock(contextData,viewDirection);
	
	/* Reset OpenGL state: */
	if(!alphaTestEnabled)
		glDisable(GL_ALPHA_TEST);
	
	#endif
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
		myParameters->sliceFactor=Scalar(cbData->value);
		#ifdef VISUALIZATION_USE_SHADERS
		renderer->setStepSize(Raycaster::Scalar(cbData->value));
		#else
		renderer->setSliceFactor(PaletteRenderer::Scalar(cbData->value));
		#endif
		
		/* Update the slice factor value text field: */
		sliceFactorValue->setValue(cbData->value);
		}
	else if(cbData->slider==transparencyGammaSlider)
		{
		/* Change the transparency gamma factor: */
		myParameters->transparencyGamma=float(cbData->value);
		#ifdef VISUALIZATION_USE_SHADERS
		renderer->setTransparencyGamma(float(cbData->value));
		#else
		transparencyGamma=float(cbData->value);
		#endif
		
		/* Update the transparency gamma value label: */
		transparencyGammaValue->setValue(cbData->value);
		}
	}

}

}
