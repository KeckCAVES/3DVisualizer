/***********************************************************************
ExtractorLocator - Class for locators applying visualization algorithms
to data sets.
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

#include "ExtractorLocator.h"

#include <Misc/FunctionCalls.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSetRenderer.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>

#ifdef VISUALIZER_USE_COLLABORATION
#include "SharedVisualizationClient.h"
#endif

#include "Visualizer.h"
#include "ElementList.h"

/*********************************
Methods of class ExtractorLocator:
*********************************/

GLMotif::PopupWindow* ExtractorLocator::createBusyDialog(const char* algorithmName)
	{
	/* Create the busy dialog window: */
	GLMotif::PopupWindow* busyDialogPopup=new GLMotif::PopupWindow("BusyDialogPopup",Vrui::getWidgetManager(),"Element Extractor");
	
	GLMotif::RowColumn* busyDialog=new GLMotif::RowColumn("BusyDialog",busyDialogPopup,false);
	busyDialog->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	busyDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	char label[256];
	snprintf(label,sizeof(label),"Extracting %s...",algorithmName);
	new GLMotif::Label("BusyLabel",busyDialog,label);
	
	percentageLabel=new GLMotif::Label("PercentageLabel",busyDialog,"");
	
	busyDialog->manageChild();
	
	return busyDialogPopup;
	}

void ExtractorLocator::busyFunction(float percentageCompletion)
	{
	if(busyDialog!=0)
		{
		char percentage[10];
		snprintf(percentage,sizeof(percentage),"%5.1f",percentageCompletion);
		percentageLabel->setLabel(percentage);
		Vrui::requestUpdate();
		}
	}

ExtractorLocator::ExtractorLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,Extractor::Algorithm* sExtractor)
	:BaseLocator(sLocatorTool,sApplication),Extractor(sExtractor),
	 settingsDialog(extractor->createSettingsDialog(Vrui::getWidgetManager())),
	 busyDialog(createBusyDialog(extractor->getName())),
	 locator(application->dataSet->getLocator()),
	 dragging(false),
	 lastSeedRequestID(0)
	{
	/* Set the algorithm's busy function: */
	extractor->setBusyFunction(new Misc::VoidMethodCall<float,ExtractorLocator>(this,&ExtractorLocator::busyFunction));
	
	#ifdef VISUALIZER_USE_COLLABORATION
	if(application->sharedVisualizationClient!=0)
		{
		/* Register this locator with the shared visualization client: */
		application->sharedVisualizationClient->createLocator(this);
		}
	#endif
	
	/* Show the algorithm's settings dialog if it has one: */
	if(settingsDialog!=0)
		Vrui::popupPrimaryWidget(settingsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	}

ExtractorLocator::~ExtractorLocator(void)
	{
	#ifdef VISUALIZER_USE_COLLABORATION
	if(application->sharedVisualizationClient!=0)
		{
		/* Unregister this locator with the shared visualization client: */
		application->sharedVisualizationClient->destroyLocator(this);
		}
	#endif
	
	/* Delete the locator: */
	delete locator;
	
	/* Delete the busy dialog: */
	delete busyDialog;
	
	/* Delete the algorithm's settings dialog: */
	delete settingsDialog;
	}

void ExtractorLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Update the locator: */
	bool positionChanged=locator->setPosition(cbData->currentTransformation.getOrigin());
	positionChanged=locator->setOrientation(cbData->currentTransformation.getRotation())||positionChanged;
	
	/* Post a seed request if the locator has moved since the last frame: */
	if(dragging&&positionChanged)
		{
		/* Bump up the seed request ID: */
		if((++lastSeedRequestID)==0) // 0 is an invalid ID
			++lastSeedRequestID;
		
		if(extractor->isMaster())
			{
			/* Get extraction parameters for the current locator state from the extractor: */
			if(extractor->hasSeededCreator())
				extractor->setSeedLocator(locator);
			
			#ifdef VISUALIZER_USE_COLLABORATION
			if(application->sharedVisualizationClient!=0)
				{
				/* Send a seed request to the shared visualization server: */
				application->sharedVisualizationClient->postSeedRequest(this,lastSeedRequestID,extractor->cloneParameters());
				}
			#endif
			
			/* Post a seed request: */
			seedRequest(lastSeedRequestID,extractor->cloneParameters());
			}
		}
	
	/* Check for updates from the extraction thread: */
	ElementPointer newElement=checkUpdates();
	if(newElement!=0)
		{
		/* Add the new element to visualizer's element list: */
		application->elementList->addElement(newElement.getPointer(),extractor->getName());
		
		/* Pop down the busy dialog: */
		if(!(extractor->hasSeededCreator()&&extractor->hasIncrementalCreator())&&busyDialog!=0)
			Vrui::popdownPrimaryWidget(busyDialog);
		}
	}

void ExtractorLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Don't do anything if we're still waiting for a final extraction result: */
	if(isFinalizationPending())
		return;
	
	/* Update the locator: */
	locator->setPosition(cbData->currentTransformation.getOrigin());
	locator->setOrientation(cbData->currentTransformation.getRotation());
	
	/* Request a visualization element if it's appropriate: */
	if(!extractor->hasSeededCreator()||extractor->hasIncrementalCreator()||locator->isValid())
		{
		/* Bump up the seed request ID: */
		if((++lastSeedRequestID)==0) // 0 is an invalid ID
			++lastSeedRequestID;
		
		if(extractor->isMaster())
			{
			/* Get extraction parameters for the current locator state from the extractor: */
			if(extractor->hasSeededCreator())
				extractor->setSeedLocator(locator);
			
			#ifdef VISUALIZER_USE_COLLABORATION
			if(application->sharedVisualizationClient!=0)
				{
				/* Send a seed request to the shared visualization server: */
				application->sharedVisualizationClient->postSeedRequest(this,lastSeedRequestID,extractor->cloneParameters());
				}
			#endif
			
			/* Post a seed request: */
			seedRequest(lastSeedRequestID,extractor->cloneParameters());
			}
		}
	
	if(extractor->hasSeededCreator()&&extractor->hasIncrementalCreator())
		{
		/* Start dragging: */
		dragging=true;
		}
	else
		{
		#ifdef VISUALIZER_USE_COLLABORATION
		if(application->sharedVisualizationClient!=0)
			{
			/* Send a finalization request to the shared visualization server: */
			application->sharedVisualizationClient->postFinalizationRequest(this,lastSeedRequestID);
			}
		#endif
		
		/* Wait for the only visualization element: */
		finalize(lastSeedRequestID);
		
		/* Pop up the busy dialog: */
		if(busyDialog!=0)
			Vrui::popupPrimaryWidget(busyDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
		}
	}

void ExtractorLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	if(dragging)
		{
		#ifdef VISUALIZER_USE_COLLABORATION
		if(application->sharedVisualizationClient!=0)
			{
			/* Send a finalization request to the shared visualization server: */
			application->sharedVisualizationClient->postFinalizationRequest(this,lastSeedRequestID);
			}
		#endif
		
		/* Wait for the final visualization element: */
		finalize(lastSeedRequestID);
		
		/* Stop dragging: */
		dragging=false;
		}
	}

void ExtractorLocator::highlightLocator(GLContextData& contextData) const
	{
	/* Highlight the locator: */
	if(locator->isValid())
		application->dataSetRenderer->highlightLocator(locator,contextData);
	}

void ExtractorLocator::glRenderAction(GLContextData& contextData) const
	{
	/* Render the currently tracked element if it is opaque: */
	draw(contextData,false);
	}

void ExtractorLocator::glRenderActionTransparent(GLContextData& contextData) const
	{
	/* Render the currently tracked element if it is transparent: */
	draw(contextData,true);
	}

void ExtractorLocator::update(void)
	{
	Vrui::requestUpdate();
	}
