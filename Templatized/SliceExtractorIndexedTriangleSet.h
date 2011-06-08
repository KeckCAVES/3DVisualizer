/***********************************************************************
SliceExtractorIndexedTriangleSet - Specialized version of SliceExtractor
class for indexed triangle sets.
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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEEXTRACTORINDEXEDTRIANGLESET_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEEXTRACTORINDEXEDTRIANGLESET_INCLUDED

#include <Misc/HashTable.h>
#include <Misc/OneTimeQueue.h>
#include <Geometry/Plane.h>
#include <Templatized/IndexedTriangleSet.h>
#include <Templatized/SliceExtractor.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class CellTopologyParam>
class SliceCaseTable;
}
}

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
class SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the slice extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef Geometry::Plane<Scalar,DataSet::dimension> Plane; // Type for planes in the data set's domain
	typedef typename DataSet::Locator Locator; // Type of data set locators
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	typedef IndexedTriangleSet<VertexParam> Slice; // Type of slice representation
	
	private:
	typedef typename DataSet::CellTopology CellTopology; // Topology of the data set's cells
	typedef typename DataSet::EdgeID EdgeID; // Type of the data set's edge IDs
	typedef typename DataSet::CellID CellID; // Type of the data set's cell IDs
	typedef typename DataSet::Cell Cell; // Type of the data set's cells
	typedef Misc::OneTimeQueue<CellID,CellID> CellQueue; // Type for queues of cell IDs waiting for expansion
	typedef SliceCaseTable<CellTopology> CaseTable; // Type of slice case table
	typedef typename Slice::Vertex Vertex; // Type of vertices stored in slice
	typedef typename Slice::Index Index; // Type for vertex indices
	typedef Misc::HashTable<EdgeID,Index,EdgeID> VertexIndexHasher; // Hash table to map edge IDs to vertex indices in the slice
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the isosurface extractor works on
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	
	/* Slice extraction state: */
	Plane slicePlane; // The current slicing plane
	Slice* slice; // Pointer to the slice representation storing extracted slice fragments
	VertexIndexHasher vertexIndices; // Hasher mapping edge IDs to vertex indices in the slice
	CellQueue cellQueue; // Queue of cells waiting for fragment extraction
	
	/* Private methods: */
	int extractSliceFragment(const Cell& cell); // Extracts a slice fragment from a cell and stores it in the current slice representation
	
	/* Constructors and destructors: */
	public:
	SliceExtractor(const DataSet* sDataSet,const ScalarExtractor& sScalarExtractor); // Creates a slice extractor for the given data set and scalar extractor
	private:
	SliceExtractor(const SliceExtractor& source); // Prohibit copy constructor
	SliceExtractor& operator=(const SliceExtractor& source); // Prohibit assignment operator
	public:
	~SliceExtractor(void); // Destroys the slice extractor
	
	/* Methods: */
	const DataSet* getDataSet(void) const // Returns the data set
		{
		return dataSet;
		}
	const ScalarExtractor& getScalarExtractor(void) const // Returns the scalar extractor
		{
		return scalarExtractor;
		}
	ScalarExtractor& getScalarExtractor(void) // Ditto
		{
		return scalarExtractor;
		}
	void update(const DataSet* newDataSet,const ScalarExtractor& newScalarExtractor) // Sets a new data set and scalar extractor for subsequent slice extraction
		{
		dataSet=newDataSet;
		scalarExtractor=newScalarExtractor;
		}
	void extractSlice(const Plane& newSlicePlane,Slice& newSlice); // Extracts a global slice for the given plane and stores it in the given slice
	void extractSeededSlice(const Locator& seedLocator,const Plane& newSlicePlane,Slice& newSlice); // Extracts a seeded slice for the given plane from the given cell and stores it in the given slice
	void startSeededSlice(const Locator& seedLocator,const Plane& newSlicePlane,Slice& newSlice); // Starts extracting a seeded slice for the given plane from the given cell
	template <class ContinueFunctorParam>
	bool continueSeededSlice(const ContinueFunctorParam& cf); // Continues extracting a seeded slice while the continue functor returns true; returns true if the slice is finished
	void finishSeededSlice(void); // Cleans up after creating a seeded slice
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SLICEEXTRACTORINDEXEDTRIANGLESET_IMPLEMENTATION
#include <Templatized/SliceExtractorIndexedTriangleSet.icpp>
#endif

#endif
