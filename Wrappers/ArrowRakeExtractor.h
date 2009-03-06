/***********************************************************************
ArrowRakeExtractor - Wrapper class extract rakes of arrows from vector
fields.
Copyright (c) 2008-2009 Oliver Kreylos

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
#include <Abstract/Parameters.h>
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
	typedef typename DS::Scalar Scalar; // Type for scalars
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Type for points
	typedef typename DS::Vector Vector; // Type for vectors
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::VE VE; // Type of templatized vector extractor
	typedef typename DataSetWrapper::VectorExtractor VectorExtractor; // Compatible vector extractor wrapper class
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::ArrowRake<DataSetWrapper> ArrowRake; // Type of created visualization elements
	typedef Misc::Autopointer<ArrowRake> ArrowRakePointer; // Type for pointers to created visualization elements
	typedef typename ArrowRake::Arrow Arrow; // Type for rake arrow glyph descriptors
	typedef typename ArrowRake::Index Index; // Type for indices into rake arrays
	typedef typename ArrowRake::Rake Rake; // Type of low-level arrow rake representation
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for seeded isosurfaces
		{
		friend class ArrowRakeExtractor;
		
		/* Elements: */
		private:
		int vectorVariableIndex; // Index of the vector variable defining the arrow rake
		int colorScalarVariableIndex; // Index of the scalar variable used to color the arrows
		Index rakeSize; // Number of columns/rows of the arrow rake
		Scalar cellSize[2]; // Distance between adjacent arrows in the same column/row
		Scalar lengthScale; // Arrow length scale
		Scalar shaftRadius; // Radius of the shafts of the arrow glyphs
		unsigned int numArrowVertices; // Number of vertices per arrow for arrow glyph creation
		Point base; // The rake base point
		Vector frame[2]; // The directions of columns and rows in the rake
		const DS* ds; // Data set from which to extract arrows
		const VE* ve; // Vector extractor for data set
		const SE* cse; // Color scalar extractor for data set
		DSL dsl; // Templatized data set locator following the seed point
		bool locatorValid; // Flag if the locator has been properly initialized, and is inside the data set's domain
		
		/* Private methods: */
		template <class DataSourceParam>
		void readBinary(DataSourceParam& dataSource,bool raw,const Visualization::Abstract::VariableManager* variableManager); // Reads parameters from a binary data source
		template <class DataSourceParam>
		void writeBinary(DataSourceParam& dataSink,bool raw,const Visualization::Abstract::VariableManager* variableManager) const; // Writes parameters to a binary data source
		
		/* Constructors and destructors: */
		public:
		Parameters(Visualization::Abstract::VariableManager* variableManager);
		
		/* Methods from Abstract::Parameters: */
		virtual bool isValid(void) const
			{
			return locatorValid;
			}
		virtual void read(Misc::File& file,bool ascii,Visualization::Abstract::VariableManager* variableManager);
		virtual void read(Comm::MulticastPipe& pipe,Visualization::Abstract::VariableManager* variableManager);
		virtual void read(Comm::ClusterPipe& pipe,Visualization::Abstract::VariableManager* variableManager);
		virtual void write(Misc::File& file,bool ascii,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual void write(Comm::MulticastPipe& pipe,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual void write(Comm::ClusterPipe& pipe,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual Visualization::Abstract::Parameters* clone(void) const
			{
			return new Parameters(*this);
			}
		
		/* New methods: */
		void update(Visualization::Abstract::VariableManager* variableManager,bool track); // Updates derived parameters after a read operation
		};
	
	/* Elements: */
	private:
	static const char* name; // Identifying name of this algorithm
	Parameters parameters; // The arrow rake extraction parameters used by this extractor
	Scalar baseCellSize; // Basis for cell size calculation
	ArrowRakePointer currentArrowRake; // The currently extracted arrow rake visualization element
	Parameters* currentParameters; // Pointer to parameter object for current extraction
	
	/* UI components: */
	GLMotif::TextField* rakeSizeValues[2]; // Text fields to display the current rake size
	GLMotif::Slider* rakeSizeSliders[2]; // Sliders to adjust the current rake size
	GLMotif::TextField* cellSizeValues[2]; // Text fields to display the current grid size
	GLMotif::Slider* cellSizeSliders[2]; // Sliders to adjust the current grid size
	GLMotif::TextField* lengthScaleValue; // Text field to display the current arrow length scaling factor
	GLMotif::Slider* lengthScaleSlider; // Sliders to adjust the current arrow length scaling factor
	
	/* Constructors and destructors: */
	public:
	ArrowRakeExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates an arrow rake extractor
	virtual ~ArrowRakeExtractor(void);
	
	/* Methods from Visualization::Abstract::Algorithm: */
	virtual const char* getName(void) const
		{
		return name;
		}
	virtual bool hasSeededCreator(void) const
		{
		return true;
		}
	virtual bool hasIncrementalCreator(void) const
		{
		return true;
		}
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual Visualization::Abstract::Parameters* cloneParameters(void) const
		{
		return new Parameters(parameters);
		}
	virtual void setSeedLocator(const Visualization::Abstract::DataSet::Locator* seedLocator);
	virtual Visualization::Abstract::Element* createElement(Visualization::Abstract::Parameters* extractParameters);
	virtual Visualization::Abstract::Element* startElement(Visualization::Abstract::Parameters* extractParameters);
	virtual bool continueElement(const Realtime::AlarmTimer& alarm);
	virtual void finishElement(void);
	virtual Visualization::Abstract::Element* startSlaveElement(Visualization::Abstract::Parameters* extractParameters);
	virtual void continueSlaveElement(void);
	
	/* New methods: */
	static const char* getClassName(void) // Returns the algorithm class name
		{
		return name;
		}
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
