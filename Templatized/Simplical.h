/***********************************************************************
Simplical - Base class for vertex-centered simplical (unstructured)
data sets containing arbitrary value types (scalars, vectors, tensors,
etc.).
Copyright (c) 2004-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SIMPLICAL_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SIMPLICAL_INCLUDED

#include <utility>
#include <Misc/PoolAllocator.h>
#include <Misc/HashTable.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>

#include <Templatized/Simplex.h>
#include <Templatized/PointerID.h>
#include <Templatized/IteratorWrapper.h>

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class ValueParam>
class Simplical
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
	typedef Simplex<dimensionParam> CellTopology; // Policy class to select appropriate cell algorithms
	
	/* Definition of the data set's value space: */
	typedef ValueParam Value; // Data set's value type
	
	/* Low-level definitions of data set storage: */
	struct GridVertex // Structure for valued grid vertices
		{
		/* Elements: */
		public:
		Point pos; // Position of grid vertex in data set's domain
		Value value; // Grid vertex' value
		GridVertex* succ; // Pointer to next grid vertex in data set
		
		/* Constructors and destructors: */
		GridVertex(void) // Dummy constructor
			:succ(0)
			{
			}
		GridVertex(const Point& sPos,const Value& sValue) // Elementwise constructor
			:pos(sPos),value(sValue),
			 succ(0)
			{
			}
		};
	
	class GridVertexIterator // Structure to iterate through grid vertices (with ability to change them)
		{
		friend class Simplical;
		
		/* Elements: */
		private:
		GridVertex* vertex; // Vertex pointed to by iterator
		
		/* Constructors and destructors: */
		public:
		GridVertexIterator(void) // Creates invalid iterator
			:vertex(0)
			{
			}
		private:
		GridVertexIterator(GridVertex* sVertex) // Creates iterator to the given vertex
			:vertex(sVertex)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const GridVertexIterator& vi1,const GridVertexIterator& vi2) // Compares two vertex iterators for equality
			{
			return vi1.vertex==vi2.vertex;
			}
		friend bool operator!=(const GridVertexIterator& vi1,const GridVertexIterator& vi2) // Compares two vertex iterators for inequality
			{
			return vi1.vertex!=vi2.vertex;
			}
		GridVertex& operator*(void) const // Returns vertex pointed to by the iterator
			{
			return *vertex;
			}
		GridVertex* operator->(void) const // Returns vertex pointed to by the iterator
			{
			return vertex;
			}
		GridVertexIterator& operator++(void) // Pre-increment operator
			{
			vertex=vertex->succ;
			return *this;
			}
		};
	
	private:
	struct GridCell // Structure for simplical grid cells
		{
		/* Elements: */
		public:
		GridVertex* vertices[CellTopology::numVertices]; // Array of pointers to cell's vertices
		GridCell* neighbours[CellTopology::numFaces]; // Array of pointers to neighbouring cells
		GridCell* succ; // Pointer to next grid cell in data set
		
		/* Constructors and destructors: */
		GridCell(void); // Creates a nonconnected grid cell with uninitialized vertex pointers
		};
	
	struct GridFace // Structure for faces of grid cells
		{
		/* Elements: */
		public:
		GridVertex* vertices[CellTopology::numFaceVertices]; // Array of pointers to face's vertices
		
		/* Constructors and destructors: */
		GridFace(GridVertex* sVertices[CellTopology::numFaceVertices]); // Creates grid face from given vertex pointer array
		
		/* Methods: */
		inline friend bool operator!=(const GridFace& f1,const GridFace& f2)
			{
			for(int i=0;i<CellTopology::numFaceVertices;++i)
				if(f1.vertices[i]!=f2.vertices[i])
					return true;
			return false;
			}
		inline static size_t hash(const GridFace& source,size_t tableSize)
			{
			size_t result=0;
			for(int i=0;i<CellTopology::numFaceVertices;++i)
				result=(result+reinterpret_cast<size_t>(source.vertices[i]))*17;
			return result%tableSize;
			}
		};
	
	typedef Misc::PoolAllocator<GridVertex> GridVertexAllocator; // Type of memory allocators for grid vertices
	typedef Misc::PoolAllocator<GridCell> GridCellAllocator; // Type of memory allocators for grid cells
	typedef Misc::HashTable<GridFace,std::pair<GridCell*,int>,GridFace> FaceHasher; // Data type for hash tables used during data set construction
	
	/* Data set interface classes: */
	public:
	typedef PointerID<GridVertex> VertexID; // Class to identify vertices
	
	class Vertex // Class to represent and iterate through vertices
		{
		friend class Simplical;
		
		/* Elements: */
		private:
		const Simplical* ds; // Pointer to data set containing the vertex
		const GridVertex* vertex; // Pointer to the grid vertex
		
		/* Constructors and destructors: */
		public:
		Vertex(void) // Creates an invalid vertex
			:ds(0),vertex(0)
			{
			}
		private:
		Vertex(const Simplical* sDs,const GridVertex* sVertex)
			:ds(sDs),vertex(sVertex)
			{
			}
		
		/* Methods: */
		public:
		const Point& getPosition(void) const // Returns vertex' position in domain
			{
			return vertex->pos;
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getValue(const ValueExtractorParam& extractor) const // Returns vertex' value based on given extractor
			{
			return extractor.getValue(vertex->value);
			}
		template <class ScalarExtractorParam>
		Vector calcGradient(const ScalarExtractorParam& extractor) const // Returns gradient at the vertex, based on given scalar extractor
			{
			return ds->calcVertexGradient(vertex,extractor);
			}
		VertexID getID(void) const // Returns vertex' ID
			{
			return VertexID(vertex);
			}
		
		/* Iterator methods: */
		friend bool operator==(const Vertex& v1,const Vertex& v2)
			{
			return v1.vertex==v2.vertex;
			}
		friend bool operator!=(const Vertex& v1,const Vertex& v2)
			{
			return v1.vertex!=v2.vertex;
			}
		Vertex& operator++(void) // Pre-increment operator
			{
			vertex=vertex->succ;
			return *this;
			}
		};
	
	typedef IteratorWrapper<Vertex> VertexIterator; // Class to iterate through vertices
	
	class EdgeID // Class to identify cell edges
		{
		friend class Cell;
		
		/* Elements: */
		private:
		const GridVertex* vertices[2]; // The vertices of the edge, sorted by pointer value
		
		/* Constructors: */
		public:
		EdgeID(void)
			{
			vertices[0]=vertices[1]=0;
			}
		EdgeID(const GridVertex* v0,const GridVertex* v1); // Elementwise constructor
		
		/* Methods: */
		friend bool operator==(const EdgeID& ei1,const EdgeID& ei2) // Compares two cell edges for equality
			{
			return ei1.vertices[0]==ei2.vertices[0]&&ei1.vertices[1]==ei2.vertices[1];
			}
		friend bool operator!=(const EdgeID& ei1,const EdgeID& ei2) // Compares two cell edges for inequality
			{
			return ei1.vertices[0]!=ei2.vertices[0]||ei1.vertices[1]!=ei2.vertices[1];
			}
		static size_t hash(const EdgeID& ei,size_t tableSize) // Calculates hash index for a cell edge
			{
			return (reinterpret_cast<size_t>(ei.vertices[0])*17+reinterpret_cast<size_t>(ei.vertices[1])*31)%tableSize;
			}
		};
	
	typedef PointerID<GridCell> CellID; // Class to identify cells
	
	class Locator;
	
	class Cell // Class to represent and iterate through cells
		{
		friend class Simplical;
		friend class Locator;
		
		/* Elements: */
		private:
		const Simplical* ds; // Pointer to data set containing the vertex
		const GridCell* cell; // Grid cell represented by the cell
		
		/* Constructors and destructors: */
		public:
		Cell(void) // Creates an invalid cell
			:ds(0),cell(0)
			{
			}
		private:
		Cell(const Simplical* sDs,const GridCell* sCell) // Elementwise constructor
			:ds(sDs),cell(sCell)
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
			return cell->vertices[vertexIndex]->pos;
			}
		template <class ValueExtractorParam>
		typename ValueExtractorParam::DestValue getVertexValue(int vertexIndex,const ValueExtractorParam& extractor) const // Returns value of given vertex of the cell, based on given extractor
			{
			return extractor.getValue(cell->vertices[vertexIndex]->value);
			}
		template <class ScalarExtractorParam>
		Vector calcVertexGradient(int vertexIndex,const ScalarExtractorParam& extractor) const; // Returns gradient at given vertex of the cell, based on given scalar extractor
		EdgeID getEdgeID(int edgeIndex) const; // Returns ID of given edge of the cell
		Point calcEdgePosition(int edgeIndex,Scalar weight) const; // Returns an interpolated point along the given edge
		CellID getID(void) const // Returns cell's ID
			{
			return CellID(cell);
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
			cell=cell->succ;
			return *this;
			}
		};
	
	typedef IteratorWrapper<Cell> CellIterator; // Class to iterate through cells
	
	class Locator:private Cell // Class responsible for evaluating a data set at a given position
		{
		friend class Simplical;
		
		/* Embedded classes: */
		private:
		typedef Geometry::ComponentArray<Scalar,dimensionParam+1> CellPosition; // Type for local cell coordinates
		
		/* Elements: */
		using Cell::ds;
		using Cell::cell;
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		Scalar epsilon,epsilon2; // Accuracy threshold of point location algorithm
		
		/* Constructors and destructors: */
		public:
		Locator(void) // Creates invalid locator
			{
			}
		private:
		Locator(const Simplical* sDs,Scalar sEpsilon); // Creates non-localized locator associated with given data set
		
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
	GridVertexAllocator vertexAllocator; // Memory allocator for grid vertices
	size_t totalNumVertices; // Total number of vertices in data set
	GridVertex* firstGridVertex; // Pointer to first vertex in data set
	GridVertex* lastGridVertex; // Pointer to last vertex in data set
	GridCellAllocator cellAllocator; // Memory allocator for grid cells
	size_t totalNumCells; // Total number of cells in data set
	GridCell* firstGridCell; // Pointer to first cell in data set
	GridCell* lastGridCell; // Pointer to last cell in data set
	CellCenterTree cellCenterTree; // Kd-tree containing cell centers
	VertexIterator firstVertex,lastVertex; // Bounds of vertex list
	CellIterator firstCell,lastCell; // Bounds of cell list
	Box domainBox; // Bounding box of all vertices
	Scalar locatorEpsilon; // Default accuracy threshold for locators working on this data set
	
	/* Private methods: */
	void connectCells(void); // Creates simplical mesh from unconnected simplices by connecting shared faces
	
	/* Constructors and destructors: */
	public:
	Simplical(void); // Creates an "empty" simplical data set
	~Simplical(void); // Destroys the data set
	
	/* Data set construction methods: */
	GridVertexIterator addVertex(const Point& pos,const Value& value); // Adds a new grid vertex to the data set
	CellIterator addCell(GridVertexIterator cellVertices[GridCell::numVertices]); // Adds a new cell to the data set
	
	/* Low-level data access methods: */
	GridVertexIterator beginGridVertices(void) // Returns iterator to the first vertex
		{
		return GridVertexIterator(firstGridVertex);
		}
	GridVertexIterator endGridVertices(void) // Returns iterator behind last vertex
		{
		return GridVertexIterator(0);
		}
	void finalizeGrid(void); // Recalculates derived grid information after grid structure change
	void setLocatorEpsilon(Scalar newLocatorEpsilon); // Sets the default accuracy threshold for locators working on this data set
	
	/* Methods implementing the data set interface: */
	size_t getTotalNumVertices(void) const // Returns total number of vertices in the data set
		{
		return totalNumVertices;
		}
	Vertex getVertex(const VertexID& vertexID) const // Returns vertex of given valid ID
		{
		return Vertex(this,vertexID.getObject());
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
	Cell getCell(const CellID& cellID) const // Returns cell of given valid ID
		{
		return Cell(this,cellID.getObject());
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
		return Locator(this,locatorEpsilon);
		}
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SIMPLICAL_IMPLEMENTATION
#include <Templatized/Simplical.icpp>
#endif

#endif
