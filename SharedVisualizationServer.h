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

#ifndef SHAREDVISUALIZATIONSERVER_INCLUDED
#define SHAREDVISUALIZATIONSERVER_INCLUDED

#include <string>
#include <vector>
#include <Misc/HashTable.h>
#include <Threads/Mutex.h>
#include <Collaboration/CollaborationPipe.h>
#include <Collaboration/ProtocolServer.h>

#include "SharedVisualizationPipe.h"

namespace Collaboration {

class SharedVisualizationServer:public ProtocolServer,public SharedVisualizationPipe
	{
	/* Embedded classes: */
	private:
	struct SeedRequest // Structure to store seed requests from locators
		{
		/* Elements: */
		public:
		unsigned int requestID; // Seed request ID
		size_t parametersSize; // Size of parameter blob in bytes
		unsigned char* parameters; // Parameter blob
		
		/* Constructors and destructors: */
		SeedRequest(void) // Creates empty seed request
			:requestID(0),
			 parametersSize(0),parameters(0)
			{
			}
		private:
		SeedRequest(const SeedRequest& source); // Prohibit copy constructor
		SeedRequest& operator=(const SeedRequest& source); // Prohibit assignment operator
		public:
		~SeedRequest(void) // Destroys a seed request
			{
			delete[] parameters;
			}
		
		/* Methods: */
		SeedRequest& receive(CollaborationPipe& pipe); // Reads seed request from pipe
		void send(CollaborationPipe& pipe) const; // Writes seed request to pipe
		};
	
	struct LocatorState // Type for states of locators
		{
		/* Persistent locator state: */
		public:
		std::string algorithmName; // Name of this locator's algorithm in client's visualization module's namespace
		
		/* Current locator state: */
		SeedRequest seedRequest; // The most recent seed request received from this locator
		unsigned int finalSeedRequestID; // ID of final seeding request in a dragging operation (or 0 if none was received)
		
		/* Constructors and destructors: */
		LocatorState(const std::string& sAlgorithmName);
		};
	
	typedef Misc::HashTable<unsigned int,LocatorState*> LocatorHash; // Type for hash tables mapping locator IDs to locator state objects
	
	struct LocatorAction // Structure to keep track of changes to the locator state of a client
		{
		/* Elements: */
		public:
		CollaborationPipe::MessageIdType action; // What kind of action, values taken from respective protocol messages
		LocatorHash::Iterator locatorIt;
		unsigned int requestID; // Request ID for seed and finalization actions
		
		/* Constructors and destructors: */
		LocatorAction(CollaborationPipe::MessageIdType sAction,const LocatorHash::Iterator& sLocatorIt,unsigned int sRequestID)
			:action(sAction),locatorIt(sLocatorIt),requestID(sRequestID)
			{
			}
		};
	
	typedef std::vector<LocatorAction> LocatorActionList; // Type for lists of locator actions
	
	class ClientState:public ProtocolServer::ClientState
		{
		friend class SharedVisualizationServer;
		
		/* Elements: */
		LocatorHash locators; // Hash table containing locators currently registered by the client
		LocatorActionList actions; // List of locator actions queued up since the last server update
		
		/* Constructors and destructors: */
		public:
		ClientState(void);
		virtual ~ClientState(void);
		};
	
	/* Constructors and destructors: */
	public:
	SharedVisualizationServer(void); // Creates a shared Visualizer server object
	virtual ~SharedVisualizationServer(void); // Destroys the shared Visualizer server object
	
	/* Methods from ProtocolServer: */
	virtual const char* getName(void) const;
	virtual unsigned int getNumMessages(void) const;
	virtual ProtocolServer::ClientState* receiveConnectRequest(unsigned int protocolMessageLength,CollaborationPipe& pipe);
	virtual void receiveClientUpdate(ProtocolServer::ClientState* cs,CollaborationPipe& pipe);
	virtual void sendClientConnect(ProtocolServer::ClientState* sourceCs,ProtocolServer::ClientState* destCs,CollaborationPipe& pipe);
	virtual void sendServerUpdate(ProtocolServer::ClientState* sourceCs,ProtocolServer::ClientState* destCs,CollaborationPipe& pipe);
	virtual void afterServerUpdate(ProtocolServer::ClientState* cs);
	};

}

#endif
