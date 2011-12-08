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

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <Misc/Time.h>
#include <Collaboration/CollaborationServer.h>

#include "SharedVisualizationServer.h"

volatile bool runServerLoop=true;

void termSignalHandler(int)
	{
	runServerLoop=false;
	}

int main(int argc,char* argv[])
	{
	/* Create a new configuration object: */
	Collaboration::CollaborationServer::Configuration* cfg=new Collaboration::CollaborationServer::Configuration;
	
	/* Parse the command line: */
	Misc::Time tickTime(cfg->getTickTime()); // Server update time interval in seconds
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"port")==0)
				{
				++i;
				if(i<argc)
					cfg->setListenPortId(atoi(argv[i]));
				else
					std::cerr<<"SharedVisualizationServerMain: ignored dangling -port option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"tick")==0)
				{
				++i;
				if(i<argc)
					tickTime=Misc::Time(atof(argv[i]));
				else
					std::cerr<<"SharedVisualizationServerMain: ignored dangling -tick option"<<std::endl;
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
	Collaboration::CollaborationServer server(cfg);
	std::cout<<"SharedVisualizationServerMain: Started server on port "<<server.getListenPortId()<<std::endl;
	
	/* Add a shared Visualizer protocol object: */
	server.registerProtocol(new SharedVisualizationServer);
	
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
