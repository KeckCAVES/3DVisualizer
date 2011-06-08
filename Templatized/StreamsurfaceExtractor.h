/***********************************************************************
StreamsurfaceExtractor - Class to extract stream surfaces from data
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

#ifndef VISUALIZATION_TEMPLATIZED_STREAMSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_STREAMSURFACEEXTRACTOR_INCLUDED

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
class StreamsurfaceExtractor
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the stream surface extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef typename DataSet::Locator Locator; // Type of data set locators
	typedef VectorExtractorParam VectorExtractor; // Type to extract vector values from a data set (to trace the streamlines)
	typedef typename VectorExtractor::Vector VVector; // Value type of vector extractor
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set (to color the streamlines)
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	typedef StreamsurfaceParam Streamsurface; // Type of stream surface representation
	
	private:
	typedef typename Streamsurface::Vertex Vertex; // Type of vertices stored in stream surface
	
	struct Streamline // Structure storing the current state of one streamline defining the surface
		{
		/* Elements: */
		public:
		Point pos0; // Tracing position at beginning of iteration step
		Locator locator; // Data set locator for current tracing position
		VScalar scalar0; // Associated scalar value at the initial position
		Vector vec0; // Vector value at the initial position
		Point pos1; // Tracing position at end of iteration step
		Scalar arclength; // Accumulated arc length of the streamline including the initial position
		typename Streamsurface::Index index; // Index of most recently created vertex
		Streamline* pred; // Pointer to previous streamline in the surface
		Streamline* succ; // Pointer to next streamline in the surface
		bool connectPred; // Flag whether this streamline is connected to the previous one
		bool connectSucc; // Flag whether this streamline is connected to the next one
		};
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the isosurface extractor works on
	VectorExtractor vectorExtractor; // Vector extractor working on data set
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	Scalar stepSize; // Fixed step size for streamline integration
	
	/* Streamline extraction state: */
	Streamline* streamlineHead; // Pointer to one of the streamlines defining the surface
	Streamsurface* streamsurface; // Pointer to the stream surface representation
	
	/* Private methods: */
	bool stepStreamline(Streamline& s); // Advances the given streamline by one step
	bool stepStreamsurface(void); // Advances all current streamline positions by one step and adds a new layer to the stream surface
	
	/* Constructors and destructors: */
	public:
	StreamsurfaceExtractor(const DataSet* sDataSet,const VectorExtractor& sVectorExtractor,const ScalarExtractor& sScalarExtractor); // Creates a stream surface extractor for the given data set and vector and scalar extractors
	private:
	StreamsurfaceExtractor(const StreamsurfaceExtractor& source); // Prohibit copy constructor
	StreamsurfaceExtractor& operator=(const StreamsurfaceExtractor& source); // Prohibit assignment operator
	public:
	~StreamsurfaceExtractor(void); // Destroys the stream surface extractor
	
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
	Scalar getStepSize(void) const // Returns the integration step size
		{
		return stepSize;
		}
	void setStepSize(Scalar newStepSize); // Sets the integration step size
	int getNumStreamlines(void) const // Returns the number of streamlines extracted in parallel
		{
		return numStreamlines;
		}
	void setNumStreamlines(int newNumStreamlines); // Sets the number of streamlines extracted in parallel
	void setClosed(bool newClosed); // Sets if the stream surface is open or a closed tube
	void initializeStreamline(int index,const Point& startPoint,const Locator& startLocator); // Initializes one streamline
	void extractStreamsurface(Streamsurface& newStreamsurface); // Extracts stream surface for the previously initialized positions and locators
	void startStreamsurface(Streamsurface& newStreamsurface); // Starts extracting stream surface for the previously initialized positions and locators
	template <class ContinueFunctorParam>
	bool continueStreamsurface(const ContinueFunctorParam& cf); // Continues extracting stream surface while the continue functor returns true; returns true if the stream surface is finished
	void finishStreamsurface(void); // Cleans up after creating stream surface
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_STREAMSURFACEEXTRACTOR_IMPLEMENTATION
#include <Templatized/StreamsurfaceExtractor.icpp>
#endif

#endif
