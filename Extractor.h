/***********************************************************************
Extractor - Helper class to drive multithreaded incremental or immediate
extraction of visualization elements from a data set.
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

#ifndef EXTRACTOR_INCLUDED
#define EXTRACTOR_INCLUDED

#include <utility>
#include <Misc/Autopointer.h>
#include <Threads/Config.h>
#include <Threads/Mutex.h>
#include <Threads/Cond.h>
#include <Threads/Thread.h>
#include <Threads/TripleBuffer.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class Parameters;
class Algorithm;
class Element;
}
}
class GLRenderState;

class Extractor
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Parameters Parameters;
	typedef Visualization::Abstract::Algorithm Algorithm;
	typedef Visualization::Abstract::Element Element;
	typedef Misc::Autopointer<Element> ElementPointer;
	
	/* Elements: */
	protected:
	
	/* Persistent state: */
	Algorithm* extractor; // Visualization element extractor
	
	/* Persistent extractor thread state: */
	private:
	#if !THREADS_CONFIG_CAN_CANCEL
	volatile bool terminate; // Flag to tell the extractor thread to shut itself down
	#endif
	Threads::Thread extractorThread; // The visualization element extractor thread
	
	/* Transient extractor state: */
	bool finalElementPending; // Flag whether the extractor is waiting for the last seed request in a dragging operation to finish
	unsigned int finalSeedRequestID; // ID of last seed request in a dragging operation
	
	/* Extractor thread communication input: */
	Threads::Mutex seedRequestMutex; // Mutex protecting the seed request state
	Threads::Cond seedRequestCond; // Condition variable for the extractor thread to block on
	Parameters* volatile seedParameters; // Extraction parameters for the most recently requested visualization element
	volatile unsigned int seedRequestID; // ID of current seed request
	
	/* Extractor thread communication output: */
	Threads::TripleBuffer<std::pair<ElementPointer,unsigned int> > trackedElements; // Triple-buffer of currently tracked visualization elements and their IDs
	
	/* Private methods: */
	private:
	void* masterExtractorThreadMethod(void); // The extractor thread method for single computers or masters in a cluster environment
	void* slaveExtractorThreadMethod(void); // The extractor thread method for slaves in a cluster environment
	
	/* Constructors and destructors: */
	public:
	Extractor(Algorithm* sExtractor); // Creates extractor for the given algorithm; inherits algorithm
	virtual ~Extractor(void); // Destroys the extractor
	
	/* Methods: */
	const Algorithm* getExtractor(void) const // Returns pointer to extractor
		{
		return extractor;
		}
	void seedRequest(unsigned int newSeedRequestID,Parameters* newSeedParameters); // Posts a new seed request to the extraction thread
	void finalize(unsigned int newFinalSeedRequestID); // Posts a finalization request for the given seed request ID
	bool isFinalizationPending(void) const // Returns true if the main thread is waiting for a new final visualization element
		{
		return finalElementPending;
		}
	virtual ElementPointer checkUpdates(void); // Method to synchronize the extraction thread's state back to the main thread; returns pointer to new finished element or 0
	void glRenderAction(GLRenderState& renderState,bool transparent) const; // Renders the extractor's current opaque or transparent geometry
	virtual void update(void); // Hook method called asynchronously when the visual state of the extractor changes
	};

#endif
