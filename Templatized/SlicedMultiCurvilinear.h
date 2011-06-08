/***********************************************************************
SlicedMultiCurvilinear - Base class for vertex-centered multi-block
curvilinear data sets containing arbitrary numbers of independent scalar
fields, combined into vector and/or tensor fields using special value
extractors.
Copyright (c) 2008-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDMULTICURVILINEAR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEDMULTICURVILINEAR_INCLUDED

#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>

#include <Templatized/SlicedDataValue.h>
#include <Templatized/Tesseract.h>
#include <Templatized/LinearIndexID.h>
#include <Templatized/IteratorWrapper.h>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
class SlicedMultiCurvilinear
	{
	/* Embedded classes: */
	public:
	
	/* Definition of the data set's domain space: */
	typedef ScalarParam Scalar; // Scalar type of data set's domain
	static const int dimension=dimensionParam; // Dimension of data set's domain
	typedef Geometry::Point<Scalar,dimensionParam> Point; // Type for points in data set's domain
	typedef Geometry::Vector<Scalar,dimensionParam> Vector; // Type for vectors in data set's domain
	typedef Geometry::Box<Scalar,dimensionParam> Box; // Type for axis-aligned boxes in data set's domain
	
	/* Definition of the data set's cell topology: */
	typedef Tesseract<dimensionParam> CellTopology; // Policy class to select appropriate cell algorithms
	
	/* Definition of the data set's value space: */
	typedef ValueScalarParam ValueScalar; // Data set's value type for scalar values
	typedef SlicedDataValue<ValueScalar> Value; // Data set's compound value type
	
	/* Low-level definitions of data set storage: */
	typedef Misc::ArrayIndex<dimensionParam> Index; // Index type for data set storage (grids and value slices)
	typedef Misc::Array<Point,dimensionParam> GridArray; // Array type for grids
	
	class Vertex;
	class Cell;
	class Locator;
	
	class Grid // Structure describing one grid
		{
		friend class SlicedMultiCurvilinear;
		friend class Vertex;
		friend class Cell;
		friend class Locator;
		
		/* Elements: */
		private:
		Index numVertices; // Number of vertices in grid in each dimension
		GridArray grid; // Array defining grid's mesh
		ptrdiff_t gridBaseLinearIndex; // Linear base index of this grid's vertices in the data set
		int vertexStrides[dimension]; // Array of pointer stride values in the vertex array
		Index numCells; // Number of cells in data set in each dimension
		int vertexOffsets[CellTopology::numVertices]; // Array of pointer offsets from a cell's base vertex to all cell vertices
		
		/* Constructors and destructors: */
		private:
		Grid(void); // Creates an empty grid
		
		/* Methods: */
		void setGrid(const Index& sNumVertices,const Point* sVertexPositions =0); // Creates a grid with the given number of vertices; copies vertex positions if pointer is not null
		public:
		const Index& getNumVertices(void) const // Returns number of vertices in the grid
			{
			return numVertices;
			}
		const GridArray& getGrid(void) const // Returns the grid's mesh
			{
			return grid;
			}
		GridArray& getGrid(void) // Ditto
			{
			return grid;
			}
		const Point& getVertexPosition(const Index& vertexIndex) const // Returns a vertex' position
			{
			return grid(vertexIndex);
			}
		Point& getVertexPosition(const Index& vertexIndex) // Ditto
			{
			return grid(vertexIndex);
			}
		const Point& getVertexPosition(ptrdiff_t vertexLinearIndex) const // Returns a vertex' position based on its linear index in the overall data set
			{
			return grid.getArray()[vertexLinearIndex-gridBaseLinearIndex];
			}
		ptrdiff_t getVertexLinearIndex(const Index& vertexIndex) const // Returns the linear index of a vertex in the overall data set
			{
			return numVertices.calcOffset(vertexIndex)+gridBaseLinearIndex;
			}
		const Index& getNumCells(void) const // Returns number of cells in grid
			{
			return numCells;
			}
		};
	
	/* Data set interface classes: */
	typedef LinearIndexID VertexID;
	
	class Cell;
	
	class Vertex // Class to represent and iterate through vertices
		{
		friend class SlicedMultiCurvilinear;
		friend class Cell;
		
		/* Elements: */
		private:
		const SlicedMultiCurvilinear* ds; // Pointer to data set containing the vertex
		int gridIndex; // Index of the grid containing the vertex
		Index index; // Array index of vertex in grid's storage
		
		/* Constructors and destructors: */
		public:
		Vertex(void) // Creates an invalid vertex
			:ds(0)
			{
			}
		private:
		Vertex(const SlicedMultiCurvilinear* sDs,int sGridIndex,const Index& sIndex)
			:ds(sDs),gridIndex(sGridIndex),index(sIndex)
			{
			}
		
		/* Methods: */
		public:
		const Point& getPosition(void) const // Returns vertex' position in domain
			{
			return ds->grids[gridIndex].grid(index);
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getValue(const ValueExtractorParam& extractor) const // Returns vertex' value based on given extractor
			{
			return extractor.getValue(ds->grids[gridIndex].getVertexLinearIndex(index));
			}
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const // Returns gradient at the vertex, based on given scalar extractor
			{
			return ds->calcVertexGradient(gridIndex,index,extractor);
			}
		VertexID getID(void) const // Returns vertex' ID
			{
			return VertexID(VertexID::Index(ds->grids[gridIndex].getVertexLinearIndex(index)));
			}
		
		/* Iterator methods: */
		friend bool operator==(const Vertex& v1,const Vertex& v2)
			{
			return v1.index==v2.index&&v1.gridIndex==v2.gridIndex&&v1.ds==v2.ds;
			}
		friend bool operator!=(const Vertex& v1,const Vertex& v2)
			{
			return v1.index!=v2.index||v1.gridIndex!=v2.gridIndex||v1.ds!=v2.ds;
			}
		Vertex& operator++(void) // Pre-increment operator
			{
			index.preInc(ds->grids[gridIndex].numVertices);
			if(index[0]==ds->grids[gridIndex].numVertices[0])
				{
				++gridIndex;
				index[0]=0;
				}
			return *this;
			}
		};
	
	typedef IteratorWrapper<Vertex> VertexIterator; // Class to iterate through vertices
	
	typedef LinearIndexID EdgeID; // Class to identify cell edges
	
	typedef LinearIndexID CellID; // Class to identify cells
	
	class Locator;
	
	class Cell // Class to represent and iterate through cells
		{
		friend class SlicedMultiCurvilinear;
		friend class Locator;
		
		/* Elements: */
		private:
		const SlicedMultiCurvilinear* ds; // Pointer to the data set containing the cell
		int gridIndex; // Index of grid containing the cell
		Index index; // Array index of cell's base vertex in data set storage
		ptrdiff_t baseVertexIndex; // Linear index of cell's base vertex in data set storage (grid and value slices)
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:ds(0),baseVertexIndex(-1)
			{
			}
		private:
		Cell(const SlicedMultiCurvilinear* sDs)
			:ds(sDs),baseVertexIndex(-1)
			{
			}
		Cell(const SlicedMultiCurvilinear* sDs,int sGridIndex,const Index& sIndex)
			:ds(sDs),gridIndex(sGridIndex),index(sIndex),baseVertexIndex(ds->grids[gridIndex].getVertexLinearIndex(index))
			{
			}
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the cell is valid
			{
			return baseVertexIndex!=-1;
			}
		VertexID getVertexID(int vertexIndex) const // Returns ID of given vertex of the cell
			{
			return VertexID(VertexID::Index(baseVertexIndex+ds->grids[gridIndex].vertexOffsets[vertexIndex]));
			}
		Vertex getVertex(int vertexIndex) const; // Returns given vertex of the cell
		const Point& getVertexPosition(int vertexIndex) const // Returns position of given vertex of the cell
			{
			return ds->grids[gridIndex].getVertexPosition(baseVertexIndex+ds->grids[gridIndex].vertexOffsets[vertexIndex]);
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getVertexValue(int vertexIndex,const ValueExtractorParam& extractor) const // Returns value of given vertex of the cell, based on given extractor
			{
			return extractor.getValue(baseVertexIndex+ds->grids[gridIndex].vertexOffsets[vertexIndex]);
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at given vertex of the cell, based on given scalar extractor
		EdgeID getEdgeID(int edgeIndex) const; // Returns ID of given edge of the cell
		Point calcEdgePosition(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		Point calcFaceCenter(int faceIndex) const; // Returns the center of the cell's given face
		CellID getID(void) const // Returns cell's ID
			{
			return CellID(CellID::Index(baseVertexIndex));
			}
		CellID getNeighbourID(int neighbourIndex) const; // Returns ID of neighbour across the given face of the cell
		
		/* Iterator methods: */
		friend bool operator==(const Cell& cell1,const Cell& cell2)
			{
			return cell1.baseVertexIndex==cell2.baseVertexIndex;
			}
		friend bool operator!=(const Cell& cell1,const Cell& cell2)
			{
			return cell1.baseVertexIndex!=cell2.baseVertexIndex;
			}
		Cell& operator++(void) // Pre-increment operator
			{
			index.preInc(ds->grids[gridIndex].numCells);
			if(index[0]==ds->grids[gridIndex].numCells[0])
				{
				++gridIndex;
				index[0]=0;
				if(gridIndex<ds->numGrids)
					baseVertexIndex=ds->grids[gridIndex].getVertexLinearIndex(index);
				else
					baseVertexIndex=-1;
				}
			else
				baseVertexIndex=ds->grids[gridIndex].getVertexLinearIndex(index);
			return *this;
			}
		};
	
	typedef IteratorWrapper<Cell> CellIterator; // Class to iterate through cells
	
	class Locator:private Cell // Class responsible for evaluating a data set at a given position
		{
		friend class SlicedMultiCurvilinear;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		using Cell::ds;
		using Cell::gridIndex;
		using Cell::index;
		using Cell::baseVertexIndex;
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		Scalar epsilon,epsilon2; // Accuracy threshold of point location algorithm
		bool canTrace; // Flag if the locator can trace on the next locatePoint call
		
		/* Private methods: */
		bool newtonRaphsonStep(const Point& position); // Performs one Newton-Raphson step while tracing the given position
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		private:
		Locator(const SlicedMultiCurvilinear* sDs,Scalar sEpsilon); // Creates non-localized locator associated with given data set
		
		/* Methods: */
		public:
		void setEpsilon(Scalar newEpsilon); // Sets a new accuracy threshold in local cell dimension
		CellID getCellID(void) const // Returns the ID of the cell containing the last located point
			{
			return Cell::getID();
			}
		bool locatePoint(const Point& position,bool traceHint =false); // Sets locator to given position; returns true if position is inside found cell
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue calcValue(const ValueExtractorParam& extractor) const; // Calculates value at last located position
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const; // Calculates gradient at last located position
		};
	
	private:
	typedef Geometry::ValuedPoint<Point,CellID> CellCenter; // Data type to associate a cell's center point and its ID
	typedef Geometry::ArrayKdTree<CellCenter> CellCenterTree; // Data type for kd-trees to locate closest cell centers
	
	friend class Vertex;
	friend class Cell;
	friend class Locator;
	
	/* Elements: */
	private:
	int numGrids; // Number of grids defining the data set
	Grid* grids; // Array of grids
	size_t totalNumVertices; // Total number of vertices in all grids
	size_t totalNumCells; // Total number of cells in all grids
	int numSlices; // Number of scalar value slices in data set
	ValueScalar** slices; // Array of 1D arrays defining data set's value slices
	CellCenterTree cellCenterTree; // Kd-tree containing cell centers of all grids
	CellID** gridConnectors; // Arrays mapping outer faces of all grids to stitched grid cells
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	Scalar avgCellRadius; // Average "radius" of all cells
	Scalar maxCellRadius2; // Squared maximum "radius" of any cell in any grid (used as trivial reject threshold during point location)
	Scalar locatorEpsilon; // Default accuracy threshold for locators working on this data set
	
	/* Private methods: */
	template <class ScalarExtractorParam>
	Vector calcVertexGradient(int gridIndex,const Index& vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at a vertex based on the given scalar extractor
	void storeGridConnector(const Cell& cell,int faceIndex,const CellID& otherCell); // Stores a connection between a cell face and another cell during grid finalization
	CellID retrieveGridConnector(const Cell& cell,int faceIndex) const; // Retrieves the ID of a cell connected to the given cell face
	
	/* Constructors and destructors: */
	public:
	SlicedMultiCurvilinear(void); // Creates an "empty" data set
	SlicedMultiCurvilinear(int sNumGrids); // Creates a data set with the given number of grids and data value slices
	SlicedMultiCurvilinear(int sNumGrids,const Index sNumGridVertices[],int sNumSlices); // Creates a data set with the given number of grids, vertices per grid, and data value slices
	~SlicedMultiCurvilinear(void);
	
	/* Data set construction methods: */
	void setNumGrids(int sNumGrids); // Creates a data set with the given number of grids
	void setGrid(int gridIndex,const Index& sNumVertices,const Point* sVertexPositions =0); // Creates a grid with the given number of vertices; copies vertex positions if pointer is not null
	int addGrid(const Index& sNumVertices,const Point* sVertexPositions =0); // Adds another grid with the given number of vertices; copies vertex positions if pointer is not null; returns index of new grid
	int addSlice(const ValueScalar* sSliceValues =0); // Adds another slice to the data set; copies slice values for all points in all grids from given array if pointer is not null; returns index of new slice
	
	/* Low-level data access methods: */
	const int getNumGrids(void) const // Returns number of grids in the data set
		{
		return numGrids;
		}
	const Grid& getGrid(int gridIndex) const // Returns one grid
		{
		return grids[gridIndex];
		}
	Grid& getGrid(int gridIndex) // Ditto
		{
		return grids[gridIndex];
		}
	int getNumSlices(void) const // Returns the number of value slices in the data set
		{
		return numSlices;
		}
	const ValueScalar* getSliceArray(int sliceIndex,int gridIndex) const // Returns one data slice for one of the grids
		{
		return slices[sliceIndex]+grids[gridIndex].gridBaseLinearIndex;
		}
	ValueScalar* getSliceArray(int sliceIndex,int gridIndex) // Ditto
		{
		return slices[sliceIndex]+grids[gridIndex].gridBaseLinearIndex;
		}
	const ValueScalar* getSliceArray(int sliceIndex) const // Returns one data slice for the entire data set
		{
		return slices[sliceIndex];
		}
	ValueScalar* getSliceArray(int sliceIndex) // Ditto
		{
		return slices[sliceIndex];
		}
	ValueScalar getVertexValue(int sliceIndex,int gridIndex,const Index& vertexIndex) const // Returns a vertex' data value from one slice
		{
		return slices[sliceIndex][grids[gridIndex].getVertexLinearIndex(vertexIndex)];
		}
	ValueScalar& getVertexValue(int sliceIndex,int gridIndex,const Index& vertexIndex) // Ditto
		{
		return slices[sliceIndex][grids[gridIndex].getVertexLinearIndex(vertexIndex)];
		}
	void finalizeGrid(void); // Recalculates derived grid information after grid structure change
	Scalar getLocatorEpsilon(void) const // Returns the current default accuracy threshold for locators working on this data set
		{
		return locatorEpsilon;
		}
	void setLocatorEpsilon(Scalar newLocatorEpsilon); // Sets the default accuracy threshold for locators working on this data set
	bool isBoundaryFace(int gridIndex,int faceIndex) const; // Returns true if the given face of the given grid is entirely on the boundary of the data set
	bool isInteriorFace(int gridIndex,int faceIndex) const; // Returns true if the given face of the given grid is entirely in the interior of the data set
	
	/* Methods implementing the data set interface: */
	size_t getTotalNumVertices(void) const // Returns total number of vertices in the data set
		{
		return totalNumVertices;
		}
	Vertex getVertex(const VertexID& vertexID) const // Returns vertex of given valid ID
		{
		/* Find index of grid containing vertex: */
		int l=0;
		int r=numGrids;
		while(r-l>1)
			{
			int mid=(l+r)>>1;
			if(grids[mid].gridBaseLinearIndex<=vertexID.getIndex())
				l=mid;
			else
				r=mid;
			}
		
		/* Return index of vertex in grid: */
		return Vertex(this,l,grids[l].grid.calcIndex(vertexID.getIndex()-grids[l].gridBaseLinearIndex));
		}
	const VertexIterator& beginVertices(void) const // Returns iterator to first vertex in the data set
		{
		return firstVertex;
		}
	const VertexIterator& endVertices(void) const // Returns iterator behind last vertex in the data set
		{
		return lastVertex;
		}
	size_t getTotalNumCells(void) const // Returns total number of cells in the data set
		{
		return totalNumCells;
		}
	Cell getCell(const CellID& cellID) const // Return cell of given valid ID
		{
		/* Find index of grid containing cell: */
		int l=0;
		int r=numGrids;
		while(r-l>1)
			{
			int mid=(l+r)>>1;
			if((unsigned int)(grids[mid].gridBaseLinearIndex)<=cellID.getIndex())
				l=mid;
			else
				r=mid;
			}
		
		return Cell(this,l,grids[l].grid.calcIndex(cellID.getIndex()-grids[l].gridBaseLinearIndex));
		}
	const CellIterator& beginCells(void) const // Returns iterator to first cell in the data set
		{
		return firstCell;
		}
	const CellIterator& endCells(void) const // Returns iterator behind last cell in the data set
		{
		return lastCell;
		}
	const Box& getDomainBox(void) const // Returns bounding box of the data set's domain
		{
		return domainBox;
		}
	Scalar calcAverageCellSize(void) const // Calculates an estimate of the average cell size in the data set
		{
		return avgCellRadius*Scalar(2);
		}
	Locator getLocator(void) const // Returns an unlocalized locator for the data set
		{
		return Locator(this,locatorEpsilon);
		}
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDMULTICURVILINEAR_IMPLEMENTATION
#include <Templatized/SlicedMultiCurvilinear.icpp>
#endif

#endif
