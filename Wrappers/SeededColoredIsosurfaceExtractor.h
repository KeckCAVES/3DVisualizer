/***********************************************************************
SeededColoredIsosurfaceExtractor - Wrapper class to map from the
abstract visualization algorithm interface to a templatized colored
isosurface extractor implementation.
Copyright (c) 2008-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_SEEDEDCOLOREDISOSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_SEEDEDCOLOREDISOSURFACEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/TextFieldSlider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/ColoredIsosurface.h>

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
class ColoredIsosurfaceExtractor;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class SeededColoredIsosurfaceExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Type for points in data set's domain
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::ColoredIsosurface<DataSetWrapper> ColoredIsosurface; // Type of created visualization elements
	typedef Misc::Autopointer<ColoredIsosurface> ColoredIsosurfacePointer; // Type for pointers to created visualization elements
	typedef typename ColoredIsosurface::Surface Surface; // Type of low-level surface representation
	typedef Visualization::Templatized::ColoredIsosurfaceExtractor<DS,SE,Surface> CISE; // Type of templatized colored isosurface extractor
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for seeded isosurfaces
		{
		friend class SeededColoredIsosurfaceExtractor;
		
		/* Elements: */
		private:
		int scalarVariableIndex; // Index of the scalar variable to color the isosurface
		int colorScalarVariableIndex; // Index of the scalar variable to color the isosurface
		size_t maxNumTriangles; // Maximum number of triangles to be extracted
		bool smoothShading; // Flag to enable smooth shading by calculating scalar field gradients at each vertex position
		bool lighting; // Flag whether to use a lighting when rendering the isosurface
		VScalar isovalue; // The isosurface's isovalue
		Point seedPoint; // Point from which the isosurface was seeded
		DSL dsl; // Templatized data set locator following the seed point
		bool locatorValid; // Flag if the locator has been properly initialized, and is inside the data set's domain
		
		/* Constructors and destructors: */
		public:
		Parameters(int sScalarVariableIndex,int sColorScalarVariableIndex)
			:scalarVariableIndex(sScalarVariableIndex),
			 colorScalarVariableIndex(sColorScalarVariableIndex),
			 locatorValid(false)
			{
			}
		
		/* Methods from Abstract::Parameters: */
		virtual bool isValid(void) const
			{
			return locatorValid;
			}
		virtual Visualization::Abstract::Parameters* clone(void) const
			{
			return new Parameters(*this);
			}
		virtual void write(Visualization::Abstract::ParametersSink& sink) const;
		virtual void read(Visualization::Abstract::ParametersSource& source);
		};
	
	/* Elements: */
	private:
	static const char* name; // Identifying name of this algorithm
	Parameters parameters; // The colored isosurface extraction parameters used by this extractor
	CISE cise; // The templatized colored isosurface extractor
	ColoredIsosurfacePointer currentColoredIsosurface; // The currently extracted colored isosurface visualization element
	
	/* UI components: */
	GLMotif::TextFieldSlider* maxNumTrianglesSlider;
	GLMotif::DropdownBox* colorScalarVariableBox;
	GLMotif::RadioBox* extractionModeBox;
	GLMotif::ToggleButton* lightingToggle;
	GLMotif::TextField* currentValue; // Text field to display scalar value at current locator position
	
	/* Private methods: */
	static const DS* getDs(Visualization::Abstract::VariableManager* sVariableManager,int scalarVariableIndex,int colorScalarVariableIndex);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	SeededColoredIsosurfaceExtractor(Visualization::Abstract::VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe); // Creates a colored isosurface extractor
	virtual ~SeededColoredIsosurfaceExtractor(void);
	
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
	virtual void readParameters(Visualization::Abstract::ParametersSource& source);
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
	const CISE& getCise(void) const // Returns the templatized colored isosurface extractor
		{
		return cise;
		}
	CISE& getCise(void) // Ditto
		{
		return cise;
		}
	void maxNumTrianglesCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void colorScalarVariableBoxCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void extractionModeBoxCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void lightingToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_SEEDEDCOLOREDISOSURFACEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/SeededColoredIsosurfaceExtractor.icpp>
#endif

#endif
