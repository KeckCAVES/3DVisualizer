/***********************************************************************
SharedVisualizationProtocol - Common interface between a shared
visualization server and a shared visualization client.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef SHAREDVISUALIZATIONPROTOCOL_INCLUDED
#define SHAREDVISUALIZATIONPROTOCOL_INCLUDED

#include <Collaboration/Protocol.h>

class SharedVisualizationProtocol:public Collaboration::Protocol
	{
	/* Embedded classes: */
	public:
	enum MessageId // Enumerated type for message IDs
		{
		CREATE_LOCATOR=0,
		SEED_REQUEST,
		FINALIZATION_REQUEST,
		DESTROY_LOCATOR,
		CREATE_ELEMENT,
		UPDATE_END,
		MESSAGES_END
		};
	
	/* Elements: */
	static const char* protocolName; // Network name of shared Visualizer protocol
	static const unsigned int protocolVersion; // Specific version number of protocol implementation
	};

#endif
