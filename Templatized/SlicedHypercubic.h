/***********************************************************************
SlicedHypercubic - Base class for vertex-centered unstructured
hypercubic data sets containing multiple scalar-valued slices.
etc.).
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDHYPERCUBIC_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEDHYPERCUBIC_INCLUDED

#include <utility>
#include <vector>
#include <Misc/UnorderedTuple.h>
#include <Misc/HashTable.h>
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
class SlicedHypercubic
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
	typedef ValueScalarParam ValueScalar; // Data set's value type
	typedef SlicedDataValue<ValueScalar> Value; // Data set's compound value type
	
	/* First batch of data set interface classes: */
	typedef LinearIndexID VertexID; // ID type for vertices
	typedef VertexID::Index VertexIndex; // Index type for vertices
	typedef Misc::UnorderedTuple<VertexIndex,2> EdgeID; // ID type for cell edges
	typedef LinearIndexID CellID; // ID type for cells
	typedef CellID::Index CellIndex; // Index type for cells
	
	/* Low-level definitions of data set storage: */
	private:
	typedef Point GridVertex; // Grid vertices are just points
	typedef std::vector<GridVertex> GridVertexList; // Type to store the list of grid vertices
	
	struct GridCell // Structure for grid cells
		{
		/* Elements: */
		public:
		VertexIndex vertices[CellTopology::numVertices]; // Array of indices of cell's vertices
		CellIndex neighbours[CellTopology::numFaces]; // Array of indices of cell's neighbouring cells
		
		/* Constructors and destructors: */
		GridCell(void); // Creates a nonconnected grid cell with uninitialized vertex indices
		};
	
	typedef std::vector<GridCell> GridCellList; // Type to store the list of grid cells
	typedef Misc::UnorderedTuple<VertexIndex,CellTopology::numFaceVertices> GridFace; // Type to identify faces of grid cells
	typedef Misc::HashTable<GridFace,std::pair<CellIndex,int>,GridFace> GridFaceHasher; // Data type from grid faces to grid cell and cell face indices; used during data set construction
	
	/* Data set interface classes: */
	public:
	class Cell;
	
	class Vertex // Class to represent and iterate through vertices
		{
		friend class SlicedHypercubic;
		friend class Cell;
		
		/* Elements: */
		private:
		const SlicedHypercubic* ds; // Pointer to data set containing the vertex
		VertexIndex index; // Index of vertex in vertex list
		
		/* Constructors and destructors: */
		public:
		Vertex(void) // Creates an invalid vertex
			:ds(0),index(~VertexIndex(0))
			{
			}
		private:
		Vertex(const SlicedHypercubic* sDs,VertexIndex sIndex)
			:ds(sDs),index(sIndex)
			{
			}
		
		/* Methods: */
		public:
		const Point& getPosition(void) const // Returns vertex' position in domain
			{
			return ds->gridVertices[index];
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getValue(const ValueExtractorParam& extractor) const // Returns vertex' value based on given extractor
			{
			return extractor.getValue(index);
			}
		VertexID getID(void) const // Returns vertex' ID
			{
			return VertexID(index);
			}
		
		/* Iterator methods: */
		friend bool operator==(const Vertex& v1,const Vertex& v2)
			{
			return v1.index==v2.index&&v1.ds==v2.ds;
			}
		friend bool operator!=(const Vertex& v1,const Vertex& v2)
			{
			return v1.index!=v2.index||v1.ds!=v2.ds;
			}
		Vertex& operator++(void) // Pre-increment operator
			{
			++index;
			return *this;
			}
		};
	
	typedef IteratorWrapper<Vertex> VertexIterator; // Class to iterate through vertices
	class Locator;
	
	class Cell // Class to represent and iterate through cells
		{
		friend class SlicedHypercubic;
		friend class Locator;
		
		/* Elements: */
		private:
		const SlicedHypercubic* ds; // Pointer to data set containing the cell
		CellIndex index; // Index of cell in cell list
		const GridCell* cell; // Direct pointer to cell
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:ds(0),index(~CellIndex(0)),cell(0)
			{
			}
		private:
		Cell(const SlicedHypercubic* sDs) // Creates an invalid cell for the given data set
			:ds(sDs),index(~CellIndex(0)),cell(0)
			{
			}
		Cell(const SlicedHypercubic* sDs,CellIndex sIndex) // Elementwise constructor
			:ds(sDs),index(sIndex),cell(index!=(~CellIndex(0))?&ds->gridCells[index]:0)
			{
			}
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the cell is valid
			{
			return cell!=0;
			}
		VertexID getVertexID(int vertexIndex) const // Returns ID of given vertex of the cell
			{
			return VertexID(cell->vertices[vertexIndex]);
			}
		Vertex getVertex(int vertexIndex) const // Returns given vertex of the cell
			{
			return Vertex(ds,cell->vertices[vertexIndex]);
			}
		const Point& getVertexPosition(int vertexIndex) const // Returns position of given vertex of the cell
			{
			return ds->gridVertices[cell->vertices[vertexIndex]];
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getVertexValue(int vertexIndex,const ValueExtractorParam& extractor) const // Returns value of given vertex of the cell, based on given extractor
			{
			return extractor.getValue(cell->vertices[vertexIndex]);
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at given vertex of the cell, based on given scalar extractor
		EdgeID getEdgeID(int edgeIndex) const // Returns ID of given edge of the cell
			{
			return EdgeID(cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][0]],cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][1]]);
			}
		Point calcEdgePosition(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		CellID getID(void) const // Returns cell's ID
			{
			return CellID(index);
			}
		CellID getNeighbourID(int neighbourIndex) const // Returns ID of neighbour across the given face of the cell
			{
			return CellID(cell->neighbours[neighbourIndex]);
			}
		
		/* Iterator methods: */
		friend bool operator==(const Cell& cell1,const Cell& cell2) // Compares two cells for equality
			{
			return cell1.cell==cell2.cell;
			}
		friend bool operator!=(const Cell& cell1,const Cell& cell2) // Compares two cells for inequality
			{
			return cell1.cell!=cell2.cell;
			}
		Cell& operator++(void) // Pre-increment operator
			{
			++index;
			++cell;
			return *this;
			}
		};
	
	typedef IteratorWrapper<Cell> CellIterator; // Class to iterate through cells
	
	class Locator:private Cell // Class responsible for evaluating a data set at a given position
		{
		friend class SlicedHypercubic;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		using Cell::ds;
		using Cell::index;
		using Cell::cell;
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		Scalar epsilon,epsilon2; // Accuracy threshold of point location algorithm
		bool cantTrace; // Flag if the locator cannot trace from the current state
		
		/* Private methods: */
		bool newtonRaphsonStep(const Point& position); // Performs one Newton-Raphson step while tracing the given position
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		private:
		Locator(const SlicedHypercubic* sDs,Scalar sEpsilon); // Creates non-localized locator associated with given data set
		
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
	GridVertexList gridVertices; // List of all grid vertices
	GridCellList gridCells; // List of all grid cells
	int numSlices; // Number of scalar value slices in data set
	size_t allocatedSliceSize; // Allocated size of all slice arrays
	ValueScalar** slices; // Array of 1D arrays defining data set's value slices
	CellCenterTree cellCenterTree; // Kd-tree containing cell centers
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	Scalar avgCellRadius; // Average "radius" of all cells
	Scalar maxCellRadius2; // Squared maximum "radius" of any cell (used as trivial reject threshold during point location)
	Scalar locatorEpsilon; // Default accuracy threshold for locators working on this data set
	GridFaceHasher* gridFaces; // Pointer to grid face hasher used during data set construction to connect grid cells
	
	/* Private methods: */
	void resizeSlices(size_t newAllocatedSize); // Resizes all existing value slices
	
	/* Constructors and destructors: */
	public:
	SlicedHypercubic(void); // Creates an "empty" hypercubic data set
	~SlicedHypercubic(void); // Destroys the data set
	
	/* Data set construction methods: */
	void reserveVertices(size_t numVertices); // Prepares the data set for subsequent addition of the given number of grid vertices (optional performance boost)
	void reserveCells(size_t numCells); // Prepares the data set for subsequent addition of the given number of grid cells (optional performance boost)
	VertexID addVertex(const Point& vertexPosition); // Adds a vertex to the grid; returns vertex' ID
	CellID addCell(const VertexID cellVertices[CellTopology::numVertices]); // Adds a cell to the grid; returns cell's ID
	int addSlice(const ValueScalar* sSliceValues =0); // Adds another slice to the data set; copies slice values for all points in all grids from given array if pointer is not null; returns index of new slice
	
	/* Low-level data access methods: */
	const Point& getVertexPosition(VertexIndex vertexIndex) const // Returns position of a vertex
		{
		return gridVertices[vertexIndex];
		}
	Point& getVertexPosition(VertexIndex vertexIndex) // Ditto
		{
		return gridVertices[vertexIndex];
		}
	int getNumSlices(void) const // Returns the number of value slices in the data set
		{
		return numSlices;
		}
	const ValueScalar* getSliceArray(int sliceIndex) const // Returns one of the data set's value slices
		{
		return slices[sliceIndex];
		}
	ValueScalar* getSliceArray(int sliceIndex) // Ditto
		{
		return slices[sliceIndex];
		}
	ValueScalar getVertexValue(int sliceIndex,VertexIndex vertexIndex) const // Returns a vertex' data value from one slice
		{
		return slices[sliceIndex][vertexIndex];
		}
	void setVertexValue(int sliceIndex,VertexIndex vertexIndex,ValueScalar newValue); // Sets the given vertex' value in the given slice
	void finalizeGrid(void); // Recalculates derived grid information after grid structure change
	Scalar getLocatorEpsilon(void) const // Returns the current default accuracy threshold for locators working on this data set
		{
		return locatorEpsilon;
		}
	void setLocatorEpsilon(Scalar newLocatorEpsilon); // Sets the default accuracy threshold for locators working on this data set
	
	/* Methods implementing the data set interface: */
	size_t getTotalNumVertices(void) const // Returns total number of vertices in the data set
		{
		return gridVertices.size();
		}
	Vertex getVertex(const VertexID& vertexID) const // Returns vertex of given valid ID
		{
		return Vertex(this,vertexID.getIndex());
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
		return gridCells.size();
		}
	Cell getCell(const CellID& cellID) const // Return cell of given valid ID
		{
		return Cell(this,cellID.getIndex());
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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDHYPERCUBIC_IMPLEMENTATION
#include <Templatized/SlicedHypercubic.icpp>
#endif

#endif
