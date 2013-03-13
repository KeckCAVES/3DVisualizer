/***********************************************************************
Simplical - Base class for vertex-centered simplical (unstructured)
data sets containing arbitrary value types (scalars, vectors, tensors,
etc.).
Copyright (c) 2004-2007 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SIMPLICAL_IMPLEMENTATION

#include <new>
#include <Misc/OneTimeQueue.h>
#include <Math/Math.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Matrix.h>

#include <Templatized/LinearInterpolator.h>

#include <Templatized/Simplical.h>

#ifdef PERF_COUNTGRADIENTCALCULATIONS
unsigned int numGradients=0;
#endif

namespace Visualization {

namespace Templatized {

/************************************
Methods of class Simplical::GridCell:
************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::GridCell::GridCell(
	void)
	:succ(0)
	{
	/* Initialize neighbour pointers: */
	for(int i=0;i<CellTopology::numFaces;++i)
		neighbours[i]=0;
	}

/************************************
Methods of class Simplical::GridFace:
************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::GridFace::GridFace(
	typename Simplical<ScalarParam,dimensionParam,ValueParam>::GridVertex* sVertices[])
	{
	/* Sort vertex pointers from source array using insertion sort: */
	for(int i=0;i<CellTopology::numFaceVertices;++i)
		{
		int j;
		for(j=i;j>0&&reinterpret_cast<size_t>(vertices[j-1])>reinterpret_cast<size_t>(sVertices[i]);--j)
			vertices[j]=vertices[j-1];
		vertices[j]=sVertices[i];
		}
	}

/**********************************
Methods of class Simplical::EdgeID:
**********************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::EdgeID::EdgeID(
	const typename Simplical<ScalarParam,dimensionParam,ValueParam>::GridVertex* v0,
	const typename Simplical<ScalarParam,dimensionParam,ValueParam>::GridVertex* v1)
	{
	if(reinterpret_cast<size_t>(v0)<reinterpret_cast<size_t>(v1))
		{
		vertices[0]=v0;
		vertices[1]=v1;
		}
	else
		{
		vertices[0]=v1;
		vertices[1]=v0;
		}
	}

/********************************
Methods of class Simplical::Cell:
********************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::Vector
Simplical<ScalarParam,dimensionParam,ValueParam>::Cell::calcVertexGradient(
	int vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	#ifdef PERF_COUNTGRADIENTCALCULATIONS
	++numGradients;
	#endif
	
	/* Gather a least-squares system of linear equations describing the gradient at the cell vertex: */
	Geometry::Matrix<double,dimension,dimension> a(0.0);
	Geometry::ComponentArray<double,dimension> b(0.0);
	
	/* Add one linear equation for each vertex connected to the query vertex by an edge: */
	const GridVertex* centralVertex=cell->vertices[vertexIndex];
	Geometry::Point<double,dimension> c=Geometry::Point<double,dimension>(centralVertex->pos);
	double fc=extractor.getValue(centralVertex->value);
	Misc::HashTable<const GridVertex*,void> vertexHasher(17);
	Misc::OneTimeQueue<const GridCell*> cellQueue(17);
	cellQueue.push(cell);
	while(!cellQueue.empty())
		{
		/* Get the next cell from the queue: */
		const GridCell* cellPtr=cellQueue.front();
		cellQueue.pop();
		
		/* Process all vertices of the cell: */
		for(int vi=0;vi<CellTopology::numVertices;++vi)
			if(cellPtr->vertices[vi]!=centralVertex)
				{
				/* Check if the vertex needs to be processed: */
				const GridVertex* vertexPtr=cellPtr->vertices[vi];
				if(!vertexHasher.isEntry(vertexPtr))
					{
					/* Add a linear equation for the vertex: */
					Geometry::Vector<double,dimension> d;
					for(int i=0;i<dimension;++i)
						d[i]=double(vertexPtr->pos[i])-c[i];
					double df=double(extractor.getValue(vertexPtr->value))-fc;
					for(int i=0;i<dimension;++i)
						{
						for(int j=0;j<dimension;++j)
							a(i,j)+=d[i]*d[j];
						b[i]+=d[i]*df;
						}
					
					/* Mark the vertex as processed: */
					vertexHasher.setEntry(vertexPtr);
					}
				
				/* Add the cell neighbour opposite from the vertex to the queue: */
				if(cellPtr->neighbours[vi]!=0)
					cellQueue.push(cellPtr->neighbours[vi]);
				}
		}
	
	/* Solve the linear system and return the gradient: */
	return Vector(b/a);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::EdgeID
Simplical<ScalarParam,dimensionParam,ValueParam>::Cell::getEdgeID(
	int edgeIndex) const
	{
	return EdgeID(cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][0]],cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][1]]);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::Point
Simplical<ScalarParam,dimensionParam,ValueParam>::Cell::calcEdgePosition(
	int edgeIndex,
	Simplical<ScalarParam,dimensionParam,ValueParam>::Scalar weight) const
	{
	const GridVertex* v0=cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][0]];
	const GridVertex* v1=cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][1]];
	return Geometry::affineCombination(v0->pos,v1->pos,weight);
	}

/***********************************
Methods of class Simplical::Locator:
***********************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::Locator::Locator(
	const Simplical<ScalarParam,dimensionParam,ValueParam>* sDs,
	typename Simplical<ScalarParam,dimensionParam,ValueParam>::Scalar sEpsilon)
	:Cell(sDs,0),
	 epsilon(sEpsilon),epsilon2(Math::sqr(epsilon))
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Simplical<ScalarParam,dimensionParam,ValueParam>::Locator::setEpsilon(
	typename Simplical<ScalarParam,dimensionParam,ValueParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	epsilon2=Math::sqr(epsilon);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
Simplical<ScalarParam,dimensionParam,ValueParam>::Locator::locatePoint(
	const typename Simplical<ScalarParam,dimensionParam,ValueParam>::Point& position,
	bool traceHint)
	{
	/* Give up if the locator is outside the bounding box: */
	if(!ds->domainBox.contains(position))
		return false;
	
	/* If traceHint parameter is false or locator is invalid, start searching from scratch: */
	if(!traceHint||cell==0)
		{
		/* Start searching from cell whose cell center is closest to query position: */
		Cell::operator=(ds->getCell(ds->cellCenterTree.findClosestPoint(position).value));
		}
	
	/* Traverse cells until the current cell contains the query position: */
	bool result=true;
	while(true)
		{
		/* Calculate barycentric coordinates of query position inside current cell: */
		#if 0
		Geometry::Matrix<Scalar,dimensionParam+1,dimensionParam+1> m;
		for(int col=0;col<CellTopology::numVertices;++col)
			{
			for(int row=0;row<dimension;++row)
				m(row,col)=cell->vertices[col]->pos[row];
			m(dimension,col)=Scalar(1);
			}
		Geometry::ComponentArray<Scalar,dimensionParam+1> a;
		for(int i=0;i<dimension;++i)
			a[i]=position[i];
		a[dimension]=Scalar(1);
		cellPos=a/m;
		#else
		Geometry::Matrix<Scalar,dimensionParam,dimensionParam> m;
		for(int col=0;col<dimension;++col)
			for(int row=0;row<dimension;++row)
				m(row,col)=cell->vertices[col+1]->pos[row]-cell->vertices[0]->pos[row];
		Geometry::ComponentArray<Scalar,dimensionParam> a;
		for(int i=0;i<dimension;++i)
			a[i]=position[i]-cell->vertices[0]->pos[i];
		a=a/m;
		cellPos[0]=Scalar(1);
		for(int i=0;i<dimension;++i)
			{
			cellPos[i+1]=a[i];
			cellPos[0]-=a[i];
			}
		#endif
		
		/* Find the most negative component of the barycentric coordinate: */
		Scalar minComp=-epsilon;
		int minFace=-1;
		for(int i=0;i<CellTopology::numVertices;++i)
			if(minComp>cellPos[i])
				{
				minComp=cellPos[i];
				minFace=i;
				}
		
		/* Check if the current cell already contains the query point: */
		if(minFace<0)
			break;
		
		/* Check if the next cell is valid: */
		if(cell->neighbours[minFace]==0)
			{
			result=false;
			break;
			}
		
		/* Go to the next cell: */
		cell=cell->neighbours[minFace];
		}
	
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ValueExtractorParam>
inline
typename ValueExtractorParam::DestValue
Simplical<ScalarParam,dimensionParam,ValueParam>::Locator::calcValue(
	const ValueExtractorParam& extractor) const
	{
	typedef typename ValueExtractorParam::DestValue DestValue;
	typedef LinearInterpolator<DestValue,Scalar> Interpolator;
	
	/* Perform barycentric interpolation: */
	DestValue values[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		values[i]=extractor.getValue(cell->vertices[i]->value);
	return Interpolator::interpolate(CellTopology::numVertices,values,cellPos.getComponents());
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::Vector
Simplical<ScalarParam,dimensionParam,ValueParam>::Locator::calcGradient(
	const ScalarExtractorParam& extractor) const
	{
	typedef LinearInterpolator<Vector,Scalar> Interpolator;
	
	/* Perform barycentric interpolation: */
	Vector values[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		values[i]=Cell::calcVertexGradient(i,extractor);
	return Interpolator::interpolate(CellTopology::numVertices,values,cellPos.getComponents());
	}

/**************************
Methods of class Simplical:
**************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Simplical<ScalarParam,dimensionParam,ValueParam>::connectCells(
	void)
	{
	/* Create face hash table: */
	FaceHasher faceHasher(totalNumCells*CellTopology::numFaces+31);
	
	/* Traverse all cells and connect shared faces: */
	for(GridCell* cPtr=firstGridCell;cPtr!=0;cPtr=cPtr->succ)
		{
		/* Iterate through all faces of the cell: */
		for(int faceIndex=0;faceIndex<CellTopology::numFaces;++faceIndex)
			{
			/* Get pointers to the vertices defining the current face: */
			/* (Invariant: face i contains all vertices except i.) */
			GridVertex* faceVertices[CellTopology::numFaceVertices];
			GridVertex** fvPtr=faceVertices;
			for(int i=0;i<CellTopology::numVertices;++i)
				if(i!=faceIndex)
					{
					*fvPtr=cPtr->vertices[i];
					++fvPtr;
					}
			
			/* Create a face data structure for the current face: */
			GridFace face(faceVertices);
			typename FaceHasher::Iterator oppositeIt=faceHasher.findEntry(face);
			if(oppositeIt.isFinished())
				{
				/* Put entry for this face into face hash table: */
				faceHasher.setEntry(typename FaceHasher::Entry(face,std::pair<GridCell*,int>(cPtr,faceIndex)));
				}
			else
				{
				/* Connect this face to existing face: */
				cPtr->neighbours[faceIndex]=oppositeIt->getDest().first;
				oppositeIt->getDest().first->neighbours[oppositeIt->getDest().second]=cPtr;
				
				/* Remove entry from hash table (won't be needed anymore): */
				faceHasher.removeEntry(oppositeIt);
				}
			}
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::Simplical(
	void)
	:totalNumVertices(0),firstGridVertex(0),lastGridVertex(0),
	 totalNumCells(0),firstGridCell(0),lastGridCell(0),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Simplical<ScalarParam,dimensionParam,ValueParam>::~Simplical(
	void)
	{
	/* Delete all grid cells: */
	while(firstGridCell!=0)
		{
		GridCell* succ=firstGridCell->succ;
		firstGridCell->~GridCell();
		cellAllocator.free(firstGridCell);
		firstGridCell=succ;
		}
	
	/* Delete all grid vertices: */
	while(firstGridVertex!=0)
		{
		GridVertex* succ=firstGridVertex->succ;
		firstGridVertex->~GridVertex();
		vertexAllocator.free(firstGridVertex);
		firstGridVertex=succ;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::GridVertexIterator
Simplical<ScalarParam,dimensionParam,ValueParam>::addVertex(
	const typename Simplical<ScalarParam,dimensionParam,ValueParam>::Point& pos,
	const typename Simplical<ScalarParam,dimensionParam,ValueParam>::Value& value)
	{
	/* Create a new vertex: */
	++totalNumVertices;
	GridVertex* newGridVertex=new(vertexAllocator.allocate()) GridVertex(pos,value);
	
	/* Link new vertex to main vertex list: */
	if(lastGridVertex!=0)
		lastGridVertex->succ=newGridVertex;
	else
		firstGridVertex=newGridVertex;
	lastGridVertex=newGridVertex;
	
	/* Return iterator to new vertex: */
	return GridVertexIterator(newGridVertex);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::CellIterator
Simplical<ScalarParam,dimensionParam,ValueParam>::addCell(
	typename Simplical<ScalarParam,dimensionParam,ValueParam>::GridVertexIterator cellVertices[])
	{
	/* Create a new cell: */
	++totalNumCells;
	GridCell* newGridCell=new(cellAllocator.allocate()) GridCell();
	
	/* Copy vertex pointers: */
	for(int i=0;i<CellTopology::numVertices;++i)
		newGridCell->vertices[i]=cellVertices[i].vertex;
	
	/* Link new cell to main cell list: */
	if(lastGridCell!=0)
		lastGridCell->succ=newGridCell;
	else
		firstGridCell=newGridCell;
	lastGridCell=newGridCell;
	
	#if 0
	/* Connect cell to adjacent cells: */
	for(int faceIndex=0;faceIndex<CellTopology::numFaces;++faceIndex)
		{
		/* Get pointers to the vertices defining the current face: */
		/* (Invariant: face i contains all vertices except i.) */
		GridVertex* faceVertices[CellTopology::numFaceVertices];
		GridVertex** fvPtr=faceVertices;
		int faceVertexIndex=0;
		for(int i=0;i<CellTopology::numVertices;++i)
			if(i!=faceIndex)
				{
				*fvPtr=newGridCell->vertices[i];
				++fvPtr;
				}
		
		/* Create a face data structure for the current face: */
		GridFace face(faceVertices);
		typename FaceHasher::Iterator oppositeIt=faceHasher.findEntry(face);
		if(oppositeIt.isFinished())
			{
			/* Put entry for this face into face hash table: */
			faceHasher.setEntry(typename FaceHasher::Entry(face,std::pair<GridCell*,int>(newGridCell,faceIndex)));
			}
		else
			{
			/* Connect this face to existing face: */
			newGridCell->neighbours[faceIndex]=oppositeIt->getDest().first;
			oppositeIt->getDest().first->neighbours[oppositeIt->getDest().second]=newGridCell;
			
			/* Remove entry from hash table (won't be needed anymore): */
			faceHasher.removeEntry(oppositeIt);
			}
		}
	#endif
	
	/* Return iterator to new cell: */
	return CellIterator(Cell(this,newGridCell));
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Simplical<ScalarParam,dimensionParam,ValueParam>::finalizeGrid(
	void)
	{
	/* Calculate bounding box of all grid vertices: */
	domainBox=Box::empty;
	for(const GridVertex* vPtr=firstGridVertex;vPtr!=0;vPtr=vPtr->succ)
		domainBox.addPoint(vPtr->pos);
	
	/* Connect all cells in the data set: */
	connectCells();
	
	/* Calculate the center of each cell: */
	CellCenter* ccPtr=cellCenterTree.createTree(totalNumCells);
	for(GridCell* cPtr=firstGridCell;cPtr!=0;cPtr=cPtr->succ)
		{
		/* Calculate cell's center point: */
		typename Point::AffineCombiner cc;
		for(int i=0;i<CellTopology::numVertices;++i)
			cc.addPoint(cPtr->vertices[i]->pos);
		
		/* Store cell center and pointer: */
		*ccPtr=CellCenter(cc.getPoint(),cPtr);
		++ccPtr;
		}
	
	/* Create the cell center tree: */
	cellCenterTree.releasePoints(4); // Let's just go ahead and use the multithreaded version
	
	/* Initialize the vertex list bounds: */
	firstVertex=Vertex(this,firstGridVertex);
	lastVertex=Vertex(this,0);
	
	/* Initialize the cell list bounds: */
	firstCell=Cell(this,firstGridCell);
	lastCell=Cell(this,0);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Simplical<ScalarParam,dimensionParam,ValueParam>::setLocatorEpsilon(
	typename Simplical<ScalarParam,dimensionParam,ValueParam>::Scalar newLocatorEpsilon)
	{
	locatorEpsilon=newLocatorEpsilon;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Simplical<ScalarParam,dimensionParam,ValueParam>::Scalar
Simplical<ScalarParam,dimensionParam,ValueParam>::calcAverageCellSize(
	void) const
	{
	/* Estimate cell size as domain volume divided by number of cells: */
	double domainVolume=double(domainBox.getSize(0));
	double cellSize=1.0;
	for(int i=1;i<dimension;++i)
		{
		domainVolume*=double(domainBox.getSize(i));
		cellSize*=double(i+1);
		}
	return Scalar(Math::pow(domainVolume*cellSize/double(totalNumCells),1.0/double(dimension)));
	}

}

}
