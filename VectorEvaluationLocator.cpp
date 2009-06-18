/***********************************************************************
VectorEvaluationLocator - Class for locators evaluating vector
properties of data sets.
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

#include "VectorEvaluationLocator.h"

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLMaterial.h>
#include <GL/GLColorMap.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSet.h>
#include <Abstract/VariableManager.h>
#include <Wrappers/RenderArrow.h>

#include "Visualizer.h"

/****************************************
Methods of class VectorEvaluationLocator:
****************************************/

VectorEvaluationLocator::VectorEvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication)
	:EvaluationLocator(sLocatorTool,sApplication,"Vector Evaluation Dialog"),
	 vectorExtractor(application->variableManager->getCurrentVectorExtractor()),
	 scalarExtractor(application->variableManager->getCurrentScalarExtractor()),
	 colorMap(application->variableManager->getCurrentColorMap()),
	 valueValid(false),
	 arrowLengthScale(1)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=Vrui::getWidgetManager()->getStyleSheet();
	
	/* Add to the evaluation dialog: */
	new GLMotif::Label("ValueLabel",evaluationDialog,application->variableManager->getVectorVariableName(application->variableManager->getCurrentVectorVariable()));
	
	GLMotif::RowColumn* valueBox=new GLMotif::RowColumn("ValueBox",evaluationDialog,false);
	valueBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	valueBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Value-%d",i+1);
		values[i]=new GLMotif::TextField(labelName,valueBox,12);
		values[i]->setPrecision(6);
		}
	
	valueBox->manageChild();
	
	new GLMotif::Label("ArrowScaleLabel",evaluationDialog,"Arrow Scale");
	
	GLMotif::RowColumn* arrowScaleBox=new GLMotif::RowColumn("ArrowScaleBox",evaluationDialog,false);
	arrowScaleBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	arrowScaleBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	arrowScaleValue=new GLMotif::TextField("ArrowScaleValue",arrowScaleBox,12);
	arrowScaleValue->setPrecision(6);
	arrowScaleValue->setValue(arrowLengthScale);
	
	arrowScaleSlider=new GLMotif::Slider("ArrowScaleSlider",arrowScaleBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	arrowScaleSlider->setValueRange(-4.0,4.0,0.1);
	arrowScaleSlider->setValue(Math::log10(arrowLengthScale));
	arrowScaleSlider->getValueChangedCallbacks().add(this,&VectorEvaluationLocator::arrowScaleSliderCallback);
	
	arrowScaleBox->manageChild();
	
	evaluationDialog->manageChild();
	
	/* Pop up the evaluation dialog: */
	Vrui::popupPrimaryWidget(evaluationDialogPopup,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	}

VectorEvaluationLocator::~VectorEvaluationLocator(void)
	{
	}

void VectorEvaluationLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Call the base class method: */
	EvaluationLocator::motionCallback(cbData);
	
	if(dragging)
		{
		/* Get the current position of the locator in model coordinates: */
		point=locator->getPosition();
		
		/* Evaluate the data set at the locator's position: */
		if(locator->isValid())
			{
			valueValid=true;
			currentScalarValue=locator->calcScalar(scalarExtractor);
			currentValue=locator->calcVector(vectorExtractor);
			for(int i=0;i<3;++i)
				values[i]->setValue(currentValue[i]);
			}
		else
			{
			valueValid=false;
			for(int i=0;i<3;++i)
				values[i]->setLabel("");
			}
		}
	}

void VectorEvaluationLocator::highlightLocator(GLContextData& contextData) const
	{
	/* Render the evaluated vector value if valid: */
	if(valueValid)
		{
		/* Set up OpenGL state for arrow rendering: */
		GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
		if(!lightingEnabled)
			glEnable(GL_LIGHTING);
		GLboolean normalizeEnabled=glIsEnabled(GL_NORMALIZE);
		if(!normalizeEnabled)
			glEnable(GL_NORMALIZE);
		GLboolean colorMaterialEnabled=glIsEnabled(GL_COLOR_MATERIAL);
		if(colorMaterialEnabled)
			glDisable(GL_COLOR_MATERIAL);
		GLMaterial frontMaterial=glGetMaterial(GLMaterialEnums::FRONT);
		GLMaterial::Color arrowColor=(*colorMap)(currentScalarValue);
		glMaterial(GLMaterialEnums::FRONT,GLMaterial(arrowColor,GLMaterial::Color(0.6f,0.6f,0.6f),25.0f));
		
		/* Render an arrow glyph: */
		Scalar arrowShaftRadius=Scalar((Vrui::Scalar(0.5)*Vrui::getUiSize())/Vrui::getNavigationTransformation().getScaling());
		Visualization::Wrappers::renderArrow(point,currentValue*arrowLengthScale,arrowShaftRadius,arrowShaftRadius*Scalar(3),arrowShaftRadius*Scalar(6),16);
		
		/* Reset OpenGL state: */
		glMaterial(GLMaterialEnums::FRONT,frontMaterial);
		if(colorMaterialEnabled)
			glEnable(GL_COLOR_MATERIAL);
		if(!normalizeEnabled)
			glDisable(GL_NORMALIZE);
		if(!lightingEnabled)
			glDisable(GL_LIGHTING);
		}
	}

void VectorEvaluationLocator::arrowScaleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to step size: */
	arrowLengthScale=Scalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	arrowScaleValue->setValue(arrowLengthScale);
	}
