/***********************************************************************
MultiStreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized multi-streamline
extractor implementation.
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

#ifndef VISUALIZATION_WRAPPERS_MULTISTREAMLINEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_MULTISTREAMLINEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/Slider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/MultiStreamline.h>

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
class MultiStreamlineExtractor;
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
class MultiStreamlineExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar DSScalar; // Scalar type of templatized data set's domain
	typedef typename DS::Point DSPoint; // Point type of templatized data set's domain
	typedef typename DS::Vector DSVector; // Vector type of templatized data set's domain
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::VE VE; // Type of templatized vector extractor
	typedef typename DataSetWrapper::VectorExtractor VectorExtractor; // Compatible vector extractor wrapper class
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::MultiStreamline<DataSetWrapper> MultiStreamline; // Type of created visualization elements
	typedef Misc::Autopointer<MultiStreamline> MultiStreamlinePointer; // Type for pointers to created visualization elements
	typedef typename MultiStreamline::Parameters Parameters; // Type for multi-streamline extraction parameters
	typedef typename MultiStreamline::MultiPolyline MultiPolyline; // Type of low-level multi-streamline representation
	typedef Visualization::Templatized::MultiStreamlineExtractor<DS,VE,SE,MultiPolyline> MSLE; // Type of templatized multi-streamline extractor
	
	/* Elements: */
	private:
	Parameters parameters; // The streamline extraction parameters used by this extractor
	MSLE msle; // The templatized multistreamline extractor
	MultiStreamlinePointer currentStreamline; // The currently extracted streamline visualization element
	
	/* UI components: */
	GLMotif::TextField* maxNumVerticesValue; // Text field to display maximum number of extracted vertices
	GLMotif::Slider* maxNumVerticesSlider; // Slider to change maximum number of extracted vertices
	GLMotif::TextField* epsilonValue; // Text field to display current accuracy threshold value
	GLMotif::Slider* epsilonSlider; // Slider to change current accuracy threshold value
	GLMotif::TextField* numStreamlinesValue; // Text field to display current number of streamlines
	GLMotif::Slider* numStreamlinesSlider; // Slider to change current number of streamlines
	GLMotif::TextField* diskRadiusValue; // Text field to display current seed disk radius
	GLMotif::Slider* diskRadiusSlider; // Slider to change current seed disk radius
	
	/* Private methods: */
	static const DS* getDs(Visualization::Abstract::VariableManager* sVariableManager,int vectorVariableIndex,int colorScalarVariableIndex);
	static const VE& getVe(const Visualization::Abstract::VectorExtractor* sVectorExtractor);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	MultiStreamlineExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates a multi-streamline extractor
	virtual ~MultiStreamlineExtractor(void);
	
	/* Methods: */
	const MSLE& getMsle(void) const // Returns the templatized multistreamline extractor
		{
		return msle;
		}
	MSLE& getMsle(void) // Ditto
		{
		return msle;
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
	void numStreamlinesSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void diskRadiusSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_MULTISTREAMLINEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/MultiStreamlineExtractor.cpp>
#endif

#endif
