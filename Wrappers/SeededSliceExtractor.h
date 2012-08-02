/***********************************************************************
SeededSliceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized slice extractor
implementation.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_SEEDEDSLICEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_SEEDEDSLICEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>

#include <Abstract/DataSet.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/Slice.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class Element;
}
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class DataSetParam,class ScalarExtractorParam,class SliceParam>
class SliceExtractor;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class SeededSliceExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Type for points in the data set's domain
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef typename DataSetWrapper::Locator Locator; // Type of locator wrapper
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::Slice<DataSetWrapper> Slice; // Type of created visualization elements
	typedef Misc::Autopointer<Slice> SlicePointer; // Type for pointers to created visualization elements
	typedef typename Slice::Surface Surface; // Type of low-level surface representation
	typedef Visualization::Templatized::SliceExtractor<DS,SE,Surface> SLE; // Type of templatized slice extractor
	typedef typename SLE::Plane Plane; // Type of slicing planes
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for seeded slices
		{
		friend class SeededSliceExtractor;
		
		/* Elements: */
		private:
		int scalarVariableIndex; // Index of the scalar variable to color the slice
		Plane plane; // The slice's plane equation
		Point seedPoint; // Point from which the slice was seeded
		DSL dsl; // Templatized data set locator following the seed point
		bool locatorValid; // Flag if the locator has been properly initialized, and is inside the data set's domain
		
		/* Constructors and destructors: */
		public:
		Parameters(int sScalarVariableIndex)
			:scalarVariableIndex(sScalarVariableIndex),
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
	Parameters parameters; // The slice extraction parameters used by this extractor
	SLE sle; // The templatized slice extractor
	SlicePointer currentSlice; // The currently extracted slice visualization element
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	SeededSliceExtractor(Visualization::Abstract::VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe); // Creates a slice extractor
	virtual ~SeededSliceExtractor(void);
	
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
	const SLE& getSle(void) const // Returns the templatized slice extractor
		{
		return sle;
		}
	SLE& getSle(void) // Ditto
		{
		return sle;
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_SEEDEDSLICEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/SeededSliceExtractor.icpp>
#endif

#endif
