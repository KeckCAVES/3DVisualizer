/***********************************************************************
StreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized streamline extractor
implementation.
Copyright (c) 2006-2008 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_STREAMLINEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_STREAMLINEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/Slider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/Streamline.h>

/* Forward declarations: */
namespace GLMotif {
class TextField;
}
namespace Visualization {
namespace Abstract {
class VectorExtractor;
class ScalarExtractor;
class Element;
}
namespace Templatized {
template <class VectorParam,class SourceValueParam>
class VectorExtractor;
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
class StreamlineExtractor;
}
namespace Wrappers {
template <class VEParam>
class VectorExtractor;
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class StreamlineExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar DSScalar; // Scalar type of templatized data set
	typedef typename DS::Point DSPoint; // Type for points in data set's domain
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::VE VE; // Type of templatized vector extractor
	typedef typename DataSetWrapper::VectorExtractor VectorExtractor; // Compatible vector extractor wrapper class
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::Streamline<DataSetWrapper> Streamline; // Type of created visualization elements
	typedef Misc::Autopointer<Streamline> StreamlinePointer; // Type for pointers to created visualization elements
	typedef typename Streamline::Parameters Parameters; // Type for streamline extraction parameters
	typedef typename Streamline::Polyline Polyline; // Type of low-level streamline representation
	typedef Visualization::Templatized::StreamlineExtractor<DS,VE,SE,Polyline> SLE; // Type of templatized streamline extractor
	
	/* Elements: */
	private:
	Parameters parameters; // The streamline extraction parameters used by this extractor
	SLE sle; // The templatized streamline extractor
	StreamlinePointer currentStreamline; // The currently extracted streamline visualization element
	
	/* UI components: */
	GLMotif::TextField* maxNumVerticesValue; // Text field to display maximum number of extracted vertices
	GLMotif::Slider* maxNumVerticesSlider; // Slider to change maximum number of extracted vertices
	GLMotif::TextField* epsilonValue; // Text field to display current error threshold value
	GLMotif::Slider* epsilonSlider; // Slider to change current error threshold value
	
	/* Private methods: */
	static const DS* getDs(Visualization::Abstract::VariableManager* sVariableManager,int vectorVariableIndex,int colorScalarVariableIndex);
	static const VE& getVe(const Visualization::Abstract::VectorExtractor* sVectorExtractor);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	StreamlineExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates a streamline extractor
	virtual ~StreamlineExtractor(void);
	
	/* Methods: */
	const SLE& getSle(void) const // Returns the templatized streamline extractor
		{
		return sle;
		}
	SLE& getSle(void) // Ditto
		{
		return sle;
		}
	size_t getMaxNumVertices(void) const // Returns the maximum number of vertices to be extracted
		{
		return parameters.maxNumVertices;
		}
	void setMaxNumVertices(size_t newMaxNumVertices); // Sets the maximum number of vertices to be extracted
	virtual bool hasSeededCreator(void) const;
	virtual bool hasIncrementalCreator(void) const;
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual Visualization::Abstract::Element* createElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual Visualization::Abstract::Element* startElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual bool continueElement(const Realtime::AlarmTimer& alarm);
	virtual void finishElement(void);
	virtual Visualization::Abstract::Element* startSlaveElement(void);
	virtual void continueSlaveElement(void);
	void maxNumVerticesSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void epsilonSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_STREAMLINEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/StreamlineExtractor.cpp>
#endif

#endif
