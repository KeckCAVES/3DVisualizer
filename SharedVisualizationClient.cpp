/***********************************************************************
SharedVisualizationClient - Client for collaborative data exploration in
spatially distributed VR environments, implemented as a plug-in of the
Vrui remote collaboration infrastructure.
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

#define VERBOSE 0

#include "SharedVisualizationClient.h"

#include <string>
#include <iostream>
#include <Vrui/Vrui.h>
#include <Collaboration/CollaborationPipe.h>

#include <Abstract/VariableManager.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>
#include <Abstract/Module.h>

#include "Visualizer.h"
#include "ExtractorLocator.h"
#include "ElementList.h"

namespace Collaboration {

/*********************************************************
Methods of class SharedVisualizationClient::RemoteLocator:
*********************************************************/

SharedVisualizationClient::RemoteLocator::RemoteLocator(SharedVisualizationClient::Algorithm* sExtractor)
	:Extractor(sExtractor)
	{
	}

void SharedVisualizationClient::RemoteLocator::readSeedRequest(CollaborationPipe& pipe)
	{
	/* Read the seed request ID: */
	unsigned int newSeedRequestID=pipe.read<unsigned int>();
	
	/* Read extraction parameters from pipe: */
	Parameters* newSeedParameters=extractor->cloneParameters();
	newSeedParameters->read(pipe,extractor->getVariableManager());
	
	/* Post a seed request: */
	if(extractor->isMaster())
		seedRequest(newSeedRequestID,newSeedParameters);
	}

void SharedVisualizationClient::RemoteLocator::update(void)
	{
	/* Wake up the main application: */
	Vrui::requestUpdate();
	}

/*************************************************************
Methods of class SharedVisualizationClient::RemoteClientState:
*************************************************************/

SharedVisualizationClient::RemoteClientState::RemoteClientState(void)
	:locators(17)
	{
	}

SharedVisualizationClient::RemoteClientState::~RemoteClientState(void)
	{
	/* Delete all remote locators: */
	for(RemoteLocatorHash::Iterator lIt=locators.begin();!lIt.isFinished();++lIt)
		delete lIt->getDest();
	}

/******************************************
Methods of class SharedVisualizationClient:
******************************************/

void SharedVisualizationClient::receiveRemoteLocator(SharedVisualizationClient::RemoteClientState* rcs,CollaborationPipe& pipe)
	{
	/* Receive the new locator's ID and algorithm name: */
	unsigned int newLocatorID=pipe.read<unsigned int>();
	std::string algorithmName=pipe.read<std::string>();
	
	/* Create an extractor for the given algorithm name: */
	Algorithm* algorithm=0;
	for(int i=0;algorithm==0&&i<application->module->getNumScalarAlgorithms();++i)
		if(algorithmName==application->module->getScalarAlgorithmName(i))
			algorithm=application->module->getScalarAlgorithm(i,application->variableManager,Vrui::openPipe());
	for(int i=0;algorithm==0&&i<application->module->getNumVectorAlgorithms();++i)
		if(algorithmName==application->module->getVectorAlgorithmName(i))
			algorithm=application->module->getVectorAlgorithm(i,application->variableManager,Vrui::openPipe());
	
	if(algorithm!=0)
		{
		/* Create a new remote locator and add it to the client's hash table: */
		RemoteLocator* newRemoteLocator=new RemoteLocator(algorithm);
		{
		Threads::Mutex::Lock locatorLock(rcs->locatorMutex);
		rcs->locators.setEntry(RemoteLocatorHash::Entry(newLocatorID,newRemoteLocator));
		}
		}
	else
		{
		/* Print a warning message, but carry on otherwise: */
		std::cout<<"SharedVisualizationClient::receiveRemoteLocator: Remote client requested locator of unknown type "<<algorithmName<<std::endl;
		}
	}

SharedVisualizationClient::RemoteLocator* SharedVisualizationClient::findRemoteLocator(SharedVisualizationClient::RemoteClientState* rcs,CollaborationPipe& pipe)
	{
	/* Read the locator ID: */
	unsigned int locatorID=pipe.read<unsigned int>();
	
	{
	Threads::Mutex::Lock locatorLock(rcs->locatorMutex);
	RemoteLocatorHash::Iterator lIt=rcs->locators.findEntry(locatorID);
	if(lIt.isFinished())
		return 0;
	else
		return lIt->getDest();
	}
	}

SharedVisualizationClient::SharedVisualizationClient(Visualizer* sApplication)
	:application(sApplication),
	 nextLocatorID(0),
	 locators(17)
	{
	}

SharedVisualizationClient::~SharedVisualizationClient(void)
	{
	}

const char* SharedVisualizationClient::getName(void) const
	{
	return protocolName;
	}

unsigned int SharedVisualizationClient::getNumMessages(void) const
	{
	return MESSAGES_END;
	}

void SharedVisualizationClient::sendConnectRequest(CollaborationPipe& pipe)
	{
	/* Send the length of the following message: */
	pipe.write<unsigned int>(sizeof(unsigned int));
	
	/* Send the client's protocol version: */
	pipe.write<unsigned int>(protocolVersion);
	}

void SharedVisualizationClient::sendClientUpdate(CollaborationPipe& pipe)
	{
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Send the locator action list to the server: */
	for(LocatorActionList::iterator aIt=actions.begin();aIt!=actions.end();++aIt)
		{
		switch(aIt->action)
			{
			case CREATE_LOCATOR:
				/* Send a creation message: */
				pipe.writeMessage(CREATE_LOCATOR);
				
				/* Send the new locator's ID and algorithm name: */
				pipe.write<unsigned int>(aIt->locatorIt->getDest().locatorID);
				pipe.write<std::string>(aIt->locatorIt->getSource()->getExtractor()->getName());
				
				#if VERBOSE
				std::cout<<"SharedVisualizationClient: Creating "<<aIt->locatorIt->getSource()->getExtractor()->getName()<<" locator with ID "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				break;
			
			case SEED_REQUEST:
				/* Only send a message if the action's request ID matches what's still in the locator (i.e., if this was the most recent request): */
				if(aIt->requestID==aIt->locatorIt->getDest().seedRequestID)
					{
					/* Send a seed request message: */
					pipe.writeMessage(SEED_REQUEST);
					
					/* Send the locator's ID and seed request ID: */
					pipe.write<unsigned int>(aIt->locatorIt->getDest().locatorID);
					pipe.write<unsigned int>(aIt->requestID);
					
					/* Send the seed request's extraction parameters (they contain the packet size): */
					aIt->locatorIt->getDest().seedParameters->write(pipe,application->variableManager);
					
					/* Delete the seed parameters: */
					delete aIt->locatorIt->getDest().seedParameters;
					aIt->locatorIt->getDest().seedParameters=0;
					
					#if VERBOSE
					std::cout<<"SharedVisualizationClient: Sending seed request "<<aIt->requestID<<" for locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
					#endif
					}
				
				break;
			
			case FINALIZATION_REQUEST:
				/* Send a finalization request message: */
				pipe.writeMessage(FINALIZATION_REQUEST);
				
				/* Send the locator's ID and final seed request ID: */
				pipe.write<unsigned int>(aIt->locatorIt->getDest().locatorID);
				pipe.write<unsigned int>(aIt->requestID);
					
				#if VERBOSE
				std::cout<<"SharedVisualizationClient: Sending finalization request "<<aIt->requestID<<" for locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				break;
			
			case DESTROY_LOCATOR:
				/* Send a destruction message: */
				pipe.writeMessage(DESTROY_LOCATOR);
				
				/* Send the locator's ID: */
				pipe.write<unsigned int>(aIt->locatorIt->getDest().locatorID);
				
				#if VERBOSE
				std::cout<<"SharedVisualizationClient: Destroying locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				/* Remove the locator's state from the hash table: */
				locators.removeEntry(aIt->locatorIt);
					
				break;
			
			default:
				; // Just to make g++ happy
			}
		}
	
	/* Terminate the action list: */
	pipe.writeMessage(UPDATE_END);
	
	/* Clear the action list: */
	actions.clear();
	}
	}

ProtocolClient::RemoteClientState* SharedVisualizationClient::receiveClientConnect(CollaborationPipe& pipe)
	{
	/* Create a new remote client state object: */
	RemoteClientState* newClient=new RemoteClientState;
	
	/* Receive the number of locators on the remote client: */
	unsigned int numLocators=pipe.read<unsigned int>();
	
	/* Create remote locators: */
	for(unsigned int i=0;i<numLocators;++i)
		receiveRemoteLocator(newClient,pipe);
	
	return newClient;
	}

void SharedVisualizationClient::receiveServerUpdate(ProtocolClient::RemoteClientState* rcs,CollaborationPipe& pipe)
	{
	RemoteClientState* myRcs=dynamic_cast<RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::receiveServerUpdate: Mismatching remote client state object type");
	
	/* Receive a list of locator action messages from the server: */
	CollaborationPipe::MessageIdType message;
	while((message=pipe.readMessage())!=UPDATE_END)
		switch(message)
			{
			case CREATE_LOCATOR:
				{
				/* Receive and create a new remote locator: */
				receiveRemoteLocator(myRcs,pipe);
				
				break;
				}
			
			case SEED_REQUEST:
				{
				/* Find the remote locator: */
				RemoteLocator* locator=findRemoteLocator(myRcs,pipe);
				if(locator!=0)
					{
					/* Read and post the seed request: */
					locator->readSeedRequest(pipe);
					}
				else
					{
					/* Read and ignore the seed request: */
					pipe.read<unsigned int>();
					unsigned int parametersSize=pipe.read<unsigned int>();
					unsigned char buffer[256];
					while(parametersSize>0)
						{
						unsigned int readSize=parametersSize;
						if(readSize>256)
							readSize=256;
						pipe.read<unsigned char>(buffer,readSize);
						parametersSize-=readSize;
						}
					}
				
				break;
				}
			
			case FINALIZATION_REQUEST:
				{
				/* Find the remote locator: */
				RemoteLocator* locator=findRemoteLocator(myRcs,pipe);
				if(locator!=0)
					{
					/* Post the finalization request: */
					locator->finalize(pipe.read<unsigned int>());
					}
				else
					{
					/* Read and ignore the final seed request ID: */
					pipe.read<unsigned int>();
					}
				
				break;
				}
			
			case DESTROY_LOCATOR:
				{
				/* Read the locator ID: */
				unsigned int locatorID=pipe.read<unsigned int>();
				
				/* Find the remote locator and remove it from the hash table: */
				RemoteLocator* locator=0;
				{
				Threads::Mutex::Lock locatorLock(myRcs->locatorMutex);
				RemoteLocatorHash::Iterator lIt=myRcs->locators.findEntry(locatorID);
				if(!lIt.isFinished())
					{
					locator=lIt->getDest();
					myRcs->locators.removeEntry(lIt);
					}
				}
				
				/* Destroy the locator: */
				delete locator;
				
				break;
				}
			
			default:
				Misc::throwStdErr("SharedVisualizationClient::receiveServerUpdate: received unknown locator action message %u",message);
			}
	}

void SharedVisualizationClient::rejectedByServer(void)
	{
	/* Write a warning message: */
	std::cout<<"SharedVisualizationClient: Server does not support shared Visualizer protocol. Bummer."<<std::endl;
	}

void SharedVisualizationClient::frame(ProtocolClient::RemoteClientState* rcs)
	{
	RemoteClientState* myRcs=dynamic_cast<RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::frame: Mismatching remote client state object type");
	
	/* Check all the client's remote locators for updates: */
	{
	Threads::Mutex::Lock locatorLock(myRcs->locatorMutex);
	for(RemoteLocatorHash::Iterator lIt=myRcs->locators.begin();!lIt.isFinished();++lIt)
		{
		Extractor::ElementPointer newElement=lIt->getDest()->checkUpdates();
		if(newElement!=0)
			application->elementList->addElement(newElement.getPointer(),lIt->getDest()->getExtractor()->getName());
		}
	}
	}

void SharedVisualizationClient::display(const ProtocolClient::RemoteClientState* rcs,GLContextData& contextData) const
	{
	const RemoteClientState* myRcs=dynamic_cast<const RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::display: Mismatching remote client state object type");
	
	/* Display the client's remote locators which have opaque current elements: */
	{
	Threads::Mutex::Lock locatorLock(myRcs->locatorMutex);
	for(RemoteLocatorHash::ConstIterator lIt=myRcs->locators.begin();!lIt.isFinished();++lIt)
		lIt->getDest()->draw(contextData,false);
	}
	}

void SharedVisualizationClient::createLocator(ExtractorLocator* locator)
	{
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Create a new locator state and store it in the locator hash table: */
	LocatorState newLocator(nextLocatorID);
	++nextLocatorID;
	locators.setEntry(LocatorHash::Entry(locator,newLocator));
	LocatorHash::Iterator locatorIt=locators.findEntry(locator);
	
	/* Enqueue a locator list action: */
	actions.push_back(LocatorAction(CREATE_LOCATOR,locatorIt,0));
	}
	}

void SharedVisualizationClient::postSeedRequest(ExtractorLocator* locator,unsigned int seedRequestID,SharedVisualizationClient::Parameters* seedParameters)
	{
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Find the locator's state in the hash table: */
	LocatorHash::Iterator locatorIt=locators.findEntry(locator);
	if(locatorIt.isFinished())
		Misc::throwStdErr("SharedVisualizationClient::postSeedRequest: Locator not found");
	
	/* Store the seed request: */
	locatorIt->getDest().seedRequestID=seedRequestID;
	delete locatorIt->getDest().seedParameters;
	locatorIt->getDest().seedParameters=seedParameters;
	
	/* Enqueue a locator list action: */
	actions.push_back(LocatorAction(SEED_REQUEST,locatorIt,seedRequestID));
	}
	}

void SharedVisualizationClient::postFinalizationRequest(ExtractorLocator* locator,unsigned int finalSeedRequestID)
	{
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Find the locator's ID in the hash table: */
	LocatorHash::Iterator locatorIt=locators.findEntry(locator);
	if(locatorIt.isFinished())
		Misc::throwStdErr("SharedVisualizationClient::postFinalizationRequest: Locator not found");
	
	/* Enqueue a locator list action: */
	actions.push_back(LocatorAction(FINALIZATION_REQUEST,locatorIt,finalSeedRequestID));
	}
	}

void SharedVisualizationClient::destroyLocator(ExtractorLocator* locator)
	{
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Find the locator's ID in the hash table: */
	LocatorHash::Iterator locatorIt=locators.findEntry(locator);
	if(locatorIt.isFinished())
		Misc::throwStdErr("SharedVisualizationClient::destroyLocator: Locator not found");
	
	/* Enqueue a locator list action: */
	actions.push_back(LocatorAction(DESTROY_LOCATOR,locatorIt,0));
	}
	}

}
