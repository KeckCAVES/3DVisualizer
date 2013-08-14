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

#ifndef SHAREDVISUALIZATIONCLIENT_INCLUDED
#define SHAREDVISUALIZATIONCLIENT_INCLUDED

#include <vector>
#include <Misc/HashTable.h>
#include <Threads/Mutex.h>
#include <Collaboration/ProtocolClient.h>

#include "SharedVisualizationProtocol.h"
#include "Extractor.h"

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class Parameters;
class Algorithm;
class Element;
}
}
class GLRenderState;
class ExtractorLocator;
class Visualizer;

class SharedVisualizationClient:public Collaboration::ProtocolClient,private SharedVisualizationProtocol
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Parameters Parameters;
	typedef Visualization::Abstract::Algorithm Algorithm;
	typedef Visualization::Abstract::Element Element;
	
	private:
	class RemoteLocator:public Extractor // Class to represent locators on remote sites and to perform visualization element extraction
		{
		/* Constructors and destructors: */
		public:
		RemoteLocator(Algorithm* sExtractor); // Creates a remote locator for the given algorithm
		
		/* Methods from Extractor: */
		virtual void update(void);
		
		/* New methods: */
		void readSeedRequest(Comm::NetPipe& pipe); // Reads a seed request from the given pipe and posts it to the extractor
		};
	
	typedef Misc::HashTable<unsigned int,RemoteLocator*> RemoteLocatorHash; // Hash table to map locator IDs to remote locator objects
	
	class RemoteClientState:public ProtocolClient::RemoteClientState // Class representing client-side state of a remote client
		{
		friend class SharedVisualizationClient;
		
		/* Elements: */
		private:
		mutable Threads::Mutex locatorMutex; // Mutex protecting the locator hash table
		RemoteLocatorHash locators; // Hash table of all locators registered with the remote client
		
		/* Constructors and destructors: */
		public:
		RemoteClientState(void);
		virtual ~RemoteClientState(void);
		};
	
	struct LocatorState // Structure for states of local locators
		{
		/* Elements: */
		public:
		unsigned int locatorID; // ID of locator as sent to server
		unsigned int seedRequestID; // ID of most recent seed request posted by this locator
		Parameters* seedParameters; // Seed parameters of most recent seed request
		
		/* Constructors and destructors: */
		LocatorState(unsigned int sLocatorID)
			:locatorID(sLocatorID),seedRequestID(0),seedParameters(0)
			{
			}
		};
	
	typedef Misc::HashTable<ExtractorLocator*,LocatorState> LocatorHash; // Type of hash tables to map Visualizer locator pointers to locator states
	
	struct LocatorAction // Structure to keep track of changes to the locator state of a client
		{
		/* Elements: */
		public:
		MessageIdType action; // What kind of action, values taken from respective protocol messages
		LocatorHash::Iterator locatorIt;
		unsigned int requestID; // Request ID for seed and finalization actions
		
		/* Constructors and destructors: */
		LocatorAction(MessageIdType sAction,const LocatorHash::Iterator& sLocatorIt,unsigned int sRequestID)
			:action(sAction),locatorIt(sLocatorIt),requestID(sRequestID)
			{
			}
		};
	
	typedef std::vector<LocatorAction> LocatorActionList; // Type for lists of locator actions
	
	/* Elements: */
	Visualizer* application; // Pointer to the Visualizer application object
	unsigned int nextLocatorID; // ID to assign to the next local locator
	Threads::Mutex locatorMutex; // Mutex protecting the locator hash table
	LocatorHash locators; // Hash table mapping Visualizer's extractor locators to server locator IDs
	LocatorActionList actions; // List of locator actions queued up since the last client update
	unsigned int mostRecentSeedRequestID; // ID of most recently posted seed request
	mutable Threads::Mutex clientStatesMutex; // Mutex protecting the remote client list
	std::vector<RemoteClientState*> clientStates; // List of currently connected remote clients
	
	/* Private methods: */
	void receiveRemoteLocator(RemoteClientState* rcs,Comm::NetPipe& pipe); // Creates a new remote locator by reading from the given pipe, and adds it to the hash table
	RemoteLocator* findRemoteLocator(RemoteClientState* rcs,Comm::NetPipe& pipe); // Returns a pointer to a remote locator whose ID was read from the given pipe, or 0 if not found
	
	/* Constructors and destructors: */
	public:
	SharedVisualizationClient(Visualizer* sApplication); // Creates a shared visualization client for the given Visualizer application
	virtual ~SharedVisualizationClient(void); // Destroys the shared visualization client
	
	/* Methods from ProtocolClient: */
	virtual const char* getName(void) const;
	virtual unsigned int getNumMessages(void) const;
	virtual void sendConnectRequest(Comm::NetPipe& pipe);
	virtual ProtocolClient::RemoteClientState* receiveClientConnect(Comm::NetPipe& pipe);
	virtual bool receiveServerUpdate(Comm::NetPipe& pipe);
	virtual bool receiveServerUpdate(ProtocolClient::RemoteClientState* rcs,Comm::NetPipe& pipe);
	virtual void sendClientUpdate(Comm::NetPipe& pipe);
	virtual void rejectedByServer(void);
	virtual void connectClient(ProtocolClient::RemoteClientState* rcs);
	virtual void disconnectClient(ProtocolClient::RemoteClientState* rcs);
	virtual void frame(ProtocolClient::RemoteClientState* rcs);
	
	/* New methods: */
	void createLocator(ExtractorLocator* locator); // Registers a newly created extractor locator
	void postSeedRequest(ExtractorLocator* locator,unsigned int seedRequestID,Parameters* seedParameters); // Sends a seed request of the given ID for the given locator; client inherits parameter object
	void postFinalizationRequest(ExtractorLocator* locator,unsigned int finalSeedRequestID); // Notifies server that the given seed request ID is the final one for a current seeding operation on the given locator
	void destroyLocator(ExtractorLocator* locator); // Unregisters an extractor locator before it is destroyed
	void drawLocators(GLRenderState& renderState,bool transparent) const;
	};

#endif
