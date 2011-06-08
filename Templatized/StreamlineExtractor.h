/***********************************************************************
StreamlineExtractor - Generic class to extract stream lines from data
sets.
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

#ifndef VISUALIZATION_TEMPLATIZED_STREAMLINEEXTRACTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_STREAMLINEEXTRACTOR_INCLUDED

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
class StreamlineExtractor
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the streamline extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef typename DataSet::Locator Locator; // Type of data set locators
	typedef VectorExtractorParam VectorExtractor; // Type to extract vector values from a data set (to trace the streamline)
	typedef typename VectorExtractor::Vector VVector; // Value type of vector extractor
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set (to color the streamline)
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	typedef StreamlineParam Streamline; // Type of streamline representation
	
	private:
	typedef typename Streamline::Vertex Vertex; // Type of vertices stored in streamline
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the streamline extractor works on
	VectorExtractor vectorExtractor; // Vector extractor working on data set
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	Scalar epsilon; // The per-step accuracy threshold for streamline integration
	
	/* Streamline extraction state: */
	Point p1; // Current streamline position
	Locator locator; // Locator following the current streamline position
	Scalar stepSize; // Step size for the current streamline integration step
	Streamline* streamline; // Pointer to the streamline representation
	
	/* Private methods: */
	bool cashKarpStep(const Vector& vfp1,Scalar trialStepSize,Vector& step,Vector& error); // Computes a trial step vector with Cash-Karp coefficients; returns false if any evaluation point was outside the domain
	bool stepStreamline(void); // Advances the current streamline position by one step
	
	/* Constructors and destructors: */
	public:
	StreamlineExtractor(const DataSet* sDataSet,const VectorExtractor& sVectorExtractor,const ScalarExtractor& sScalarExtractor); // Creates a streamline extractor for the given data set and vector and scalar extractors
	private:
	StreamlineExtractor(const StreamlineExtractor& source); // Prohibit copy constructor
	StreamlineExtractor& operator=(const StreamlineExtractor& source); // Prohibit assignment operator
	public:
	~StreamlineExtractor(void); // Destroys the streamline extractor
	
	/* Methods: */
	const DataSet* getDataSet(void) const // Returns the data set
		{
		return dataSet;
		}
	const VectorExtractor& getVectorExtractor(void) const // Returns the vector extractor
		{
		return vectorExtractor;
		}
	VectorExtractor& getVectorExtractor(void) // Ditto
		{
		return vectorExtractor;
		}
	const ScalarExtractor& getScalarExtractor(void) const // Returns the scalar extractor
		{
		return scalarExtractor;
		}
	ScalarExtractor& getScalarExtractor(void) // Ditto
		{
		return scalarExtractor;
		}
	Scalar getEpsilon(void) const // Returns the integration error threshold
		{
		return epsilon;
		}
	void update(const DataSet* newDataSet,const VectorExtractor& newVectorExtractor,const ScalarExtractor& newScalarExtractor) // Sets a new data set and scalar / vector extractors for subsequent streamline extraction
		{
		dataSet=newDataSet;
		vectorExtractor=newVectorExtractor;
		scalarExtractor=newScalarExtractor;
		}
	void setEpsilon(Scalar newEpsilon); // Sets the integration error threshold
	void extractStreamline(const Point& startPoint,const Locator& startLocator,Scalar startStepSize,Streamline& newStreamline); // Extracts a streamline for the given position and locator and stores it in the given streamline
	void startStreamline(const Point& startPoint,const Locator& startLocator,Scalar startStepSize,Streamline& newStreamline); // Starts extracting a streamline for the given position and locator and stores it in the given streamline
	template <class ContinueFunctorParam>
	bool continueStreamline(const ContinueFunctorParam& cf); // Continues extracting a streamline while the continue functor returns true; returns true if the streamline is finished
	void finishStreamline(void); // Cleans up after creating a streamline
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_STREAMLINEEXTRACTOR_IMPLEMENTATION
#include <Templatized/StreamlineExtractor.icpp>
#endif

#endif
