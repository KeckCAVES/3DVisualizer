/***********************************************************************
Extractor - Helper class to drive multithreaded incremental or immediate
extraction of visualization elements from a data set.
Copyright (c) 2009-2012 Oliver Kreylos

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
#include <Threads/Config.h>
#include <Realtime/AlarmTimer.h>
#include <Cluster/MulticastPipe.h>
#include <Vrui/Vrui.h>

#include <Abstract/Parameters.h>
#include <Abstract/BinaryParametersSink.h>
#include <Abstract/BinaryParametersSource.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>

/**************************
Methods of class Extractor:
**************************/

void* Extractor::masterExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Create a data sink for the multicast pipe: */
	Visualization::Abstract::BinaryParametersSink sink(extractor->getVariableManager(),*extractor->getPipe(),true);
	
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
		#if !THREADS_CONFIG_CAN_CANCEL
		while(!terminate&&seedParameters==0)
			seedRequestCond.wait(seedRequestMutex);
		if(terminate)
			return 0;
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
		
		/* Start a new visualization element: */
		std::pair<ElementPointer,unsigned int>& element=trackedElements.startNewValue();
		if(parameters->isValid())
			{
			/* Prepare for extracting a new visualization element: */
			if(extractor->getPipe()!=0)
				{
				/* Notify the slave nodes that a new visualization element is coming: */
				extractor->getPipe()->write<unsigned int>(requestID);
				
				/* Send the extraction parameters to the slaves: */
				parameters->write(sink);
				extractor->getPipe()->flush();
				}
			
			if(extractor->hasIncrementalCreator())
				{
				/* Start the visualization element: */
				element.first=extractor->startElement(parameters);
				element.second=requestID;
				
				/* Continue extracting the visualization element until it is done: */
				bool keepGrowing;
				do
					{
					/* Grow the visualization element by a little bit: */
					alarm.armTimer(expirationTime);
					keepGrowing=!extractor->continueElement(alarm);
					
					/* Push this visualization element to the main thread: */
					trackedElements.postNewValue();
					update();
					
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
						extractor->getPipe()->flush();
						}
					}
				while(keepGrowing);
				
				/* Finish the element: */
				extractor->finishElement();
				}
			else
				{
				/* Extract the visualization element: */
				element.first=extractor->createElement(parameters);
				element.second=requestID;
				
				if(extractor->getPipe()!=0)
					{
					/* Tell the slave nodes that the current visualization element is finished: */
					extractor->getPipe()->write<unsigned int>(0);
					extractor->getPipe()->flush();
					}
				
				/* Push this visualization element to the main thread: */
				trackedElements.postNewValue();
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
				extractor->getPipe()->flush();
				}
			
			/* Store an invalid visualization element: */
			element.first=0;
			element.second=requestID;
			
			/* Push this visualization element to the main thread: */
			trackedElements.postNewValue();
			update();
			
			/* Delete the unused extraction parameters: */
			delete parameters;
			}
		}
	
	return 0;
	}

void* Extractor::slaveExtractorThreadMethod(void)
	{
	/* Enable asynchronous cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Create a data source for the multicast pipe: */
	Visualization::Abstract::BinaryParametersSource source(extractor->getVariableManager(),*extractor->getPipe(),true);
	
	/* Receive visualization elements from master until interrupted: */
	while(true)
		{
		/* Wait for a new visualization element: */
		#if !THREADS_CONFIG_CAN_CANCEL
		if(terminate)
			return 0;
		#endif
		unsigned int requestID=extractor->getPipe()->read<unsigned int>();
		#if !THREADS_CONFIG_CAN_CANCEL
		if(terminate)
			return 0;
		#endif
		
		/* Start a new visualization element: */
		std::pair<ElementPointer,unsigned int>& element=trackedElements.startNewValue();
		if(requestID!=0)
			{
			/* Receive the new element's parameters from the master: */
			Parameters* parameters=extractor->cloneParameters();
			parameters->read(source);
			
			/* Start receiving the visualization element from the master: */
			element.first=extractor->startSlaveElement(parameters);
			element.second=requestID;
			
			/* Receive fragments of the visualization element until finished: */
			do
				{
				extractor->continueSlaveElement();
				
				/* Push this visualization element to the main thread: */
				trackedElements.postNewValue();
				update();
				}
			while(extractor->getPipe()->read<unsigned int>()!=0);
			}
		else
			{
			/* Get the request ID from the master: */
			unsigned int requestID=extractor->getPipe()->read<unsigned int>();
			
			/* Store an invalid visualization element: */
			element.first=0;
			element.second=requestID;
			
			/* Push this visualization element to the main thread: */
			trackedElements.postNewValue();
			update();
			}
		}
	
	return 0;
	}

Extractor::Extractor(Extractor::Algorithm* sExtractor)
	:extractor(sExtractor),
	 #if !THREADS_CONFIG_CAN_CANCEL
	 terminate(false),
	 #endif
	 finalElementPending(false),finalSeedRequestID(0),
	 seedParameters(0),
	 seedRequestID(0)
	{
	/* Initialize the extraction thread communications: */
	for(int i=0;i<3;++i)
		{
		trackedElements.getBuffer(i).first=0;
		trackedElements.getBuffer(i).second=0;
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
	#if !THREADS_CONFIG_CAN_CANCEL
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
	if(trackedElements.hasNewValue())
		{
		/* Delete the currently locked visualization element: */
		trackedElements.getLockedValue().first=0;
		
		/* Lock the most recent visualization element: */
		trackedElements.lockNewValue();
		}
	
	/* Check if the final element from a concluded dragging operation or an immediate extraction has arrived: */
	ElementPointer result=0;
	if(finalElementPending&&trackedElements.getLockedValue().second==finalSeedRequestID)
		{
		/* Return the new element: */
		result=trackedElements.getLockedValue().first;
		trackedElements.getLockedValue().first=0;
		
		/* Reset the finalization marker: */
		finalElementPending=false;
		}
	
	return result;
	}

void Extractor::glRenderAction(GLRenderState& renderState,bool transparent) const
	{
	/* Render the tracked visualization element if its transparency matches the parameter: */
	const Element* element=trackedElements.getLockedValue().first.getPointer();
	if(element!=0&&element->usesTransparency()==transparent)
		element->glRenderAction(renderState);
	}

void Extractor::update(void)
	{
	}
