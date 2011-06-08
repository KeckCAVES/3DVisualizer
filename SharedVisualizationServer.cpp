/***********************************************************************
SharedVisualizationServer - Server for collaborative data exploration in
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

#include <SharedVisualizationServer.h>

#ifdef VERBOSE
#include <iostream>
#endif
#include <Misc/ThrowStdErr.h>

namespace Collaboration {

/*******************************************************
Methods of class SharedVisualizationServer::SeedRequest:
*******************************************************/

SharedVisualizationServer::SeedRequest& SharedVisualizationServer::SeedRequest::receive(CollaborationPipe& pipe)
	{
	requestID=pipe.read<unsigned int>();
	unsigned int newParametersSize=pipe.read<unsigned int>();
	if(newParametersSize!=parametersSize)
		{
		delete[] parameters;
		parametersSize=newParametersSize;
		parameters=new unsigned char[parametersSize];
		}
	pipe.read<unsigned char>(parameters,parametersSize);
	
	return *this;
	}

void SharedVisualizationServer::SeedRequest::send(CollaborationPipe& pipe) const
	{
	pipe.write<unsigned int>(requestID);
	pipe.write<unsigned int>(parametersSize);
	pipe.write<unsigned char>(parameters,parametersSize);
	}

/********************************************************
Methods of class SharedVisualizationServer::LocatorState:
********************************************************/

SharedVisualizationServer::LocatorState::LocatorState(const std::string& sAlgorithmName)
	:algorithmName(sAlgorithmName),
	 finalSeedRequestID(0)
	{
	}

/***************************************************
Methods of class SharedVisualizationServer::Element:
***************************************************/

SharedVisualizationServer::Element::Element(const SharedVisualizationServer::LocatorState& ls)
	:algorithmName(ls.algorithmName),
	 parametersSize(ls.seedRequest.parametersSize),parameters(new unsigned char[parametersSize]),
	 enabled(true)
	{
	/* Copy the seed request's parameters blob: */
	memcpy(parameters,ls.seedRequest.parameters,parametersSize);
	}

void SharedVisualizationServer::Element::send(CollaborationPipe& pipe) const
	{
	pipe.write<std::string>(algorithmName);
	pipe.write<unsigned int>(parametersSize);
	pipe.write<unsigned char>(parameters,parametersSize);
	pipe.write<unsigned char>(enabled?1:0);
	}

/*******************************************************
Methods of class SharedVisualizationServer::ClientState:
*******************************************************/

SharedVisualizationServer::ClientState::ClientState(void)
	:firstUpdate(true),
	 locators(17)
	{
	}

SharedVisualizationServer::ClientState::~ClientState(void)
	{
	/* Delete all locators from the client's hash table: */
	for(LocatorHash::Iterator lIt=locators.begin();!lIt.isFinished();++lIt)
		delete lIt->getDest();
	}

/******************************************
Methods of class SharedVisualizationServer:
******************************************/

SharedVisualizationServer::SharedVisualizationServer(void)
	:nextElementID(0),elements(31)
	{
	}

SharedVisualizationServer::~SharedVisualizationServer(void)
	{
	/* Delete all elements: */
	Threads::Mutex::Lock elementListLock(elementListMutex);
	for(ElementHash::Iterator eIt=elements.begin();!eIt.isFinished();++eIt)
		delete eIt->getDest();
	}

const char* SharedVisualizationServer::getName(void) const
	{
	return protocolName;
	}

unsigned int SharedVisualizationServer::getNumMessages(void) const
	{
	return MESSAGES_END;
	}

ProtocolServer::ClientState* SharedVisualizationServer::receiveConnectRequest(unsigned int protocolMessageLength,CollaborationPipe& pipe)
	{
	/* Receive the client's protocol version: */
	unsigned int clientProtocolVersion=pipe.read<unsigned int>();
	
	/* Check for the correct version number: */
	if(clientProtocolVersion==protocolVersion)
		return new ClientState;
	else
		return 0;
	}

void SharedVisualizationServer::receiveClientUpdate(ProtocolServer::ClientState* cs,CollaborationPipe& pipe)
	{
	ClientState* myCs=dynamic_cast<ClientState*>(cs);
	if(myCs==0)
		Misc::throwStdErr("SharedVisualizationServer::receiveClientUpdate: Mismatching client state object type");
	
	/* Receive a list of locator action messages from the client: */
	CollaborationPipe::MessageIdType message;
	while((message=pipe.readMessage())!=UPDATE_END)
		switch(message)
			{
			case CREATE_LOCATOR:
				{
				/* Read the new locator's ID and algorithm name: */
				unsigned int locatorID=pipe.read<unsigned int>();
				std::string algorithmName=pipe.read<std::string>();
				
				/* Add a new locator to the list: */
				LocatorState* newLocator=new LocatorState(algorithmName);
				myCs->locators.setEntry(LocatorHash::Entry(locatorID,newLocator));
				
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationServer: Creating "<<algorithmName<<" locator with ID "<<locatorID<<std::endl;
				#endif
				
				/* Enqueue a locator action: */
				myCs->actions.push_back(LocatorAction(CREATE_LOCATOR,myCs->locators.findEntry(locatorID),0));
				
				break;
				}
			
			case SEED_REQUEST:
				{
				/* Read the locator's ID: */
				unsigned int locatorID=pipe.read<unsigned int>();
				
				/* Find the locator's state: */
				LocatorHash::Iterator locatorIt=myCs->locators.findEntry(locatorID);
				
				if(locatorIt.isFinished())
					{
					/* We could just silently ignore the request, but it's safer to bail out with a protocol error: */
					Misc::throwStdErr("SharedVisualizationServer::handleMessage: Locator ID %u not found",locatorID);
					}
				
				/* Store the seed request parameters: */
				locatorIt->getDest()->seedRequest.receive(pipe);
				
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationServer: Received seed request "<<locatorIt->getDest()->seedRequest.requestID<<" from locator "<<locatorID<<std::endl;
				#endif
				
				/* Enqueue a locator action: */
				myCs->actions.push_back(LocatorAction(SEED_REQUEST,locatorIt,locatorIt->getDest()->seedRequest.requestID));
				
				break;
				}
			
			case FINALIZATION_REQUEST:
				{
				/* Read the locator's ID: */
				unsigned int locatorID=pipe.read<unsigned int>();
				
				/* Find the locator's state: */
				LocatorHash::Iterator locatorIt=myCs->locators.findEntry(locatorID);
				
				if(locatorIt.isFinished())
					{
					/* We could just silently ignore the request, but it's safer to bail out with a protocol error: */
					Misc::throwStdErr("SharedVisualizationServer::handleMessage: Locator ID %u not found",locatorID);
					}
				
				/* Store the final seed request ID: */
				locatorIt->getDest()->finalSeedRequestID=pipe.read<unsigned int>();
				
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationServer: Received finalization request "<<locatorIt->getDest()->finalSeedRequestID<<" from locator "<<locatorID<<std::endl;
				#endif
				
				/* Enqueue a locator action: */
				myCs->actions.push_back(LocatorAction(FINALIZATION_REQUEST,locatorIt,locatorIt->getDest()->finalSeedRequestID));
				
				break;
				}
			
			case DESTROY_LOCATOR:
				{
				/* Read the locator's ID: */
				unsigned int locatorID=pipe.read<unsigned int>();
				
				/* Find the locator's state: */
				LocatorHash::Iterator locatorIt=myCs->locators.findEntry(locatorID);
				
				if(locatorIt.isFinished())
					{
					/* We could just silently ignore the request, but it's safer to bail out with a protocol error: */
					Misc::throwStdErr("SharedVisualizationServer::handleMessage: Locator ID %u not found",locatorID);
					}
				
				#ifdef VERBOSE
				std::cout<<"SharedVisualizationServer: Destroying locator "<<locatorID<<std::endl;
				#endif
				
				/* Enqueue a locator action: */
				myCs->actions.push_back(LocatorAction(DESTROY_LOCATOR,locatorIt,0));
				
				break;
				}
			
			default:
				Misc::throwStdErr("SharedVisualizationServer::receiveClientUpdate: received unknown locator action message %u",message);
			}
	}

void SharedVisualizationServer::sendClientConnect(ProtocolServer::ClientState* sourceCs,ProtocolServer::ClientState* destCs,CollaborationPipe& pipe)
	{
	ClientState* mySourceCs=dynamic_cast<ClientState*>(sourceCs);
	ClientState* myDestCs=dynamic_cast<ClientState*>(destCs);
	if(mySourceCs==0||myDestCs==0)
		Misc::throwStdErr("SharedVisualizationServer::sendClientConnect: Mismatching client state object type");
	
	/* Send the existing locators of the source client to the destination client: */
	unsigned int numLocators=mySourceCs->locators.getNumEntries();
	pipe.write<unsigned int>(numLocators);
	for(LocatorHash::Iterator lIt=mySourceCs->locators.begin();!lIt.isFinished();++lIt)
		{
		/* Send the locator's ID and algorithm name: */
		pipe.write<unsigned int>(lIt->getSource());
		pipe.write<std::string>(lIt->getDest()->algorithmName);
		}
	}

void SharedVisualizationServer::sendServerUpdate(ProtocolServer::ClientState* destCs,CollaborationPipe& pipe)
	{
	ClientState* myDestCs=dynamic_cast<ClientState*>(destCs);
	if(myDestCs==0)
		Misc::throwStdErr("SharedVisualizationServer::sendServerUpdate: Mismatching client state object type");
	
	if(myDestCs->firstUpdate)
		{
		/* Send all existing visualization elements to the newly-connected client: */
		Threads::Mutex::Lock elementListLock(elementListMutex);
		for(ElementHash::Iterator eIt=elements.begin();!eIt.isFinished();++eIt)
			{
			pipe.writeMessage(CREATE_ELEMENT);
			pipe.write<unsigned int>(eIt->getSource());
			eIt->getDest()->send(pipe);
			}
		
		myDestCs->firstUpdate=false;
		}
	
	/* Terminate the per-server action list: */
	pipe.writeMessage(UPDATE_END);
	}

void SharedVisualizationServer::sendServerUpdate(ProtocolServer::ClientState* sourceCs,ProtocolServer::ClientState* destCs,CollaborationPipe& pipe)
	{
	ClientState* mySourceCs=dynamic_cast<ClientState*>(sourceCs);
	ClientState* myDestCs=dynamic_cast<ClientState*>(destCs);
	if(mySourceCs==0||myDestCs==0)
		Misc::throwStdErr("SharedVisualizationServer::sendServerUpdate: Mismatching client state object type");
	
	/* Send the source client's locator action list to the destination client: */
	for(LocatorActionList::const_iterator aIt=mySourceCs->actions.begin();aIt!=mySourceCs->actions.end();++aIt)
		{
		switch(aIt->action)
			{
			case CREATE_LOCATOR:
				/* Send a creation message: */
				pipe.writeMessage(CREATE_LOCATOR);
				
				/* Send the new locator's ID and algorithm name: */
				pipe.write<unsigned int>(aIt->locatorIt->getSource());
				pipe.write<std::string>(aIt->locatorIt->getDest()->algorithmName);
				
				break;
			
			case SEED_REQUEST:
				/* Only send a message if the action's request ID matches what's still in the locator (i.e., if this was the most recent request): */
				if(aIt->requestID==aIt->locatorIt->getDest()->seedRequest.requestID)
					{
					/* Send a seed request message: */
					pipe.writeMessage(SEED_REQUEST);
					
					/* Send the locator's ID and seed parameters: */
					pipe.write<unsigned int>(aIt->locatorIt->getSource());
					aIt->locatorIt->getDest()->seedRequest.send(pipe);
					}
				
				break;
			
			case FINALIZATION_REQUEST:
				/* Only send a message if the action's request ID matches what's still in the locator (i.e., if this was the most recent request): */
				if(aIt->requestID==aIt->locatorIt->getDest()->finalSeedRequestID)
					{
					/* Send a finalization request message: */
					pipe.writeMessage(FINALIZATION_REQUEST);
					
					/* Send the locator's ID and final seed request ID: */
					pipe.write<unsigned int>(aIt->locatorIt->getSource());
					pipe.write<unsigned int>(aIt->locatorIt->getDest()->finalSeedRequestID);
					}
				
				break;
			
			case DESTROY_LOCATOR:
				/* Send a destruction message: */
				pipe.writeMessage(DESTROY_LOCATOR);
				
				/* Send the locator's ID: */
				pipe.write<unsigned int>(aIt->locatorIt->getSource());
				
				break;
			
			default:
				; // Just to make g++ happy
			}
		}
	
	/* Terminate the action list: */
	pipe.writeMessage(UPDATE_END);
	}

void SharedVisualizationServer::afterServerUpdate(ProtocolServer::ClientState* cs)
	{
	ClientState* myCs=dynamic_cast<ClientState*>(cs);
	if(myCs==0)
		Misc::throwStdErr("SharedVisualizationServer::afterServerUpdate: Mismatching client state object type");
	
	/* Destroy all locators with a pending destruction action: */
	for(LocatorActionList::const_iterator aIt=myCs->actions.begin();aIt!=myCs->actions.end();++aIt)
		if(aIt->action==DESTROY_LOCATOR)
			{
			/* Delete the locator state object and remove it from the hash table: */
			delete aIt->locatorIt->getDest();
			myCs->locators.removeEntry(aIt->locatorIt);
			}
	
	/* Clear the action list: */
	myCs->actions.clear();
	}

}

/****************
DSO entry points:
****************/

extern "C" {

Collaboration::ProtocolServer* createObject(Collaboration::ProtocolServerLoader& objectLoader)
	{
	return new Collaboration::SharedVisualizationServer;
	}

void destroyObject(Collaboration::ProtocolServer* object)
	{
	delete object;
	}

}
