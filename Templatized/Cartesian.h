/***********************************************************************
Cartesian - Base class for vertex-centered cartesian data sets
containing arbitrary value types (scalars, vectors, tensors, etc.).
Copyright (c) 2005-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_CARTESIAN_INCLUDED
#define VISUALIZATION_TEMPLATIZED_CARTESIAN_INCLUDED

#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>

#include <Templatized/Tesseract.h>
#include <Templatized/LinearIndexID.h>
#include <Templatized/IteratorWrapper.h>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class ValueParam>
class Cartesian
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
	typedef ValueParam Value; // Data set's value type
	
	/* Low-level definitions of data set storage: */
	typedef Misc::Array<Value,dimensionParam> Array; // Array type for data set storage
	typedef typename Array::Index Index; // Index type for data set storage
	
	/* Data set interface classes: */
	typedef LinearIndexID VertexID; // Class to identify vertices
	
	class Vertex // Class to represent and iterate through vertices
		{
		friend class Cartesian;
		
		/* Elements: */
		private:
		const Cartesian* ds; // Pointer to data set containing the vertex
		Index index; // Array index of vertex in data set storage
		
		/* Constructors and destructors: */
		public:
		Vertex(void) // Creates an invalid vertex
			:ds(0)
			{
			}
		private:
		Vertex(const Cartesian* sDs,const Index& sIndex)
			:ds(sDs),index(sIndex)
			{
			}
		
		/* Methods: */
		public:
		Point getPosition(void) const; // Returns vertex' position in domain
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getValue(const ValueExtractorParam& extractor) const // Returns vertex' value based on given extractor
			{
			return extractor.getValue(ds->vertices(index));
			}
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const // Returns gradient at the vertex, based on given scalar extractor
			{
			return ds->calcVertexGradient(index,extractor);
			}
		VertexID getID(void) const // Returns vertex' ID
			{
			return VertexID(VertexID::Index(ds->vertices.calcLinearIndex(index)));
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
			index.preInc(ds->numVertices);
			return *this;
			}
		};
	
	typedef IteratorWrapper<Vertex> VertexIterator; // Class to iterate through vertices
	
	typedef LinearIndexID EdgeID; // Class to identify cell edges
	
	typedef LinearIndexID CellID; // Class to identify cells
	
	class Locator;
	
	class Cell // Class to represent and iterate through cells
		{
		friend class Cartesian;
		friend class Locator;
		
		/* Elements: */
		private:
		const Cartesian* ds; // Pointer to the data set containing the cell
		Index index; // Array index of cell's base vertex in data set storage
		const Value* baseVertex; // Pointer to cell's base vertex in data set storage
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:ds(0),baseVertex(0)
			{
			}
		private:
		Cell(const Cartesian* sDs)
			:ds(sDs),baseVertex(0)
			{
			}
		Cell(const Cartesian* sDs,const Index& sIndex)
			:ds(sDs),index(sIndex),baseVertex(ds->vertices.getAddress(index))
			{
			}
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the cell is valid
			{
			return baseVertex!=0;
			}
		VertexID getVertexID(int vertexIndex) const; // Returns ID of given vertex of the cell
		Vertex getVertex(int vertexIndex) const; // Returns the given vertex of the cell
		Point getVertexPosition(int vertexIndex) const; // Returns position of given vertex of the cell
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getVertexValue(int vertexIndex,const ValueExtractorParam& extractor) const // Returns value of given vertex of the cell, based on given extractor
			{
			return extractor.getValue(baseVertex[ds->vertexOffsets[vertexIndex]]);
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at given vertex of the cell, based on given scalar extractor
		EdgeID getEdgeID(int edgeIndex) const; // Returns ID of given edge of the cell
		Point calcEdgePosition(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		CellID getID(void) const // Returns cell's ID
			{
			// TODO: Try which version is faster
			#if 1
			return CellID(CellID::Index(baseVertex-ds->vertices.getArray()));
			#else
			return CellID(CellID::Index(ds->vertices.calcLinearIndex(index)));
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
			index.preInc(ds->numCells);
			baseVertex=ds->vertices.getAddress(index);
			return *this;
			}
		};
	
	typedef IteratorWrapper<Cell> CellIterator; // Class to iterate through cells
	
	class Locator:private Cell // Class responsible for evaluating a data set at a given position
		{
		friend class Cartesian;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		using Cell::ds;
		using Cell::index;
		using Cell::baseVertex;
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		private:
		Locator(const Cartesian* sDs); // Creates non-localized locator associated with given data set
		
		/* Methods: */
		public:
		void setEpsilon(Scalar newEpsilon) // Sets a new accuracy threshold in local cell dimension
			{
			/* Not needed for Cartesian data sets */
			}
		CellID getCellID(void) const // Returns the ID of the cell containing the last located point
			{
			return Cell::getID();
			}
		bool locatePoint(const Point& position,bool traceHint =false); // Sets locator to given position; returns true if position is inside found cell
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue calcValue(const ValueExtractorParam& extractor) const; // Calculates value at last located position, based on given value extractor
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const; // Calculates gradient at last located position, based on given scalar extractor
		};
	
	friend class Vertex;
	friend class Cell;
	friend class Locator;
	
	/* Elements: */
	private:
	Index numVertices; // Number of vertices in data set in each dimension
	Array vertices; // Array of vertices defining data set
	int vertexStrides[dimension]; // Array of pointer stride values in the vertex array
	Index numCells; // Number of cells in data set in each dimension
	int vertexOffsets[CellTopology::numVertices]; // Array of pointer offsets from a cell's base vertex to all cell vertices
	Size cellSize; // Size of the data set's cells in each dimension
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	
	template <class ScalarExtractorParam>
	Vector calcVertexGradient(const Index& vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at a vertex based on the given scalar extractor
	
	/* Constructors and destructors: */
	public:
	Cartesian(void); // Creates an "empty" data set
	Cartesian(const Index& sNumVertices,const Size& sCellSize,const Value* sVertexValues =0); // Creates a data set of the given number of vertices and cell size; copies vertex data if pointer is not null
	~Cartesian(void); // Destroys the data set
	
	/* Data set construction methods: */
	void setData(const Index& sNumVertices,const Size& sCellSize,const Value* sVertexValues =0); // Sets the number of vertices and cell size of the data set; copies vertex data if pointer is not null
	
	/* Low-level data access methods: */
	const Index& getNumVertices(void) const // Returns number of vertices in the data set
		{
		return numVertices;
		}
	const Array& getVertices(void) const // Returns the vertices defining the data set
		{
		return vertices;
		}
	Array& getVertices(void) // Ditto
		{
		return vertices;
		}
	Point getVertexPosition(const Index& vertexIndex) const; // Returns a vertex' position
	const Value& getVertexValue(const Index& vertexIndex) const // Returns a vertex' data value
		{
		return vertices(vertexIndex);
		}
	Value& getVertexValue(const Index& vertexIndex) // Ditto
		{
		return vertices(vertexIndex);
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
	Vertex getVertex(const VertexID& vertexID) const // Returns vertex of given valid ID
		{
		return Vertex(this,vertices.calcIndex(vertexID.getIndex()));
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
	Cell getCell(const CellID& cellID) const // Return cell of given valid ID
		{
		return Cell(this,vertices.calcIndex(cellID.getIndex()));
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
	Scalar calcAverageCellSize(void) const; // Calculates an estimate of the average cell size in the data set
	Locator getLocator(void) const // Returns an unlocalized locator for the data set
		{
		return Locator(this);
		}
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_CARTESIAN_IMPLEMENTATION
#include <Templatized/Cartesian.cpp>
#endif

#endif
