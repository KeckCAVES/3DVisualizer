/***********************************************************************
ScalarEvaluationLocator - Class for locators evaluating scalar
properties of data sets.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include "ScalarEvaluationLocator.h"

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetStateHelper.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSet.h>
#include <Abstract/VariableManager.h>

#include "Visualizer.h"

/****************************************
Methods of class ScalarEvaluationLocator:
****************************************/

ScalarEvaluationLocator::ScalarEvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg)
	:EvaluationLocator(sLocatorTool,sApplication,""),
	 scalarExtractor(0),
	 valueValid(false)
	{
	Visualization::Abstract::VariableManager* vm=application->variableManager;
	
	/* Get the scalar extractor: */
	if(cfg!=0)
		{
		/* Read the scalar variable from the configuration file: */
		std::string scalarVariableName=vm->getScalarVariableName(vm->getCurrentScalarVariable());
		scalarVariableName=cfg->retrieveValue<std::string>("./scalarVariableName",scalarVariableName);
		scalarExtractor=vm->getScalarExtractor(vm->getScalarVariable(scalarVariableName.c_str()));
		}
	else
		{
		/* Get an extractor for the current scalar variable: */
		scalarExtractor=vm->getCurrentScalarExtractor();
		}
	
	/* Set the dialog's title string: */
	std::string title="Evaluate Scalars -- ";
	title.append(vm->getScalarVariableName(vm->getScalarVariable(scalarExtractor)));
	evaluationDialogPopup->setTitleString(title.c_str());
	
	/* Add to the evaluation dialog: */
	new GLMotif::Label("ValueLabel",evaluationDialog,vm->getScalarVariableName(vm->getScalarVariable(scalarExtractor)));
	
	GLMotif::RowColumn* valueBox=new GLMotif::RowColumn("ValueBox",evaluationDialog,false);
	valueBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	valueBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	value=new GLMotif::TextField("Value",valueBox,16);
	value->setPrecision(10);
	
	new GLMotif::Blind("Blind1",valueBox);
	
	valueBox->manageChild();
	
	new GLMotif::Blind("Blind2",evaluationDialog);
	
	GLMotif::Margin* insertControlPointMargin=new GLMotif::Margin("ValueMargin",evaluationDialog,false);
	insertControlPointMargin->setAlignment(GLMotif::Alignment::RIGHT);
	
	GLMotif::Button* insertControlPointButton=new GLMotif::Button("InsertControlPointButton",insertControlPointMargin,"Insert Color Map Control Point");
	insertControlPointButton->getSelectCallbacks().add(this,&ScalarEvaluationLocator::insertControlPointCallback);
	
	insertControlPointMargin->manageChild();
	
	evaluationDialog->manageChild();
	
	/* Pop up the evaluation dialog: */
	Vrui::popupPrimaryWidget(evaluationDialogPopup,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	
	if(cfg!=0)
		{
		/* Read the evaluation dialog's position: */
		GLMotif::readTopLevelPosition(evaluationDialogPopup,*cfg);
		}
	}

ScalarEvaluationLocator::~ScalarEvaluationLocator(void)
	{
	}

void ScalarEvaluationLocator::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	Visualization::Abstract::VariableManager* vm=application->variableManager;
	
	/* Write the algorithm type: */
	configFileSection.storeString("./algorithm","Evaluate Scalars");
	
	/* Write the scalar variable name: */
	configFileSection.storeValue<std::string>("./scalarVariableName",vm->getScalarVariableName(vm->getScalarVariable(scalarExtractor)));
	
	/* Write the evaluation dialog's position: */
	GLMotif::writeTopLevelPosition(evaluationDialogPopup,configFileSection);
	}

void ScalarEvaluationLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
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
			currentValue=locator->calcScalar(scalarExtractor);
			value->setValue(currentValue);
			}
		else
			{
			valueValid=false;
			value->setString("");
			}
		}
	}

void ScalarEvaluationLocator::insertControlPointCallback(Misc::CallbackData* cbData)
	{
	/* Insert a new control point into the color map: */
	if(valueValid)
		application->variableManager->insertPaletteEditorControlPoint(currentValue);
	}
