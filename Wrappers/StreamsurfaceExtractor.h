/***********************************************************************
StreamsurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized stream surface
extractor implementation.
Copyright (c) 2006-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_STREAMSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_STREAMSURFACEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/Slider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

/* Forward declarations: */
class GLColorMap;
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
template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
class StreamsurfaceExtractor;
}
namespace Wrappers {
template <class VEParam>
class VectorExtractor;
template <class SEParam>
class ScalarExtractor;
template <class DataSetWrapperParam>
class Streamsurface;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class StreamsurfaceExtractor:public Visualization::Abstract::Algorithm
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
	typedef Streamsurface<DataSetWrapper> Streamsurface; // Type of created visualization elements
	typedef Misc::Autopointer<Streamsurface> StreamsurfacePointer; // Type for pointers to created visualization elements
	typedef typename Streamsurface::Surface Surface; // Type of low-level stream surface representation
	typedef Visualization::Templatized::StreamsurfaceExtractor<DS,VE,SE,Surface> SSE; // Type of templatized stream surface extractor
	
	/* Elements: */
	private:
	const GLColorMap* colorMap; // Color map for the auxiliary scalar extractor of extracted streamlines
	SSE sse; // The templatized stream surface extractor
	DSScalar diskRadius; // Radius of disk of streamline seed positions around original query position
	size_t maxNumVertices; // The maximum number of vertices to be extracted
	StreamsurfacePointer currentStreamsurface; // The currently extracted stream surface visualization element
	
	/* UI components: */
	GLMotif::TextField* numStreamlinesValue; // Text field to display current number of streamlines
	GLMotif::Slider* numStreamlinesSlider; // Slider to change current number of streamlines
	GLMotif::TextField* diskRadiusValue; // Text field to display current seed disk radius
	GLMotif::Slider* diskRadiusSlider; // Slider to change current seed disk radius
	GLMotif::TextField* stepSizeValue; // Text field to display current step size value
	GLMotif::Slider* stepSizeSlider; // Slider to change current step size value
	GLMotif::TextField* maxNumVerticesValue; // Text field to display maximum number of extracted vertices
	GLMotif::Slider* maxNumVerticesSlider; // Slider to change maximum number of extracted vertices
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	static const VE& getVe(const Visualization::Abstract::VectorExtractor* sVectorExtractor);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	StreamsurfaceExtractor(const GLColorMap* sColorMap,const Visualization::Abstract::DataSet* sDataSet,const Visualization::Abstract::VectorExtractor* sVectorExtractor,const Visualization::Abstract::ScalarExtractor* sScalarExtractor,Visualization::Abstract::DataSet::Scalar sDiskRadius); // Creates a stream surface extractor for the given data set, vector extractor, and auxiliary scalar extractor
	virtual ~StreamsurfaceExtractor(void);
	
	/* Methods: */
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	const SSE& getSse(void) const // Returns the templatized stream surface extractor
		{
		return sse;
		}
	SSE& getSse(void) // Ditto
		{
		return sse;
		}
	Visualization::Abstract::DataSet::Scalar getDiskRadius(void) const // Returns the radius of the seed disk
		{
		return Visualization::Abstract::DataSet::Scalar(diskRadius);
		}
	size_t getMaxNumVertices(void) const // Returns the maximum number of vertices to be extracted
		{
		return maxNumVertices;
		}
	void setMaxNumVertices(size_t newMaxNumVertices); // Sets the maximum number of vertices to be extracted
	virtual bool hasSeededCreator(void) const;
	virtual bool hasIncrementalCreator(void) const;
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual Visualization::Abstract::Element* createElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual Visualization::Abstract::Element* startElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual bool continueElement(const Realtime::AlarmTimer& alarm);
	virtual void finishElement(void);
	void numStreamlinesSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void diskRadiusSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void stepSizeSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void maxNumVerticesSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_STREAMSURFACEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/StreamsurfaceExtractor.icpp>
#endif

#endif
