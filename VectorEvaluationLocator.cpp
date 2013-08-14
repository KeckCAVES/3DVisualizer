/***********************************************************************
VectorEvaluationLocator - Class for locators evaluating vector
properties of data sets.
Copyright (c) 2008-2012 Oliver Kreylos

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

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLMaterial.h>
#include <GL/GLColorMap.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Label.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetStateHelper.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSet.h>
#include <Abstract/VariableManager.h>
#include <Wrappers/RenderArrow.h>

#include "GLRenderState.h"
#include "Visualizer.h"

/****************************************
Methods of class VectorEvaluationLocator:
****************************************/

VectorEvaluationLocator::VectorEvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg)
	:EvaluationLocator(sLocatorTool,sApplication,"Vector Evaluation Dialog"),
	 vectorExtractor(0),
	 scalarExtractor(0),
	 colorMap(application->variableManager->getCurrentColorMap()),
	 valueValid(false),
	 arrowLengthScale(1)
	{
	Visualization::Abstract::VariableManager* vm=application->variableManager;
	
	/* Get the vector and scalar extractors: */
	if(cfg!=0)
		{
		/* Read the vector variable from the configuration file: */
		std::string vectorVariableName=vm->getVectorVariableName(vm->getCurrentVectorVariable());
		vectorVariableName=cfg->retrieveValue<std::string>("./vectorVariableName",vectorVariableName);
		vectorExtractor=vm->getVectorExtractor(vm->getVectorVariable(vectorVariableName.c_str()));
		
		/* Read the scalar variable from the configuration file: */
		std::string scalarVariableName=vm->getScalarVariableName(vm->getCurrentScalarVariable());
		scalarVariableName=cfg->retrieveValue<std::string>("./scalarVariableName",scalarVariableName);
		scalarExtractor=vm->getScalarExtractor(vm->getScalarVariable(scalarVariableName.c_str()));
		}
	else
		{
		/* Get extractors for the current vector and scalar variables: */
		vectorExtractor=vm->getCurrentVectorExtractor();
		scalarExtractor=vm->getCurrentScalarExtractor();
		}
	
	/* Get the color map for the scalar extractor: */
	colorMap=vm->getColorMap(vm->getScalarVariable(scalarExtractor));
	
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=Vrui::getWidgetManager()->getStyleSheet();
	
	/* Set the dialog's title string: */
	std::string title="Evaluate Vectors -- ";
	title.append(vm->getVectorVariableName(vm->getVectorVariable(vectorExtractor)));
	evaluationDialogPopup->setTitleString(title.c_str());
	
	/* Add to the evaluation dialog: */
	new GLMotif::Label("ValueLabel",evaluationDialog,vm->getVectorVariableName(vm->getVectorVariable(vectorExtractor)));
	
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
	
	GLMotif::TextFieldSlider* arrowScaleSlider=new GLMotif::TextFieldSlider("ArrowScaleSlider",evaluationDialog,12,ss->fontHeight*10.0f);
	arrowScaleSlider->getTextField()->setPrecision(6);
	arrowScaleSlider->setSliderMapping(GLMotif::TextFieldSlider::EXP10);
	arrowScaleSlider->setValueRange(1.0e-4,1.0e4,0.1);
	arrowScaleSlider->setValue(arrowLengthScale);
	arrowScaleSlider->getValueChangedCallbacks().add(this,&VectorEvaluationLocator::arrowScaleCallback);
	
	arrowScaleBox->manageChild();
	
	evaluationDialog->manageChild();
	
	/* Pop up the evaluation dialog: */
	Vrui::popupPrimaryWidget(evaluationDialogPopup);
	
	if(cfg!=0)
		{
		/* Read the evaluation dialog's position: */
		GLMotif::readTopLevelPosition(evaluationDialogPopup,*cfg);
		}
	}

VectorEvaluationLocator::~VectorEvaluationLocator(void)
	{
	}

void VectorEvaluationLocator::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	Visualization::Abstract::VariableManager* vm=application->variableManager;
	
	/* Write the algorithm type: */
	configFileSection.storeString("./algorithm","Evaluate Vectors");
	
	/* Write the vector variable name: */
	configFileSection.storeValue<std::string>("./vectorVariableName",vm->getVectorVariableName(vm->getVectorVariable(vectorExtractor)));
	
	/* Write the scalar variable name: */
	configFileSection.storeValue<std::string>("./scalarVariableName",vm->getScalarVariableName(vm->getScalarVariable(scalarExtractor)));
	
	/* Write the evaluation dialog's position: */
	GLMotif::writeTopLevelPosition(evaluationDialogPopup,configFileSection);
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
				values[i]->setString("");
			}
		}
	}

void VectorEvaluationLocator::highlightLocator(GLRenderState& renderState) const
	{
	/* Call the base class method: */
	EvaluationLocator::highlightLocator(renderState);
	
	/* Render the evaluated vector value if valid: */
	if(valueValid)
		{
		/* Set up OpenGL state for arrow rendering: */
		renderState.enableCulling(GL_BACK);
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(false);
		glColor((*colorMap)(currentScalarValue));
		renderState.enableColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		
		/* Render an arrow glyph: */
		Scalar arrowShaftRadius=Scalar((Vrui::Scalar(0.5)*Vrui::getUiSize())/Vrui::getNavigationTransformation().getScaling());
		Visualization::Wrappers::renderArrow(point,currentValue*arrowLengthScale,arrowShaftRadius,arrowShaftRadius*Scalar(3),arrowShaftRadius*Scalar(6),16);
		}
	}

void VectorEvaluationLocator::arrowScaleCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to step size: */
	arrowLengthScale=Scalar(cbData->value);
	}
