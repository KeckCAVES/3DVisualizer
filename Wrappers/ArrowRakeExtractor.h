/***********************************************************************
ArrowRakeExtractor - Wrapper class extract rakes of arrows from vector
fields.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/Slider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/ArrowRake.h>

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
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class VectorParam,class SourceValueParam>
class VectorExtractor;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
template <class VEParam>
class VectorExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class ArrowRakeExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::VE VE; // Type of templatized vector extractor
	typedef typename DataSetWrapper::VectorExtractor VectorExtractor; // Compatible vector extractor wrapper class
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::ArrowRake<DataSetWrapper> ArrowRake; // Type of created visualization elements
	typedef Misc::Autopointer<ArrowRake> ArrowRakePointer; // Type for pointers to created visualization elements
	typedef typename ArrowRake::Scalar Scalar; // Type for scalars
	typedef typename ArrowRake::Point Point; // Type for points
	typedef typename ArrowRake::Vector Vector; // Type for vectors
	typedef typename ArrowRake::Arrow Arrow; // Type for rake arrow glyph descriptors
	typedef typename ArrowRake::Index Index; // Type for indices into rake arrays
	typedef typename ArrowRake::Rake Rake; // Type of low-level arrow rake representation
	typedef typename ArrowRake::Parameters Parameters; // Type for arrow rake extraction parameters
	
	/* Elements: */
	private:
	Parameters parameters; // The arrow rake extraction parameters used by this extractor
	Scalar baseCellSize; // Basis for cell size calculation
	const DS* dataSet; // Data set the arrow rake extractor works on
	VE vectorExtractor; // Vector extractor working on data set
	SE colorScalarExtractor; // Color scalar extractor working on data set
	DSL currentDsl; // The current locator
	ArrowRakePointer currentArrowRake; // The currently extracted arrow rake visualization element
	
	/* UI components: */
	GLMotif::TextField* rakeSizeValues[2]; // Text fields to display the current rake size
	GLMotif::Slider* rakeSizeSliders[2]; // Sliders to adjust the current rake size
	GLMotif::TextField* cellSizeValues[2]; // Text fields to display the current grid size
	GLMotif::Slider* cellSizeSliders[2]; // Sliders to adjust the current grid size
	GLMotif::TextField* lengthScaleValue; // Text field to display the current arrow length scaling factor
	GLMotif::Slider* lengthScaleSlider; // Sliders to adjust the current arrow length scaling factor
	
	/* Private methods: */
	static const DS* getDs(Visualization::Abstract::VariableManager* sVariableManager,int vectorVariableIndex,int colorScalarVariableIndex);
	static const VE& getVe(const Visualization::Abstract::VectorExtractor* sVectorExtractor);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	ArrowRakeExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates an arrow rake extractor
	virtual ~ArrowRakeExtractor(void);
	
	/* Methods: */
	virtual bool hasSeededCreator(void) const;
	virtual bool hasIncrementalCreator(void) const;
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual Visualization::Abstract::Element* createElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual Visualization::Abstract::Element* startElement(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual bool continueElement(const Realtime::AlarmTimer& alarm);
	virtual void finishElement(void);
	virtual Visualization::Abstract::Element* startSlaveElement(void);
	virtual void continueSlaveElement(void);
	void rakeSizeSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void cellSizeSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void lengthScaleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/ArrowRakeExtractor.cpp>
#endif

#endif
