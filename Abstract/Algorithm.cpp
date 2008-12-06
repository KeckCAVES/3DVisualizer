/***********************************************************************
Algorithm - Abstract base class for visualization algorithms that
extract visualization elements from data sets.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <Abstract/Algorithm.h>

namespace Visualization {

namespace Abstract {

/***************************
Methods of class Algortithm:
***************************/

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

Element* Algorithm::createElement(void)
	{
	Misc::throwStdErr("Algorithm::createElement: No global visualization element creation defined");
	return 0;
	}

Element* Algorithm::createElement(const DataSet::Locator* seedLocator)
	{
	Misc::throwStdErr("Algorithm::createElement: No seeded visualization element creation defined");
	return 0;
	}

Element* Algorithm::startElement(void)
	{
	Misc::throwStdErr("Algorithm::startElement: No global incremental visualization element creation defined");
	return 0;
	}

Element* Algorithm::startElement(const DataSet::Locator* seedLocator)
	{
	Misc::throwStdErr("Algorithm::startElement: No seeded incremental visualization element creation defined");
	return 0;
	}

bool Algorithm::continueElement(const Realtime::AlarmTimer& alarm)
	{
	Misc::throwStdErr("Algorithm::continueElement: No incremental visualization element creation defined");
	return false;
	}

void Algorithm::finishElement(void)
	{
	Misc::throwStdErr("Algorithm::finishElement: No incremental visualization element creation defined");
	}

Element* Algorithm::startSlaveElement(void)
	{
	Misc::throwStdErr("Algorithm::startSlaveElement: No cluster-optimized element creation defined");
	return 0;
	}

void Algorithm::continueSlaveElement(void)
	{
	Misc::throwStdErr("Algorithm::continueSlaveElement: No cluster-optimized element creation defined");
	}

}

}
