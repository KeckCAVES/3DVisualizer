/***********************************************************************
DataLocator - Class for locators applying visualization algorithms to
data sets.
Copyright (c) 2005-2008 Oliver Kreylos

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

#include <Realtime/AlarmTimer.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSetRenderer.h>
#include <Abstract/Element.h>
#include <Abstract/Algorithm.h>

#include "Visualizer.h"

#define DATALOCATOR_USE_BARRIER 0

/****************************************
Methods of class Visualizer::DataLocator:
****************************************/

void* Visualizer::DataLocator::incrementalExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Handle extraction requests until interrupted: */
	Realtime::AlarmTimer alarm;
	Misc::Time expirationTime(0.1);
	while(true)
		{
		/* Wait until there is a seed request: */
		Locator* locator;
		{
		Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
		#ifdef __DARWIN__
		while(seedLocator==0)
			{
			seedRequestCond.wait(seedRequestMutex);
			if(terminate)
				return 0;
			}
		#else
		while(seedLocator==0)
			seedRequestCond.wait(seedRequestMutex);
		#endif
		locator=seedLocator;
		seedLocator=0;
		extracting=true;
		}
		
		/* Start extracting a new visualization element: */
		int nextIndex=(renderIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		
		if(locator->isValid())
			{
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that a new visualization element is coming: */
				extractor->getPipe()->write<int>(1);
				}
			trackedElements[nextIndex]=extractor->startElement(locator);
			
			/* Continue extracting the visualization element until it is done: */
			bool keepGrowing;
			do
				{
				/* Grow the visualization element by a little bit: */
				alarm.armTimer(expirationTime);
				keepGrowing=!extractor->continueElement(alarm);
				
				/* Set the most recently updated visualization element: */
				mostRecentIndex=nextIndex;
				Vrui::requestUpdate();
				
				/* Check if the current element is still being tracked: */
				{
				Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
				if(seedTracking||seedLocator!=0)
					keepGrowing=false;
				extracting=keepGrowing;
				}
				
				if(extractor->getPipe()!=0)
					{
					/* Tell the slave nodes whether the current visualization element is finished: */
					extractor->getPipe()->write<int>(keepGrowing?1:0);
					}
				}
			while(keepGrowing);
			extractor->finishElement();
			}
		else
			{
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that the currently tracked visualization element should be deleted: */
				extractor->getPipe()->write<int>(0);
				extractor->getPipe()->finishMessage();
				}
			trackedElements[nextIndex]=0;
			mostRecentIndex=nextIndex;
			Vrui::requestUpdate();
			}
		delete locator;
		#if DATALOCATOR_USE_BARRIER
		if(extractor->getPipe()!=0)
			extractor->getPipe()->barrier();
		#endif
		}
	
	return 0;
	}

void* Visualizer::DataLocator::immediateExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Handle extraction requests until interrupted: */
	while(true)
		{
		/* Wait until there is an extraction request: */
		Locator* locator;
		{
		Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
		#ifdef __DARWIN__
		do
			{
			seedRequestCond.wait(seedRequestMutex);
			if(terminate)
				return 0;
			}
		while(!extracting);
		#else
		do
			{
			seedRequestCond.wait(seedRequestMutex);
			}
		while(!extracting);
		#endif
		locator=seedLocator;
		seedLocator=0;
		}
		
		/* Extract a new visualization element: */
		int nextIndex=(renderIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		
		if(extractor->hasSeededCreator()&&locator!=0&&locator->isValid())
			{
			/* Extract a seeded element: */
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that a new visualization element is coming: */
				extractor->getPipe()->write<int>(1);
				}
			
			/* Extract the visualization element: */
			trackedElements[nextIndex]=extractor->createElement(locator);
			
			if(extractor->getPipe()!=0)
				{
				/* Tell the slave nodes that the current visualization element is finished: */
				extractor->getPipe()->write<int>(0);
				extractor->getPipe()->finishMessage();
				}
			}
		else if(!extractor->hasSeededCreator())
			{
			/* Extract a global element: */
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that a new visualization element is coming: */
				extractor->getPipe()->write<int>(1);
				}
			
			/* Extract the visualization element: */
			trackedElements[nextIndex]=extractor->createElement();
			
			if(extractor->getPipe()!=0)
				{
				/* Tell the slave nodes that the current visualization element is finished: */
				extractor->getPipe()->write<int>(0);
				extractor->getPipe()->finishMessage();
				}
			}
		else
			{
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that the currently tracked visualization element should be deleted: */
				extractor->getPipe()->write<int>(0);
				extractor->getPipe()->finishMessage();
				}
			trackedElements[nextIndex]=0;
			}
		delete locator;
		#if DATALOCATOR_USE_BARRIER
		if(extractor->getPipe()!=0)
			extractor->getPipe()->barrier();
		#endif
		
		/* Hand the new visualization element to the application: */
		mostRecentIndex=nextIndex;
		Vrui::requestUpdate();
		}
	
	return 0;
	}

void* Visualizer::DataLocator::slaveExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Receive visualization elements from master until interrupted: */
	while(true)
		{
		/* Wait for a new visualization element: */
		int validElement=extractor->getPipe()->read<int>();
		#ifdef __DARWIN__
		if(validElement==0&&terminate)
			return 0;
		#endif
		
		/* Prepare a new visualization element: */
		int nextIndex=(renderIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		if(validElement!=0)
			{
			trackedElements[nextIndex]=extractor->startSlaveElement();
			
			/* Receive fragments of the visualization element until finished: */
			do
				{
				extractor->continueSlaveElement();
				mostRecentIndex=nextIndex;
				}
			while(extractor->getPipe()->read<int>()!=0);
			}
		else
			{
			/* Invalidate the next visualization element: */
			trackedElements[nextIndex]=0;
			mostRecentIndex=nextIndex;
			}
		#if DATALOCATOR_USE_BARRIER
		extractor->getPipe()->barrier();
		#endif
		}
	
	return 0;
	}

GLMotif::PopupWindow* Visualizer::DataLocator::createBusyDialog(const char* algorithmName)
	{
	/* Create the busy dialog window: */
	GLMotif::PopupWindow* busyDialogPopup=new GLMotif::PopupWindow("BusyDialogPopup",Vrui::getWidgetManager(),"Element Extractor");
	
	char label[256];
	snprintf(label,sizeof(label),"Extracting %s...",algorithmName);
	new GLMotif::Label("BusyLabel",busyDialogPopup,label);
	
	return busyDialogPopup;
	}

Visualizer::DataLocator::DataLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,const char* algorithmName,Visualizer::Algorithm* sExtractor)
	:BaseLocator(sLocatorTool,sApplication),
	 #ifdef __DARWIN__
	 terminate(false),
	 #endif
	 extractor(sExtractor),
	 settingsDialog(extractor->createSettingsDialog(Vrui::getWidgetManager())),
	 busyDialog(createBusyDialog(algorithmName)),
	 locator(application->dataSet->getLocator()),
	 dragging(false),firstExtraction(false),
	 seedTracking(false),
	 seedLocator(0),
	 extracting(false)
	{
	/* Show the algorithm's settings dialog if it has one: */
	if(settingsDialog!=0)
		Vrui::popupPrimaryWidget(settingsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	
	/* Initialize the extraction thread communications: */
	for(int i=0;i<3;++i)
		trackedElements[i]=0;
	renderIndex=0;
	mostRecentIndex=0;
	
	if(Vrui::isMaster())
		{
		if(extractor->hasSeededCreator()&&extractor->hasIncrementalCreator())
			{
			/* Start the incremental extraction thread: */
			extractorThread.start(this,&Visualizer::DataLocator::incrementalExtractorThreadMethod);
			}
		else
			{
			/* Start the immediate extraction thread: */
			extractorThread.start(this,&Visualizer::DataLocator::immediateExtractorThreadMethod);
			}
		}
	else
		{
		/* Start the slave-side extraction thread: */
		extractorThread.start(this,&Visualizer::DataLocator::slaveExtractorThreadMethod);
		}
	}

Visualizer::DataLocator::~DataLocator(void)
	{
	/* Stop the extraction thread: */
	#ifdef __DARWIN__
	if(Vrui::isMaster())
		{
		if(extractor->getPipe()!=0)
			{
			/* Send a flag across the pipe to wake up and kill the extractor threads on the slave node(s): */
			extractor->getPipe()->write<int>(0);
			}
		
		/* Wake the extractor thread up to die: */
		Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
		terminate=true;
		seedRequestCond.signal();
		}
	else
		{
		/* Set the terminate flag and wait for the wake-up message from the master: */
		terminate=true;
		}
	#else
	extractorThread.cancel();
	#endif
	extractorThread.join();
	
	/* Clear the extractor thread communication: */
	delete seedLocator;
	
	/* Delete the locator: */
	delete locator;
	
	/* Hide and delete the busy dialog: */
	Vrui::popdownPrimaryWidget(busyDialog);
	delete busyDialog;
	
	/* Hide the algorithm's settings dialog if it has one: */
	if(settingsDialog!=0)
		Vrui::popdownPrimaryWidget(settingsDialog);
	delete settingsDialog;
	
	/* Delete the visualization element extractor: */
	delete extractor;
	}

void Visualizer::DataLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Update the locator: */
	bool positionChanged=firstExtraction;
	positionChanged=locator->setPosition(cbData->currentTransformation.getOrigin())||positionChanged;
	positionChanged=locator->setOrientation(cbData->currentTransformation.getRotation())||positionChanged;
	firstExtraction=false;
	
	if(extractor->hasSeededCreator()&&extractor->hasIncrementalCreator())
		{
		if(Vrui::isMaster()&&dragging&&positionChanged)
			{
			/* Request another visualization element extraction: */
			{
			Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
			delete seedLocator;
			seedLocator=locator->clone();
			seedRequestCond.signal();
			}
			}
		
		/* Select the most recent visualization element for rendering: */
		int storeElement=renderIndex!=mostRecentIndex&&!dragging?1:0;
		if(Vrui::getMainPipe()!=0)
			{
			Vrui::getMainPipe()->broadcast<int>(storeElement);
			Vrui::getMainPipe()->finishMessage();
			}
		
		/* Show the most recent visualization element: */
		if(renderIndex!=mostRecentIndex)
			{
			trackedElements[renderIndex]=0;
			renderIndex=mostRecentIndex;
			}
		
		/* Store the most recent visualization element in the application's list if it is the final one: */
		if(storeElement==1)
			{
			if(trackedElements[renderIndex]!=0)
				application->addElement(trackedElements[renderIndex].getPointer());
			trackedElements[renderIndex]=0;
			}
		}
	else
		{
		/* Check if the immediate extraction thread has a result: */
		int hasResult=renderIndex!=mostRecentIndex?1:0;
		if(Vrui::getMainPipe()!=0)
			{
			Vrui::getMainPipe()->broadcast<int>(hasResult);
			Vrui::getMainPipe()->finishMessage();
			}
		if(hasResult==1)
			{
			trackedElements[renderIndex]=0;
			renderIndex=mostRecentIndex;
			
			/* Store the most recent visualization element in the application's list: */
			if(trackedElements[renderIndex]!=0)
				application->addElement(trackedElements[renderIndex].getPointer());
			trackedElements[renderIndex]=0;
			
			if(Vrui::isMaster())
				{
				/* The extractor thread is idle again: */
				{
				Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
				extracting=false;
				}
				}
			
			/* Pop down the busy dialog: */
			Vrui::popdownPrimaryWidget(busyDialog);
			}
		}
	
	#if 0
	if(dragging)
		Vrui::requestUpdate();
	#endif
	}

void Visualizer::DataLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	if(extractor->hasSeededCreator())
		{
		if(extractor->hasIncrementalCreator())
			{
			/* Start dragging: */
			dragging=true;
			if(Vrui::isMaster())
				{
				/* Wake up the extraction thread: */
				firstExtraction=true;
				{
				Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
				seedTracking=true;
				}
				}
			}
		else if(locator->isValid())
			{
			bool startNewElement;
			if(Vrui::isMaster())
				{
				/* Start extracting a seeded element: */
				{
				Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
				startNewElement=!extracting;
				if(startNewElement)
					{
					delete seedLocator;
					seedLocator=locator->clone();
					extracting=true;
					seedRequestCond.signal();
					}
				}
				
				if(Vrui::getMainPipe()!=0)
					{
					Vrui::getMainPipe()->write<int>(startNewElement?1:0);
					Vrui::getMainPipe()->finishMessage();
					}
				}
			else
				startNewElement=Vrui::getMainPipe()->read<int>()!=0;
			
			if(startNewElement)
				{
				/* Pop up the busy dialog: */
				Vrui::popupPrimaryWidget(busyDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
				}
			}
		}
	else
		{
		bool startNewElement;
		if(Vrui::isMaster())
			{
			/* Start extracting a global element: */
			{
			Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
			startNewElement=!extracting;
			if(startNewElement)
				{
				delete seedLocator;
				seedLocator=0;
				extracting=true;
				seedRequestCond.signal();
				}
			}

			if(Vrui::getMainPipe()!=0)
				{
				Vrui::getMainPipe()->write<int>(startNewElement?1:0);
				Vrui::getMainPipe()->finishMessage();
				}
			}
		else
			startNewElement=Vrui::getMainPipe()->read<int>()!=0;
		
		if(startNewElement)
			{
			/* Pop up the busy dialog: */
			Vrui::popupPrimaryWidget(busyDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
			}
		}
	}

void Visualizer::DataLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	if(extractor->hasSeededCreator()&&extractor->hasIncrementalCreator())
		{
		/* Stop visualization element extraction: */
		if(Vrui::isMaster())
			{
			Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
			seedTracking=false;
			delete seedLocator;
			seedLocator=0;
			
			/* Check if the extractor needs to be woken up: */
			if(!extracting)
				{
				/* Ignore the current most recent element: */
				if(renderIndex!=mostRecentIndex)
					{
					trackedElements[renderIndex]=0;
					renderIndex=mostRecentIndex;
					}
				
				/* Start another one to completion: */
				seedLocator=locator->clone();
				seedRequestCond.signal();
				}
			}
		
		/* Stop dragging: */
		dragging=false;
		}
	}

void Visualizer::DataLocator::highlightLocator(GLContextData& contextData) const
	{
	/* Highlight the locator: */
	if(locator->isValid())
		application->dataSetRenderer->highlightLocator(locator,contextData);
	}

void Visualizer::DataLocator::glRenderAction(GLContextData& contextData) const
	{
	/* Render the tracked visualization element if it is opaque: */
	if(trackedElements[renderIndex]!=0&&!trackedElements[renderIndex]->usesTransparency())
		trackedElements[renderIndex]->glRenderAction(contextData);
	}

void Visualizer::DataLocator::glRenderActionTransparent(GLContextData& contextData) const
	{
	/* Render the tracked visualization element if it is transparent: */
	if(trackedElements[renderIndex]!=0&&trackedElements[renderIndex]->usesTransparency())
		trackedElements[renderIndex]->glRenderAction(contextData);
	}
