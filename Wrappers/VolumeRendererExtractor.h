/***********************************************************************
VolumeRendererExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized volume renderer
implementation.
Copyright (c) 2005-2008 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/VolumeRenderer.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class Element;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class VolumeRendererExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::VolumeRenderer<DataSetWrapper> VolumeRenderer; // Type of created visualization elements
	typedef Misc::Autopointer<VolumeRenderer> VolumeRendererPointer; // Type for pointers to created visualization elements
	typedef typename VolumeRenderer::Parameters Parameters; // Type for volume renderer extraction parameters
	
	/* Elements: */
	private:
	Parameters parameters; // The volume renderer extraction parameters used by this extractor
	const DS* ds; // The templatized data set
	const SE& se; // The templatized scalar extractor
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	VolumeRendererExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates a volume renderer extractor
	virtual ~VolumeRendererExtractor(void);
	
	/* Methods: */
	virtual bool hasGlobalCreator(void) const;
	virtual Visualization::Abstract::Element* createElement(void);
	virtual Visualization::Abstract::Element* startSlaveElement(void);
	virtual void continueSlaveElement(void);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_IMPLEMENTATION
#include <Wrappers/VolumeRendererExtractor.cpp>
#endif

#endif
