/***********************************************************************
MultiStreamlineExtractor - Generic class to extract multiple stream
lines from data sets in parallel.
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

#ifndef VISUALIZATION_TEMPLATIZED_MULTISTREAMLINEEXTRACTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_MULTISTREAMLINEEXTRACTOR_INCLUDED

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
class MultiStreamlineExtractor
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
	typedef MultiStreamlineParam MultiStreamline; // Type of multi-streamline representation
	
	struct StreamlineState // Structure containing the state of the streamline extractor for each streamline
		{
		/* Elements: */
		public:
		Point p1; // Current streamline position
		Locator locator; // Locator following the current streamline position
		bool valid; // Flag if the streamline locator is valid
		Scalar stepSize; // Step size for the current streamline integration step
		};
	
	private:
	typedef typename MultiStreamline::Vertex Vertex; // Type of vertices stored in streamlines
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the isosurface extractor works on
	VectorExtractor vectorExtractor; // Vector extractor working on data set
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	Scalar epsilon; // The per-step accuracy threshold for streamline integration
	
	/* Streamline extraction state: */
	unsigned int numStreamlines; // Number of individual streamlines reflected in current state variables
	StreamlineState* streamlineStates; // Array of streamline states
	MultiStreamline* multiStreamline; // Pointer to the multi-streamline representations
	
	/* Private methods: */
	Vector cashKarpStep(unsigned int index,const Vector& vfp1,Scalar trialStepSize,Vector& error); // Computes a trial step vector with Cash-Karp coefficients
	bool stepStreamline(unsigned int index); // Advances one current streamline position by one step
	
	/* Constructors and destructors: */
	public:
	MultiStreamlineExtractor(const DataSet* sDataSet,const VectorExtractor& sVectorExtractor,const ScalarExtractor& sScalarExtractor); // Creates a streamline extractor for the given data set and vector and scalar extractors
	private:
	MultiStreamlineExtractor(const MultiStreamlineExtractor& source); // Prohibit copy constructor
	MultiStreamlineExtractor& operator=(const MultiStreamlineExtractor& source); // Prohibit assignment operator
	public:
	~MultiStreamlineExtractor(void); // Destroys the streamline extractor
	
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
	Scalar getEpsilon(void) const // Returns the integration accuracy threshold
		{
		return epsilon;
		}
	unsigned int getNumStreamlines(void) const // Returns the number of individual streamlines used in the last extraction
		{
		return numStreamlines;
		}
	void update(const DataSet* newDataSet,const VectorExtractor& newVectorExtractor,const ScalarExtractor& newScalarExtractor) // Sets a new data set and scalar / vector extractors for subsequent multi-streamline extraction
		{
		dataSet=newDataSet;
		vectorExtractor=newVectorExtractor;
		scalarExtractor=newScalarExtractor;
		}
	void setEpsilon(Scalar newEpsilon); // Sets the integration accuracy threshold
	void setNumStreamlines(unsigned int newNumStreamlines); // Sets number of streamlines without setting the multi-streamline itself
	void setMultiStreamline(MultiStreamline& newMultiStreamline); // Sets the multi-streamline object
	void initializeStreamline(unsigned int index,const Point& startPoint,const Locator& startLocator,Scalar startEpsilon); // Initializes one streamline
	void extractStreamlines(void); // Extracts streamlines for the previously initialized positions, locators, and streamline storages
	void startStreamlines(void); // Starts extracting streamlines for the previously initialized positions, locators, and streamline storages
	template <class ContinueFunctorParam>
	bool continueStreamlines(const ContinueFunctorParam& cf); // Continues extracting streamlines while the continue functor returns true; returns true if the streamlines are finished
	void finishStreamlines(void); // Cleans up after creating streamlines
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_MULTISTREAMLINEEXTRACTOR_IMPLEMENTATION
#include <Templatized/MultiStreamlineExtractor.icpp>
#endif

#endif
