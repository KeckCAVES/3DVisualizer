/***********************************************************************
SharedVisualizationClient - Client for collaborative data exploration in
spatially distributed VR environments, implemented as a plug-in of the
Vrui remote collaboration infrastructure.
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

#include "SharedVisualizationClient.h"

#include <string>
#include <iostream>
#include <Comm/NetPipe.h>
#include <Cluster/MulticastPipe.h>
#include <Vrui/Vrui.h>

#include <Abstract/VariableManager.h>
#include <Abstract/Parameters.h>
#include <Abstract/BinaryParametersSize.h>
#include <Abstract/BinaryParametersSink.h>
#include <Abstract/BinaryParametersSource.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>
#include <Abstract/Module.h>

#include "GLRenderState.h"
#include "Visualizer.h"
#include "ExtractorLocator.h"
#include "ElementList.h"

/*********************************************************
Methods of class SharedVisualizationClient::RemoteLocator:
*********************************************************/

SharedVisualizationClient::RemoteLocator::RemoteLocator(SharedVisualizationClient::Algorithm* sExtractor)
	:Extractor(sExtractor)
	{
	}

void SharedVisualizationClient::RemoteLocator::readSeedRequest(Comm::NetPipe& pipe)
	{
	/* Read the seed request ID: */
	unsigned int newSeedRequestID=pipe.read<Card>();
	#ifdef VERBOSE
	std::cout<<"SharedVisualizationClient: Received seed request "<<newSeedRequestID<<std::endl;
	#endif
	
	/* Read extraction parameters from pipe: */
	Visualization::Abstract::BinaryParametersSource source(extractor->getVariableManager(),pipe,false);
	Parameters* newSeedParameters=extractor->cloneParameters();
	pipe.read<Card>(); // Read and ignore parameter blob size
	newSeedParameters->read(source);
	
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

void SharedVisualizationClient::receiveRemoteLocator(SharedVisualizationClient::RemoteClientState* rcs,Comm::NetPipe& pipe)
	{
	/* Receive the new locator's ID and algorithm name: */
	unsigned int newLocatorID=pipe.read<Card>();
	std::string algorithmName=read<std::string>(pipe);
	
	#ifdef VERBOSE
	std::cout<<"SharedVisualizationClient: Creating "<<algorithmName<<" locator with ID "<<newLocatorID<<std::endl;
	#endif
	
	/* Create an extractor for the given algorithm name: */
	Cluster::MulticastPipe* algorithmPipe=Vrui::openPipe();
	Algorithm* algorithm=application->module->getAlgorithm(algorithmName.c_str(),application->variableManager,algorithmPipe);
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
		delete algorithmPipe;
		}
	}

SharedVisualizationClient::RemoteLocator* SharedVisualizationClient::findRemoteLocator(SharedVisualizationClient::RemoteClientState* rcs,Comm::NetPipe& pipe)
	{
	/* Read the locator ID: */
	unsigned int locatorID=pipe.read<Card>();
	
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

void SharedVisualizationClient::sendConnectRequest(Comm::NetPipe& pipe)
	{
	/* Send the length of the following message: */
	pipe.write<Card>(sizeof(Card));
	
	/* Send the client's protocol version: */
	pipe.write<Card>(protocolVersion);
	}

Collaboration::ProtocolClient::RemoteClientState* SharedVisualizationClient::receiveClientConnect(Comm::NetPipe& pipe)
	{
	/* Create a new remote client state object: */
	RemoteClientState* newClient=new RemoteClientState;
	
	/* Receive the number of locators on the remote client: */
	unsigned int numLocators=pipe.read<Card>();
	
	/* Create remote locators: */
	for(unsigned int i=0;i<numLocators;++i)
		receiveRemoteLocator(newClient,pipe);
	
	return newClient;
	}

bool SharedVisualizationClient::receiveServerUpdate(Comm::NetPipe& pipe)
	{
	/* Ignore a list of global server messages: */
	MessageIdType message;
	while((message=readMessage(pipe))!=UPDATE_END)
		switch(message)
			{
			case CREATE_ELEMENT:
				{
				/* Skip an element creation message: */
				unsigned int elementId=pipe.read<Card>();
				std::string algorithmName=read<std::string>(pipe);
				size_t parametersSize=pipe.read<Card>();
				Byte* parameters=new Byte[parametersSize];
				pipe.read(parameters,parametersSize);
				pipe.read<Byte>(); // bool enabled=pipe.read<unsigned char>()!=0;
				
				#ifdef VERBOSE
				std::cout<<"Ignored creation of "<<algorithmName<<" element with ID "<<elementId<<std::endl;
				#endif
				}
			}
	
	return false;
	}

bool SharedVisualizationClient::receiveServerUpdate(Collaboration::ProtocolClient::RemoteClientState* rcs,Comm::NetPipe& pipe)
	{
	RemoteClientState* myRcs=dynamic_cast<RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::receiveServerUpdate: Mismatching remote client state object type");
	
	/* Receive a list of locator action messages from the server: */
	MessageIdType message;
	while((message=readMessage(pipe))!=UPDATE_END)
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
					pipe.skip<Card>(1);
					unsigned int parametersSize=pipe.read<Card>();
					pipe.skip<Byte>(parametersSize);
					}
				
				break;
				}
			
			case FINALIZATION_REQUEST:
				{
				/* Find the remote locator: */
				RemoteLocator* locator=findRemoteLocator(myRcs,pipe);
				
				/* Read the final seed request ID: */
				unsigned int finalRequestID=pipe.read<Card>();
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationServer: Received finalization request "<<finalRequestID<<std::endl;
				#endif
				
				if(locator!=0)
					{
					/* Post the finalization request: */
					locator->finalize(finalRequestID);
					}
				
				break;
				}
			
			case DESTROY_LOCATOR:
				{
				/* Read the locator ID: */
				unsigned int locatorID=pipe.read<Card>();
				
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
	
	return false;
	}

void SharedVisualizationClient::sendClientUpdate(Comm::NetPipe& pipe)
	{
	Threads::Mutex::Lock locatorLock(locatorMutex);
	
	/* Send the locator action list to the server: */
	for(LocatorActionList::iterator aIt=actions.begin();aIt!=actions.end();++aIt)
		{
		switch(aIt->action)
			{
			case CREATE_LOCATOR:
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationClient: Creating "<<aIt->locatorIt->getSource()->getExtractor()->getName()<<" locator with ID "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				/* Send a creation message: */
				writeMessage(CREATE_LOCATOR,pipe);
				
				/* Send the new locator's ID and algorithm name: */
				pipe.write<Card>(aIt->locatorIt->getDest().locatorID);
				write(std::string(aIt->locatorIt->getSource()->getExtractor()->getName()),pipe);
				
				break;
			
			case SEED_REQUEST:
				/* Only send a message if the action's request ID matches what's still in the locator (i.e., if this was the most recent request): */
				if(aIt->requestID==aIt->locatorIt->getDest().seedRequestID)
					{
					#ifdef VERBOSE
					std::cout<<"SharedVisualizationClient: Sending seed request "<<aIt->requestID<<" for locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
					#endif
					
					/* Send a seed request message: */
					writeMessage(SEED_REQUEST,pipe);
					
					/* Send the locator's ID and seed request ID: */
					pipe.write<Card>(aIt->locatorIt->getDest().locatorID);
					pipe.write<Card>(aIt->requestID);
					
					/* Calculate and send the seed request's extraction parameters message size: */
					Visualization::Abstract::BinaryParametersSize size(application->variableManager,false);
					aIt->locatorIt->getDest().seedParameters->write(size);
					pipe.write<Card>(size.getSize());
					
					/* Send the seed request's extraction parameters: */
					Visualization::Abstract::BinaryParametersSink sink(application->variableManager,pipe,false);
					aIt->locatorIt->getDest().seedParameters->write(sink);
					
					/* Delete the seed parameters: */
					delete aIt->locatorIt->getDest().seedParameters;
					aIt->locatorIt->getDest().seedParameters=0;
					}
				
				break;
			
			case FINALIZATION_REQUEST:
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationClient: Sending finalization request "<<aIt->requestID<<" for locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				/* Send a finalization request message: */
				writeMessage(FINALIZATION_REQUEST,pipe);
				
				/* Send the locator's ID and final seed request ID: */
				pipe.write<Card>(aIt->locatorIt->getDest().locatorID);
				pipe.write<Card>(aIt->requestID);
					
				break;
			
			case DESTROY_LOCATOR:
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationClient: Destroying locator "<<aIt->locatorIt->getDest().locatorID<<std::endl;
				#endif
				
				/* Send a destruction message: */
				writeMessage(DESTROY_LOCATOR,pipe);
				
				/* Send the locator's ID: */
				pipe.write<Card>(aIt->locatorIt->getDest().locatorID);
				
				/* Remove the locator's state from the hash table: */
				locators.removeEntry(aIt->locatorIt);
					
				break;
			
			default:
				; // Just to make g++ happy
			}
		}
	
	/* Terminate the action list: */
	writeMessage(UPDATE_END,pipe);
	
	/* Clear the action list: */
	actions.clear();
	}

void SharedVisualizationClient::rejectedByServer(void)
	{
	/* Write a warning message: */
	std::cout<<"SharedVisualizationClient: Server does not support shared Visualizer protocol. Bummer."<<std::endl;
	}

void SharedVisualizationClient::connectClient(Collaboration::ProtocolClient::RemoteClientState* rcs)
	{
	RemoteClientState* myRcs=dynamic_cast<RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::connectClient: Mismatching remote client state object type");
	
	/* Add the new remote client object to the list: */
	Threads::Mutex::Lock clientStatesLock(clientStatesMutex);
	clientStates.push_back(myRcs);
	}

void SharedVisualizationClient::disconnectClient(Collaboration::ProtocolClient::RemoteClientState* rcs)
	{
	RemoteClientState* myRcs=dynamic_cast<RemoteClientState*>(rcs);
	if(myRcs==0)
		Misc::throwStdErr("SharedVisualizationClient::disconnectClient: Mismatching remote client state object type");
	
	/* Remove the remote client object from the list: */
	Threads::Mutex::Lock clientStatesLock(clientStatesMutex);
	for(std::vector<RemoteClientState*>::iterator csIt=clientStates.begin();csIt!=clientStates.end();++csIt)
		if(*csIt==rcs)
			{
			clientStates.erase(csIt);
			break;
			}
	}

void SharedVisualizationClient::frame(Collaboration::ProtocolClient::RemoteClientState* rcs)
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

void SharedVisualizationClient::drawLocators(GLRenderState& renderState,bool transparent) const
	{
	Threads::Mutex::Lock clientStatesLock(clientStatesMutex);
	for(std::vector<RemoteClientState*>::const_iterator csIt=clientStates.begin();csIt!=clientStates.end();++csIt)
		{
		/* Display the client's remote locators: */
		{
		Threads::Mutex::Lock locatorLock((*csIt)->locatorMutex);
		for(RemoteLocatorHash::Iterator lIt=(*csIt)->locators.begin();!lIt.isFinished();++lIt)
			lIt->getDest()->glRenderAction(renderState,transparent);
		}
		}
	}
