/***********************************************************************
MultiCurvilinear - Base class for vertex-centered multi-block
curvilinear data sets containing arbitrary value types (scalars,
vectors, tensors, etc.).
Copyright (c) 2007-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_MULTICURVILINEAR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_MULTICURVILINEAR_INCLUDED

#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>

#include <Templatized/Tesseract.h>
#include <Templatized/LinearIndexID.h>
#include <Templatized/IteratorWrapper.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class DataSetParam>
class HypercubicLocator;
}
}

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class ValueParam>
class MultiCurvilinear
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
	typedef ValueParam Value; // Data set's value type
	
	/* Low-level definitions of data set storage: */
	struct GridVertex // Structure for valued grid vertices
		{
		/* Elements: */
		public:
		Point pos; // Position of grid vertex in data set's domain
		Value value; // Grid vertex' value
		
		/* Constructors and destructors: */
		GridVertex(void) // Dummy constructor
			{
			}
		GridVertex(const Point& sPos,const Value& sValue) // Elementwise constructor
			:pos(sPos),value(sValue)
			{
			}
		};
	
	typedef Misc::Array<GridVertex,dimensionParam> Array; // Array type for data set storage
	typedef typename Array::Index Index; // Index type for data set storage
	
	class Vertex;
	class Cell;
	class Locator;
	
	class Grid // Structure describing one grid
		{
		friend class MultiCurvilinear;
		friend class Vertex;
		friend class Cell;
		friend class Locator;
		
		/* Elements: */
		private:
		Index numVertices; // Number of vertices in grid in each dimension
		Array vertices; // Array of vertices defining grid
		int vertexStrides[dimension]; // Array of pointer stride values in the vertex array
		Index numCells; // Number of cells in data set in each dimension
		int vertexOffsets[CellTopology::numVertices]; // Array of pointer offsets from a cell's base vertex to all cell vertices
		
		/* Constructors and destructors: */
		private:
		Grid(void); // Creates an empty grid
		
		/* Methods: */
		void setNumVertices(const Index& sNumVertices); // Sets the grid's number of vertices
		public:
		const Index& getNumVertices(void) const // Returns number of vertices in the grid
			{
			return numVertices;
			}
		const Array& getVertices(void) const // Returns the vertices defining the grid
			{
			return vertices;
			}
		Array& getVertices(void) // Ditto
			{
			return vertices;
			}
		const GridVertex& getVertex(const Index& vertexIndex) const // Returns a grid vertex
			{
			return vertices(vertexIndex);
			}
		GridVertex& getVertex(const Index& vertexIndex) // Ditto
			{
			return vertices(vertexIndex);
			}
		const Point& getVertexPosition(const Index& vertexIndex) const // Returns a vertex' position
			{
			return vertices(vertexIndex).pos;
			}
		Point& getVertexPosition(const Index& vertexIndex) // Ditto
			{
			return vertices(vertexIndex).pos;
			}
		const Value& getVertexValue(const Index& vertexIndex) const // Returns a vertex' data value
			{
			return vertices(vertexIndex).value;
			}
		Value& getVertexValue(const Index& vertexIndex) // Ditto
			{
			return vertices(vertexIndex).value;
			}
		const Index& getNumCells(void) const // Returns number of cells in grid
			{
			return numCells;
			}
		};
	
	/* Data set interface classes: */
	typedef LinearIndexID VertexID;
	
	class Vertex // Class to represent and iterate through vertices
		{
		friend class MultiCurvilinear;
		
		/* Elements: */
		private:
		const MultiCurvilinear* ds; // Pointer to data set containing the vertex
		int gridIndex; // Index of the grid containing the vertex
		Index index; // Array index of vertex in grid's storage
		
		/* Constructors and destructors: */
		public:
		Vertex(void) // Creates an invalid vertex
			:ds(0)
			{
			}
		private:
		Vertex(const MultiCurvilinear* sDs,int sGridIndex,const Index& sIndex)
			:ds(sDs),gridIndex(sGridIndex),index(sIndex)
			{
			}
		
		/* Methods: */
		public:
		const Point& getPosition(void) const // Returns vertex' position in domain
			{
			return ds->grids[gridIndex].vertices(index).pos;
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getValue(const ValueExtractorParam& extractor) const // Returns vertex' value based on given extractor
			{
			return extractor.getValue(ds->grids[gridIndex].vertices(index).value);
			}
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const // Returns gradient at the vertex, based on given scalar extractor
			{
			return ds->calcVertexGradient(gridIndex,index,extractor);
			}
		VertexID getID(void) const // Returns vertex' ID
			{
			return VertexID(VertexID::Index(ds->grids[gridIndex].vertices.calcLinearIndex(index))+ds->vertexIDBases[gridIndex]);
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
		friend class HypercubicLocator<MultiCurvilinear>;
		friend class MultiCurvilinear;
		friend class Locator;
		
		/* Elements: */
		private:
		const MultiCurvilinear* ds; // Pointer to the data set containing the cell
		int gridIndex; // Index of grid containing the cell
		Index index; // Array index of cell's base vertex in data set storage
		const GridVertex* baseVertex; // Pointer to cell's base vertex in data set storage
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:ds(0),baseVertex(0)
			{
			}
		private:
		Cell(const MultiCurvilinear* sDs)
			:ds(sDs),baseVertex(0)
			{
			}
		Cell(const MultiCurvilinear* sDs,int sGridIndex,const Index& sIndex)
			:ds(sDs),gridIndex(sGridIndex),index(sIndex),baseVertex(ds->grids[gridIndex].vertices.getAddress(index))
			{
			}
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the cell is valid
			{
			return baseVertex!=0;
			}
		VertexID getVertexID(int vertexIndex) const; // Returns ID of given vertex of the cell
		Vertex getVertex(int vertexIndex) const; // Returns given vertex of the cell
		const Point& getVertexPosition(int vertexIndex) const // Returns position of given vertex of the cell
			{
			return baseVertex[ds->grids[gridIndex].vertexOffsets[vertexIndex]].pos;
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getVertexValue(int vertexIndex,const ValueExtractorParam& extractor) const // Returns value of given vertex of the cell, based on given extractor
			{
			return extractor.getValue(baseVertex[ds->grids[gridIndex].vertexOffsets[vertexIndex]].value);
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at given vertex of the cell, based on given scalar extractor
		EdgeID getEdgeID(int edgeIndex) const; // Returns ID of given edge of the cell
		Point calcEdgePosition(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		Point calcFaceCenter(int faceIndex) const; // Returns the center of the cell's given face
		CellID getID(void) const // Returns cell's ID
			{
			// TODO: Try which version is faster
			#if 1
			return CellID(CellID::Index(baseVertex-ds->grids[gridIndex].vertices.getArray())+ds->cellIDBases[gridIndex]);
			#else
			return CellID(CellID::Index(ds->grids[gridIndex].vertices.calcLinearIndex(index))+ds->cellIDBases[gridIndex]);
			#endif
			}
		CellID getNeighbourID(int neighbourIndex) const; // Returns ID of neighbour across the given face of the cell
		
		/* Iterator methods: */
		friend bool operator==(const Cell& cell1,const Cell& cell2)
			{
			return cell1.baseVertex==cell2.baseVertex;
			}
		friend bool operator!=(const Cell& cell1,const Cell& cell2)
			{
			return cell1.baseVertex!=cell2.baseVertex;
			}
		Cell& operator++(void) // Pre-increment operator
			{
			index.preInc(ds->grids[gridIndex].numCells);
			if(index[0]==ds->grids[gridIndex].numCells[0])
				{
				++gridIndex;
				index[0]=0;
				if(gridIndex<ds->numGrids)
					baseVertex=ds->grids[gridIndex].vertices.getAddress(index);
				else
					baseVertex=0;
				}
			else
				baseVertex=ds->grids[gridIndex].vertices.getAddress(index);
			return *this;
			}
		};
	
	typedef IteratorWrapper<Cell> CellIterator; // Class to iterate through cells
	
	class Locator:private Cell // Class responsible for evaluating a data set at a given position
		{
		friend class HypercubicLocator<MultiCurvilinear>;
		friend class MultiCurvilinear;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		using Cell::ds;
		using Cell::gridIndex;
		using Cell::index;
		using Cell::baseVertex;
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		Scalar epsilon,epsilon2; // Accuracy threshold of point location algorithm
		bool canTrace; // Flag if the locator can trace on the next locatePoint call
		
		/* Private methods: */
		bool traverse(int stepDimension,int stepDirection); // Moves the locator into a neighboring cell and estimates the new local cell position
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		private:
		Locator(const MultiCurvilinear* sDs,Scalar sEpsilon); // Creates non-localized locator associated with given data set
		
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
	VertexID::Index* vertexIDBases; // Bases of vertex IDs for each grid
	EdgeID::Index* edgeIDBases; // Bases of edge IDs for each grid
	CellID::Index* cellIDBases; // Bases of cell IDs for each grid
	CellID** gridConnectors; // Arrays mapping outer faces of all grids to stitched grid cells
	CellCenterTree cellCenterTree; // Kd-tree containing cell centers of all grids
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	Scalar avgCellRadius; // Average "radius" of all cells
	Scalar maxCellRadius2; // Squared maximum "radius" of any cell in any grid (used as trivial reject threshold during point location)
	Scalar locatorEpsilon; // Default accuracy threshold for locators working on this data set
	
	/* Private methods: */
	void initStructure(void);
	template <class ScalarExtractorParam>
	Vector calcVertexGradient(int gridIndex,const Index& vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at a vertex based on the given scalar extractor
	void storeGridConnector(const Cell& cell,int faceIndex,const CellID& otherCell); // Stores a connection between a cell face and another cell during grid finalization
	CellID retrieveGridConnector(const Cell& cell,int faceIndex) const; // Retrieves the ID of a cell connected to the given cell face
	
	/* Constructors and destructors: */
	public:
	MultiCurvilinear(void); // Creates an "empty" data set
	MultiCurvilinear(int sNumGrids); // Creates a data set with the given number of grids
	MultiCurvilinear(int sNumGrids,const Index sNumGridVertices[]); // Creates a data set with the given number of grids and vertices per grid
	~MultiCurvilinear(void);
	
	/* Data set construction methods: */
	void setGrids(int sNumGrids); // Creates a data set with the given number of grids
	void setGridData(int gridIndex,const Index& sNumVertices,const Point* sVertexPositions =0,const Value* sVertexValues =0); // Creates a grid with the given number of vertices; copies vertex positions and values if pointers are not null
	void setGridData(int gridIndex,const Index& sNumVertices,const GridVertex* sVertices); // Creates a data set with the given number of vertices and vertices
	
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
	void finalizeGrid(void); // Recalculates derived grid information after grid structure change
	CellID findClosestCell(const Point& position) const; // Finds the cell whose center is closest to the given position, or an invalid ID if there is no close cell
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
			if(vertexIDBases[mid]<=vertexID.getIndex())
				l=mid;
			else
				r=mid;
			}
		
		/* Return index of vertex in grid: */
		return Vertex(this,l,grids[l].vertices.calcIndex(vertexID.getIndex()-vertexIDBases[l]));
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
			if(cellIDBases[mid]<=cellID.getIndex())
				l=mid;
			else
				r=mid;
			}
		
		return Cell(this,l,grids[l].vertices.calcIndex(cellID.getIndex()-cellIDBases[l]));
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

#ifndef VISUALIZATION_TEMPLATIZED_MULTICURVILINEAR_IMPLEMENTATION
#include <Templatized/MultiCurvilinear.icpp>
#endif

#endif
