/***********************************************************************
Main program for a dedicated server to support collaborative data
exploration in spatially distributed VR environments.
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

#include "SharedVisualizationServer.h"

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <Misc/Time.h>
#include <Collaboration/CollaborationServer.h>
#include <Collaboration/AgoraServer.h>
#include <Collaboration/EmineoServer.h>
#include <Collaboration/GrapheinServer.h>

volatile bool runServerLoop=true;

void termSignalHandler(int)
	{
	runServerLoop=false;
	}

int main(int argc,char* argv[])
	{
	/* Parse the command line: */
	int listenPortId=0;
	Misc::Time tickTime(0.02); // Server update time interval in seconds
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"port")==0)
				{
				++i;
				if(i<argc)
					listenPortId=atoi(argv[i]);
				else
					std::cerr<<"CollaborationServerMain: ignored dangling -port option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"tick")==0)
				{
				++i;
				if(i<argc)
					tickTime=Misc::Time(atof(argv[i]));
				else
					std::cerr<<"CollaborationServerMain: ignored dangling -tick option"<<std::endl;
				}
			}
		}
	
	/* Ignore SIGPIPE and leave handling of pipe errors to TCP sockets: */
	struct sigaction sigPipeAction;
	sigPipeAction.sa_handler=SIG_IGN;
	sigemptyset(&sigPipeAction.sa_mask);
	sigPipeAction.sa_flags=0x0;
	sigaction(SIGPIPE,&sigPipeAction,0);
	
	/* Create the collaboration server object: */
	Collaboration::CollaborationServer server(listenPortId);
	std::cout<<"SharedVisualizationServerMain: Started server on port "<<server.getListenPortId()<<std::endl;
	
	/* Add an Agora protocol object: */
	server.registerProtocol(new Collaboration::AgoraServer);
	
	/* Add an Emineo protocol object: */
	server.registerProtocol(new Collaboration::EmineoServer);
	
	/* Add a Graphein protocol object: */
	server.registerProtocol(new Collaboration::GrapheinServer);
	
	/* Add a shared Visualizer protocol object: */
	server.registerProtocol(new Collaboration::SharedVisualizationServer);
	
	/* Reroute SIG_INT signals to cleanly shut down multiplexer: */
	struct sigaction sigIntAction;
	sigIntAction.sa_handler=termSignalHandler;
	if(sigaction(SIGINT,&sigIntAction,0)!=0)
		std::cerr<<"SharedVisualizationServerMain: Cannot intercept SIG_INT signals. Server won't shut down cleanly."<<std::endl;
	
	/* Run the server loop at the specified time interval: */
	while(runServerLoop)
		{
		/* Sleep for the tick time: */
		Misc::sleep(tickTime);
		
		/* Update the server state: */
		server.update();
		}
	
	return 0;
	}
