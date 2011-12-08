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

#ifndef VISUALIZATION_ABSTRACT_ALGORITHM_INCLUDED
#define VISUALIZATION_ABSTRACT_ALGORITHM_INCLUDED

#include <Misc/FunctionCalls.h>

#include <Abstract/DataSet.h>

/* Forward declarations: */
namespace Realtime {
class AlarmTimer;
}
namespace Cluster {
class MulticastPipe;
}
namespace GLMotif {
class WidgetManager;
class Widget;
}
namespace Visualization {
namespace Abstract {
class VariableManager;
class Parameters;
class ParametersSource;
class Element;
}
}

namespace Visualization {

namespace Abstract {

class Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Misc::FunctionCall<float> BusyFunction; // Type for functions called during long-running operations
	
	/* Elements: */
	private:
	VariableManager* variableManager; // Pointer to the variable manager containing the source data set and variables for this algorithm
	Cluster::MulticastPipe* pipe; // Multicast pipe to synchronize element extraction in a cluster-based environment; created externally but owned by Algorithm object
	bool master; // Flag if this instance of the algorithm runs on the master node of a visualization cluster
	BusyFunction* busyFunction; // Function called at regular intervals during a long-running operation
	
	/* Constructors and destructors: */
	public:
	Algorithm(VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe); // Creates algorithm to own the given pipe
	private:
	Algorithm(const Algorithm& source); // Prohibit copy constructor
	Algorithm& operator=(const Algorithm& source); // Prohibit assignment operator
	public:
	virtual ~Algorithm(void); // Destroys the visualization algorithm
	
	/* Methods: */
	VariableManager* getVariableManager(void) const // Returns the algorithm's variable manager
		{
		return variableManager;
		}
	Cluster::MulticastPipe* getPipe(void) const // Returns the algorithm's pipe
		{
		return pipe;
		}
	bool isMaster(void) const // Returns the master flag
		{
		return master;
		}
	void setBusyFunction(BusyFunction* newBusyFunction); // Sets the busy function; object inherits function call object
	void callBusyFunction(float completionPercentage) // Calls the busy function with a new percentage value
		{
		if(busyFunction!=0)
			(*busyFunction)(completionPercentage);
		}
	virtual const char* getName(void) const =0; // Returns the algorithm's name
	virtual bool hasGlobalCreator(void) const; // Returns true if the algorithm has a global creation method
	virtual bool hasSeededCreator(void) const; // Returns true if the algorithm has a seeded creation method
	virtual bool hasIncrementalCreator(void) const; // Returns true if the algorithm has incremental creation methods
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager); // Returns a new UI widget to change internal settings of the algorithm
	virtual void readParameters(ParametersSource& source) =0; // Reads parameters from source and updates algorithm's internal state
	virtual Parameters* cloneParameters(void) const =0; // Returns a copy of the algorithm's current extraction parameters
	virtual void setSeedLocator(const DataSet::Locator* seedLocator); // Updates the algorithm's current extraction parameters according to the given seed locator
	virtual Element* createElement(Parameters* extractParameters); // Creates a complete visualization element using the current extraction settings; inherits parameter object
	virtual Element* startElement(Parameters* extractParameters); // Starts creating a visualization element using the current extraction settings; inherits parameter object
	virtual bool continueElement(const Realtime::AlarmTimer& alarm); // Continues creating the current element; returns true if element is complete
	virtual void finishElement(void); // Cleans up after an element has been created
	virtual Element* startSlaveElement(Parameters* extractParameters) =0; // Starts creating a visualization element on the slave node(s) of a cluster environment; inherits parameter object
	virtual void continueSlaveElement(void); // Receives a fragment of a visualization element on the slave node(s) of a cluster environment
	};

}

}

#endif
