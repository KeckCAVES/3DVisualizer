/***********************************************************************
SeededIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
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

#ifndef VISUALIZATION_WRAPPERS_SEEDEDISOSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_SEEDEDISOSURFACEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/Isosurface.h>

/* Forward declarations: */
namespace GLMotif {
class TextField;
}
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class Element;
}
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
class IsosurfaceExtractor;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class SeededIsosurfaceExtractor:public Visualization::Abstract::Algorithm
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
	typedef Visualization::Wrappers::Isosurface<DataSetWrapper> Isosurface; // Type of created visualization elements
	typedef Misc::Autopointer<Isosurface> IsosurfacePointer; // Type for pointers to created visualization elements
	typedef typename Isosurface::Parameters Parameters; // Type for isosurface extraction parameters
	typedef typename Isosurface::Surface Surface; // Type of low-level surface representation
	typedef Visualization::Templatized::IsosurfaceExtractor<DS,SE,Surface> ISE; // Type of templatized isosurface extractor
	
	/* Elements: */
	private:
	Parameters parameters; // The isosurface extraction parameters used by this extractor
	ISE ise; // The templatized isosurface extractor
	IsosurfacePointer currentIsosurface; // The currently extracted isosurface visualization element
	
	/* UI components: */
	GLMotif::TextField* maxNumTrianglesValue; // Text field to display current maximum number of triangles
	GLMotif::Slider* maxNumTrianglesSlider; // Slider to change current maximum number of triangles
	GLMotif::RadioBox* extractionModeBox; // Radio box with toggles for extraction modes
	GLMotif::TextField* currentValue; // Text field to display scalar value at current locator position
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	SeededIsosurfaceExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates an isosurface extractor
	virtual ~SeededIsosurfaceExtractor(void);
	
	/* Methods: */
	const ISE& getIse(void) const // Returns the templatized isosurface extractor
		{
		return ise;
		}
	ISE& getIse(void) // Ditto
		{
		return ise;
		}
	size_t getMaxNumTriangles(void) const // Returns the maximum number of triangles to be extracted
		{
		return parameters.maxNumTriangles;
		}
	void setMaxNumTriangles(size_t newMaxNumTriangles); // Sets the maximum number of triangles to be extracted
	virtual bool hasSeededCreator(void) const;
	virtual bool hasIncrementalCreator(void) const;
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual Visualization::Abstract::Element* createElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual Visualization::Abstract::Element* startElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual bool continueElement(const Realtime::AlarmTimer& alarm);
	virtual void finishElement(void);
	virtual Visualization::Abstract::Element* startSlaveElement(void);
	virtual void continueSlaveElement(void);
	void maxNumTrianglesSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void extractionModeBoxCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_SEEDEDISOSURFACEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/SeededIsosurfaceExtractor.cpp>
#endif

#endif
