/***********************************************************************
Extractor - Helper class to drive multithreaded incremental or immediate
extraction of visualization elements from a data set.
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

#include "Extractor.h"

#include <Misc/Time.h>
#include <Realtime/AlarmTimer.h>
#include <Comm/MulticastPipe.h>
#include <Vrui/Vrui.h>

#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>

/**************************
Methods of class Extractor:
**************************/

void* Extractor::masterExtractorThreadMethod(void)
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
		Parameters* parameters;
		unsigned int requestID;
		{
		Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
		#ifdef __DARWIN__
		if(terminate)
			return 0;
		while(seedParameters==0)
			{
			seedRequestCond.wait(seedRequestMutex);
			if(terminate)
				return 0;
			}
		#else
		while(seedParameters==0)
			seedRequestCond.wait(seedRequestMutex);
		#endif
		
		/* Grab the seed request parameters: */
		parameters=seedParameters;
		seedParameters=0;
		
		/* Grab the seed request ID: */
		requestID=seedRequestID;
		}
		
		/* Get the next free visualization element: */
		int nextIndex=(lockedIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		
		if(parameters->isValid())
			{
			/* Prepare for extracting a new visualization element: */
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that a new visualization element is coming: */
				extractor->getPipe()->write<unsigned int>(requestID);
				
				/* Send the extraction parameters to the slaves: */
				parameters->write(*extractor->getPipe(),extractor->getVariableManager());
				extractor->getPipe()->finishMessage();
				}
			
			if(extractor->hasIncrementalCreator())
				{
				/* Start the visualization element: */
				trackedElements[nextIndex]=extractor->startElement(parameters);
				trackedElementIDs[nextIndex]=requestID;
				
				/* Continue extracting the visualization element until it is done: */
				bool keepGrowing;
				do
					{
					/* Grow the visualization element by a little bit: */
					alarm.armTimer(expirationTime);
					keepGrowing=!extractor->continueElement(alarm);
					
					/* Push this visualization element to the main thread: */
					mostRecentIndex=nextIndex;
					Vrui::requestUpdate();
					
					/* Check if there is another seed request: */
					if(keepGrowing)
						{
						Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
						keepGrowing=seedParameters==0;
						}
					
					if(extractor->getPipe()!=0)
						{
						/* Tell the slave nodes whether the current visualization element is finished: */
						extractor->getPipe()->write<unsigned int>(keepGrowing?1:0);
						extractor->getPipe()->finishMessage();
						}
					}
				while(keepGrowing);
				
				/* Finish the element: */
				extractor->finishElement();
				}
			else
				{
				/* Extract the visualization element: */
				trackedElements[nextIndex]=extractor->createElement(parameters);
				trackedElementIDs[nextIndex]=requestID;
				
				if(extractor->getPipe()!=0)
					{
					/* Tell the slave nodes that the current visualization element is finished: */
					extractor->getPipe()->write<unsigned int>(0);
					extractor->getPipe()->finishMessage();
					}
				
				/* Push this visualization element to the main thread: */
				mostRecentIndex=nextIndex;
				update();
				}
			}
		else
			{
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that there is no visualization element: */
				extractor->getPipe()->write<unsigned int>(0);
				extractor->getPipe()->write<unsigned int>(requestID);
				extractor->getPipe()->finishMessage();
				}
			
			/* Store an invalid visualization element: */
			trackedElements[nextIndex]=0;
			trackedElementIDs[nextIndex]=requestID;
			
			/* Push this visualization element to the main thread: */
			mostRecentIndex=nextIndex;
			update();
			}
		}
	
	return 0;
	}

void* Extractor::slaveExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Receive visualization elements from master until interrupted: */
	while(true)
		{
		/* Wait for a new visualization element: */
		#ifdef __DARWIN__
		if(terminate)
			return 0;
		#endif
		unsigned int requestID=extractor->getPipe()->read<unsigned int>();
		#ifdef __DARWIN__
		if(terminate)
			return 0;
		#endif
		
		/* Get the next free visualization element: */
		int nextIndex=(lockedIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		
		if(requestID!=0)
			{
			/* Receive the new element's parameters from the master: */
			Parameters* parameters=extractor->cloneParameters();
			parameters->read(*extractor->getPipe(),extractor->getVariableManager());
			
			/* Start receiving the visualization element from the master: */
			trackedElements[nextIndex]=extractor->startSlaveElement(parameters);
			trackedElementIDs[nextIndex]=requestID;
			
			/* Receive fragments of the visualization element until finished: */
			do
				{
				extractor->continueSlaveElement();
				
				/* Push this visualization element to the main thread: */
				mostRecentIndex=nextIndex;
				update();
				}
			while(extractor->getPipe()->read<unsigned int>()!=0);
			}
		else
			{
			/* Get the request ID from the master: */
			unsigned int requestID=extractor->getPipe()->read<unsigned int>();
			
			/* Store an invalid visualization element: */
			trackedElements[nextIndex]=0;
			trackedElementIDs[nextIndex]=requestID;
			
			/* Push this visualization element to the main thread: */
			mostRecentIndex=nextIndex;
			update();
			}
		}
	
	return 0;
	}

Extractor::Extractor(Extractor::Algorithm* sExtractor)
	:extractor(sExtractor),
	 #ifdef __DARWIN__
	 terminate(false),
	 #endif
	 finalElementPending(false),finalSeedRequestID(0),
	 seedParameters(0),
	 seedRequestID(0),
	 lockedIndex(0),mostRecentIndex(0)
	{
	/* Initialize the extraction thread communications: */
	for(int i=0;i<3;++i)
		{
		trackedElements[i]=0;
		trackedElementIDs[i]=0;
		}
	
	if(extractor->isMaster())
		{
		/* Start the master-side extraction thread: */
		extractorThread.start(this,&Extractor::masterExtractorThreadMethod);
		}
	else
		{
		/* Start the slave-side extraction thread: */
		extractorThread.start(this,&Extractor::slaveExtractorThreadMethod);
		}
	}

Extractor::~Extractor(void)
	{
	/* Stop the extraction thread: */
	#ifdef __DARWIN__
	if(extractor->isMaster())
		{
		if(extractor->getPipe()!=0)
			{
			/* Send a flag across the pipe to wake up and kill the extractor threads on the slave node(s): */
			extractor->getPipe()->write<unsigned int>(0);
			}
		
		/* Wake the extractor thread up to die: */
		{
		Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
		terminate=true;
		seedRequestCond.signal();
		}
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
	delete seedParameters;
	
	/* Delete the visualization element extractor: */
	delete extractor;
	}

void Extractor::seedRequest(unsigned int newSeedRequestID,Extractor::Parameters* newSeedParameters)
	{
	/* Request another visualization element extraction: */
	Threads::Mutex::Lock seedRequestLock(seedRequestMutex);
	delete seedParameters;
	seedParameters=newSeedParameters;
	seedRequestID=newSeedRequestID;
	seedRequestCond.signal();
	}

void Extractor::finalize(unsigned int newFinalSeedRequestID)
	{
	finalElementPending=true;
	finalSeedRequestID=newFinalSeedRequestID;
	}

Extractor::ElementPointer Extractor::checkUpdates(void)
	{
	/* Get the most recent visualization element from the extractor thread: */
	if(lockedIndex!=mostRecentIndex)
		{
		/* Delete the previously locked visualization element: */
		trackedElements[lockedIndex]=0;

		/* Lock the most recent visualization element: */
		lockedIndex=mostRecentIndex;
		}
	
	/* Check if the final element from a concluded dragging operation or an immediate extraction has arrived: */
	ElementPointer result=0;
	if(finalElementPending&&trackedElementIDs[lockedIndex]==finalSeedRequestID)
		{
		/* Return the new element: */
		result=trackedElements[lockedIndex];
		trackedElements[lockedIndex]=0;
		
		/* Reset the finalization marker: */
		finalElementPending=false;
		}
	
	return result;
	}

void Extractor::draw(GLContextData& contextData,bool transparent) const
	{
	/* Render the tracked visualization element if its transparency matches the parameter: */
	if(trackedElements[lockedIndex]!=0&&trackedElements[lockedIndex]->usesTransparency()==transparent)
		trackedElements[lockedIndex]->glRenderAction(contextData);
	}

void Extractor::update(void)
	{
	}
