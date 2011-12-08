/***********************************************************************
Algorithm - Abstract base class for visualization algorithms that
extract visualization elements from data sets.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2011 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>
#include <Cluster/MulticastPipe.h>

#include <Abstract/Parameters.h>

#include <Abstract/Algorithm.h>

namespace Visualization {

namespace Abstract {

/***************************
Methods of class Algortithm:
***************************/

Algorithm::Algorithm(VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe)
	:variableManager(sVariableManager),pipe(sPipe),
	 master(pipe==0||pipe->isMaster()),
	 busyFunction(0)
	{
	}

Algorithm::~Algorithm(void)
	{
	/* Shut down a cluster communication pipe (doesn't do anything if there was no pipe): */
	delete pipe;
	
	/* Delete the busy function: */
	delete busyFunction;
	}

void Algorithm::setBusyFunction(Algorithm::BusyFunction* newBusyFunction)
	{
	/* Delete the previous busy function: */
	delete busyFunction;
	
	/* Set the busy function: */
	busyFunction=newBusyFunction;
	}

bool Algorithm::hasGlobalCreator(void) const
	{
	return false;
	}

bool Algorithm::hasSeededCreator(void) const
	{
	return false;
	}

bool Algorithm::hasIncrementalCreator(void) const
	{
	return false;
	}

GLMotif::Widget* Algorithm::createSettingsDialog(GLMotif::WidgetManager* widgetManager)
	{
	return 0;
	}

void Algorithm::setSeedLocator(const DataSet::Locator* seedLocator)
	{
	/* Signal an error: */
	Misc::throwStdErr("Algorithm: No seeded element creation method defined");
	}

Element* Algorithm::createElement(Parameters* extractParameters)
	{
	/* Inherit the parameters object: */
	delete extractParameters;
	
	/* Signal an error: */
	Misc::throwStdErr("Algorithm: No immediate element creation method defined");
	return 0;
	}

Element* Algorithm::startElement(Parameters* extractParameters)
	{
	/* Inherit the parameters object: */
	delete extractParameters;
	
	/* Signal an error: */
	Misc::throwStdErr("Algorithm: No incremental element creation methods defined");
	return 0;
	}

bool Algorithm::continueElement(const Realtime::AlarmTimer& alarm)
	{
	Misc::throwStdErr("Algorithm: No incremental element creation methods defined");
	return true;
	}

void Algorithm::finishElement(void)
	{
	/* Just don't do anything */
	}

void Algorithm::continueSlaveElement(void)
	{
	/* Just don't do anything */
	}

}

}
