/***********************************************************************
SlicedCartesian - Base class for vertex-centered cartesian data sets
containing multiple scalar-valued slices.
Copyright (c) 2006-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDCARTESIAN_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEDCARTESIAN_INCLUDED

#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>

#include <Templatized/Tesseract.h>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class VScalarParam>
class SlicedCartesian
	{
	/* Embedded classes: */
	public:
	
	/* Definition of the data set's domain space: */
	typedef ScalarParam Scalar; // Scalar type of data set's domain
	static const int dimension=dimensionParam; // Dimension of data set's domain
	typedef Geometry::Point<Scalar,dimensionParam> Point; // Type for points in data set's domain
	typedef Geometry::Vector<Scalar,dimensionParam> Vector; // Type for vectors in data set's domain
	typedef Geometry::ComponentArray<Scalar,dimensionParam> Size; // Type for sizes in data set's domain
	typedef Geometry::Box<Scalar,dimensionParam> Box; // Type for axis-aligned boxes in data set's domain
	
	/* Definition of the data set's cell topology: */
	typedef Tesseract<dimensionParam> CellTopology; // Policy class to select appropriate cell algorithms
	
	/* Definition of the data set's value space: */
	typedef VScalarParam VScalar; // Data set's value type
	
	/* Low-level definitions of data set storage: */
	typedef Misc::Array<VScalar,dimensionParam> Array; // Array type for data set storage
	typedef typename Array::Index Index; // Index type for data set storage
	
	/* Data set interface classes: */
	class VertexIterator;
	class CellEdge;
	class Cell;
	class CellIterator;
	class Locator;
	
	class VertexIterator // Class to iterate through all vertices in a data set
		{
		friend class SlicedCartesian;
		
		/* Elements: */
		private:
		const SlicedCartesian* grid; // Data set containing the vertex pointed to by the iterator
		Index index; // Index of vertex pointed to by the iterator
		
		/* Constructors and destructors: */
		public:
		VertexIterator(void) // Creates invalid iterator
			:grid(0)
			{
			}
		private:
		VertexIterator(const SlicedCartesian* sGrid,const Index& sIndex) // Creates iterator to given vertex
			:grid(sGrid),index(sIndex)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const VertexIterator& vi1,const VertexIterator& vi2) // Compares two vertex iterators for equality
			{
			return vi1.index==vi2.index&&vi1.grid==vi2.grid;
			}
		friend bool operator!=(const VertexIterator& vi1,const VertexIterator& vi2) // Compares two vertex iterators for inequality
			{
			return vi1.index!=vi2.index||vi1.grid!=vi2.grid;
			}
		Point getVertexPos(void) const; // Returns position of vertex pointed to by the iterator
		VScalar getVertexValue(const Array& slice) const // Returns per-slice value of vertex pointed to by the iterator
			{
			return slice(index);
			}
		VertexIterator& operator++(void) // Pre-increment operator
			{
			index.preInc(grid->numVertices);
			return *this;
			}
		};
	
	class CellEdge // Class to represent edges of cells in a data set
		{
		friend class Cell;
		
		/* Elements: */
		private:
		const Value* baseVertex; // Base vertex of the edge
		int edgeDirection; // Direction of the edge
		
		/* Constructors: */
		private:
		CellEdge(const Value* sBaseVertex,int sEdgeDirection) // Elementwise constructor
			:baseVertex(sBaseVertex),edgeDirection(sEdgeDirection)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const CellEdge& ce1,const CellEdge& ce2) // Compares two cell edges for equality
			{
			return ce1.baseVertex==ce2.baseVertex&&ce1.edgeDirection==ce2.edgeDirection;
			}
		friend bool operator!=(const CellEdge& ce1,const CellEdge& ce2) // Compares two cell edges for inequality
			{
			return ce1.baseVertex!=ce2.baseVertex||ce1.edgeDirection!=ce2.edgeDirection;
			}
		static size_t hash(const CellEdge& source,size_t tableSize) // Calculates hash index for a cell edge
			{
			return (reinterpret_cast<size_t>(source.baseVertex)*dimension+source.edgeDirection)%tableSize;
			}
		};
	
	class Cell // Class to represent individual cells in a data set
		{
		friend class SlicedCartesian;
		friend class CellIterator;
		friend class Locator;
		
		/* Elements: */
		private:
		const SlicedCartesian* grid; // Data set containing the cell
		Index index; // Index of the cell
		ptrdiff_t baseVertexIndex; // Index of base vertex of the cell
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:grid(0),baseVertexIndex(-1)
			{
			}
		private:
		Cell(const SlicedCartesian* sGrid) // Creates an invalid cell in the given data set
			:grid(sGrid),baseVertexIndex(-1)
			{
			}
		Cell(const SlicedCartesian* sGrid,const Index& sIndex) // Elementwise constructor
			:grid(sGrid),index(sIndex),baseVertexIndex(grid->slice[0].calcLinearIndex(index))
			{
			}
		
		/* Methods: */
		public:
		const SlicedCartesian* getGrid(void) const // Returns the data set containing the cell
			{
			return grid;
			}
		bool isValid(void) const // Returns true if the cell is valid
			{
			return baseVertexIndex>=0;
			}
		friend bool operator==(const Cell& cell1,const Cell& cell2) // Compares two cells for equality
			{
			return cell1.baseVertexIndex==cell2.baseVertexIndex;
			}
		friend bool operator!=(const Cell& cell1,const Cell& cell2) // Compares two cells for inequality
			{
			return cell1.baseVertexIndex!=cell2.baseVertexIndex;
			}
		static size_t hash(const Cell& source,size_t tableSize) // Calculates hash index for a cell
			{
			return size_t(source.baseVertexIndex)%tableSize;
			}
		Point getVertexPos(int vertexIndex) const; // Returns position of given vertex of the cell
		VScalar getVertexValue(int vertexIndex,const Array& slice) const // Returns value of given vertex of the cell
			{
			return slice.getArray()[baseVertexIndex+grid->vertexOffsets[vertexIndex]];
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const Aray& slice) const; // Returns gradient at given vertex of the cell, based on given scalar slice
		CellEdge getEdge(int edgeIndex) const; // Returns an edge of the cell
		Point getEdgePos(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		Cell getNeighbour(int neighbourIndex) const; // Returns a face neighbour of the cell
		};
	
	class CellIterator // Class to iterate through all cells in a data set
		{
		friend class SlicedCartesian;
		
		/* Elements: */
		private:
		Cell cell; // Cell pointed to by the iterator
		
		/* Constructors and destructors: */
		public:
		CellIterator(void) // Creates invalid iterator
			{
			}
		private:
		CellIterator(const SlicedCartesian* sGrid,const Index& sIndex) // Creates iterator to given cell
			:cell(sGrid,sIndex)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const CellIterator& ci1,const CellIterator& ci2) // Compares two cell iterators for equality
			{
			return ci1.cell==ci2.cell;
			}
		friend bool operator!=(const CellIterator& ci1,const CellIterator& ci2) // Compares two cell iterators for inequality
			{
			return ci1.cell!=ci2.cell;
			}
		const Cell& operator*(void) const // Returns cell pointed to by the iterator
			{
			return cell;
			}
		const Cell* operator->(void) const // Returns cell pointed to by the iterator
			{
			return &cell;
			}
		CellIterator& operator++(void) // Pre-increment operator
			{
			cell.index.preInc(cell.grid->numCells);
			cell.baseVertexIndex=cell.grid->slice[0].calcLinearIndex(cell.index);
			return *this;
			}
		};
	
	class Locator // Class responsible for evaluating a data set at a given position
		{
		friend class SlicedCartesian;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		Cell cell; // Cell containing the last located point
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		private:
		Locator(const SlicedCartesian* sGrid); // Creates non-localized locator associated with given data set
		
		/* Methods: */
		public:
		void setEpsilon(Scalar newEpsilon) // Sets a new accuracy threshold in local cell dimension
			{
			/* Not needed for cartesian data sets */
			}
		bool isValid(void) const // Returns true if the locator points to a valid cell
			{
			return cell.isValid();
			}
		const Cell& getCell(void) const // Returns the cell containing the last located point
			{
			return cell;
			}
		bool locatePoint(const Point& position,bool traceHint =false); // Sets locator to given position; returns true if position is inside found cell
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue calcValue(const ValueExtractorParam& extractor) const; // Calculates value at last located position
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const; // Calculates gradient at last located position
		};
	
	friend class Cell;
	friend class Locator;
	
	/* Elements: */
	private:
	Index numVertices; // Number of vertices in data set in each dimension
	int numSlices; // Number of scalar value slices in the data set
	Array* slices; // Array of vertex values defining data set for each slice
	int vertexStrides[dimension]; // Array of pointer stride values in the vertex array
	Index numCells; // Number of cells in data set in each dimension
	int vertexOffsets[CellTopology::numVertices]; // Array of pointer offsets from a cell's base vertex to all cell vertices
	Size cellSize; // Size of the data set's cells in each dimension
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	
	/* Private methods: */
	Vector calcVertexGradient(const Index& vertexIndex,const Array& slice) const; // Returns gradient at a vertex based on the given value slice
	
	/* Constructors and destructors: */
	public:
	SlicedCartesian(void); // Creates an "empty" data set
	SlicedCartesian(const Index& sNumVertices,int sNumSlices,const Size& sCellSize,const VScalar* sVertexValues =0); // Creates a data set of the given number of vertices and cell size; copies slice-major vertex data if pointer is not null
	~SlicedCartesian(void); // Destroys the data set
	
	/* Data set construction methods: */
	void setData(const Index& sNumVertices,int sNumSlices,const Size& sCellSize,const Value* sVertexValues =0); // Sets the number of vertices and cell size of the data set; copies slice-major vertex data if pointer is not null
	
	/* Low-level data access methods: */
	const Index& getNumVertices(void) const // Returns number of vertices in the data set
		{
		return numVertices;
		}
	int getNumSlices(void) const
		{
		return numSlices;
		}
	const Array& getVertices(int sliceIndex) const // Returns the vertices defining the data set for the given slice
		{
		return slices[sliceIndex];
		}
	Array& getVertices(int sliceIndex) // Ditto
		{
		return slices[sliceIndex];
		}
	Point getVertexPos(const Index& vertexIndex) const; // Returns a vertex' position
	VScalar getVertexValue(const Index& vertexIndex,int sliceIndex) const // Returns a vertex' data value inside a slice
		{
		return slices[sliceIndex](vertexIndex).value;
		}
	VScalar& getVertexValue(const Index& vertexIndex,int sliceIndex) // Ditto
		{
		return slices[sliceIndex](vertexIndex).value;
		}
	const Index& getNumCells(void) const // Returns number of cells in grid
		{
		return numCells;
		}
	const Size& getCellSize(void) const // Returns size of a single cell
		{
		return cellSize;
		}
	
	/* Methods implementing the data set interface: */
	size_t getTotalNumVertices(void) const // Returns total number of vertices in the data set
		{
		return numVertices.calcIncrement(-1);
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
		return numCells.calcIncrement(-1);
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
	Locator getLocator(void) const // Returns an unlocalized locator for the data set
		{
		return Locator(this);
		}
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SLICEDCARTESIAN_IMPLEMENTATION
#include <Templatized/SlicedCartesian.cpp>
#endif

#endif
