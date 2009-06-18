/***********************************************************************
ScalarEvaluationLocator - Class for locators evaluating scalar
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

#include "ScalarEvaluationLocator.h"

#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSet.h>
#include <Abstract/VariableManager.h>

#include "Visualizer.h"

/****************************************
Methods of class ScalarEvaluationLocator:
****************************************/

ScalarEvaluationLocator::ScalarEvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication)
	:EvaluationLocator(sLocatorTool,sApplication,"Scalar Evaluation Dialog"),
	 scalarExtractor(application->variableManager->getCurrentScalarExtractor()),
	 valueValid(false)
	{
	/* Add to the evaluation dialog: */
	new GLMotif::Label("ValueLabel",evaluationDialog,application->variableManager->getScalarVariableName(application->variableManager->getCurrentScalarVariable()));
	
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
	}

ScalarEvaluationLocator::~ScalarEvaluationLocator(void)
	{
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
			value->setLabel("");
			}
		}
	}

void ScalarEvaluationLocator::insertControlPointCallback(Misc::CallbackData* cbData)
	{
	/* Insert a new control point into the color map: */
	if(valueValid)
		application->variableManager->insertPaletteEditorControlPoint(currentValue);
	}
