/***********************************************************************
StreamlineExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized streamline extractor
implementation.
Copyright (c) 2006-2012 Oliver Kreylos

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
#include <GLMotif/TextFieldSlider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/Streamline.h>

/* Forward declarations: */
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
	typedef typename DS::Scalar Scalar; // Scalar type of templatized data set
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Type for points in data set's domain
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::VE VE; // Type of templatized vector extractor
	typedef typename DataSetWrapper::VectorExtractor VectorExtractor; // Compatible vector extractor wrapper class
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::Streamline<DataSetWrapper> Streamline; // Type of created visualization elements
	typedef Misc::Autopointer<Streamline> StreamlinePointer; // Type for pointers to created visualization elements
	typedef typename Streamline::Polyline Polyline; // Type of low-level streamline representation
	typedef Visualization::Templatized::StreamlineExtractor<DS,VE,SE,Polyline> SLE; // Type of templatized streamline extractor
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for streamlines
		{
		friend class StreamlineExtractor;
		
		/* Elements: */
		private:
		int vectorVariableIndex; // Index of the vector variable defining the arrow rake
		int colorScalarVariableIndex; // Index of the scalar variable used to color the arrows
		size_t maxNumVertices; // Maximum number of vertices to be extracted
		Scalar epsilon; // Per-step accuracy threshold for streamline integration
		Point seedPoint; // The streamline's seeding point
		const DS* ds; // Data set from which to extract streamlines
		const VE* ve; // Vector extractor for data set
		const SE* cse; // Color scalar extractor for data set
		DSL dsl; // Templatized data set locator following the seed point
		bool locatorValid; // Flag if the locator has been properly initialized, and is inside the data set's domain
		
		/* Constructors and destructors: */
		public:
		Parameters(Visualization::Abstract::VariableManager* variableManager);
		
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
		
		/* New methods: */
		void update(Visualization::Abstract::VariableManager* variableManager,bool track); // Updates derived parameters after a read operation
		};
	
	/* Elements: */
	private:
	static const char* name; // Identifying name of this algorithm
	Parameters parameters; // The streamline extraction parameters used by this extractor
	SLE sle; // The templatized streamline extractor
	StreamlinePointer currentStreamline; // The currently extracted streamline visualization element
	
	/* UI components: */
	GLMotif::TextFieldSlider* maxNumVerticesSlider;
	GLMotif::TextFieldSlider* epsilonSlider;
	
	/* Constructors and destructors: */
	public:
	StreamlineExtractor(Visualization::Abstract::VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe); // Creates a streamline extractor
	virtual ~StreamlineExtractor(void);
	
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
	const SLE& getSle(void) const // Returns the templatized streamline extractor
		{
		return sle;
		}
	SLE& getSle(void) // Ditto
		{
		return sle;
		}
	void maxNumVerticesCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void epsilonCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_STREAMLINEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/StreamlineExtractor.icpp>
#endif

#endif
