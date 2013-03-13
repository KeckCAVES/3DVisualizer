/***********************************************************************
SliceExtractorIndexedTriangleSet - Specialized version of SliceExtractor
class for indexed triangle sets.
Copyright (c) 2006-2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEEXTRACTORINDEXEDTRIANGLESET_IMPLEMENTATION

#include <Templatized/SliceExtractorIndexedTriangleSet.h>

namespace Visualization {

namespace Templatized {

/*******************************
Methods of class SliceExtractor:
*******************************/

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
int
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractSliceFragment(
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Cell& cell)
	{
	/* Determine cell vertex offsets and case index: */
	Scalar cvos[CellTopology::numVertices];
	int caseIndex=0x0;
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		cvos[i]=slicePlane.calcDistance(cell.getVertexPosition(i));
		if(cvos[i]>=Scalar(0))
			caseIndex|=1<<i;
		}
	
	/* Calculate the intersection points: */
	int numPoints;
	Index edgeVertexIndices[CellTopology::numEdges];
	int edge;
	for(numPoints=0;(edge=CaseTable::edgeIndices[caseIndex][numPoints])>=0;++numPoints)
		{
		/* Get the ID of the current edge: */
		EdgeID edgeID=cell.getEdgeID(edge);
		
		/* Check if the edge already has a vertex in the slice: */
		typename VertexIndexHasher::Iterator vIt=vertexIndices.findEntry(edgeID);
		if(vIt.isFinished())
			{
			/* Create a new vertex: */
			Vertex* vertex=slice->getNextVertex();
			
			/* Calculate intersection point on the edge: */
			int vi0=CellTopology::edgeVertexIndices[edge][0];
			int vi1=CellTopology::edgeVertexIndices[edge][1];
			Scalar w1=(Scalar(0)-cvos[vi0])/(cvos[vi1]-cvos[vi0]);
			Scalar w0=Scalar(1)-w1;
			VScalar val0=cell.getVertexValue(vi0,scalarExtractor);
			VScalar val1=cell.getVertexValue(vi1,scalarExtractor);
			vertex->texCoord[0]=val0*VScalar(w0)+val1*VScalar(w1);
			vertex->position=cell.calcEdgePosition(edge,w1).getComponents();
			
			/* Store the vertex in the slice, and its index in the hash table: */
			edgeVertexIndices[numPoints]=slice->addVertex();
			vertexIndices.setEntry(typename VertexIndexHasher::Entry(edgeID,edgeVertexIndices[numPoints]));
			}
		else
			edgeVertexIndices[numPoints]=vIt->getDest();
		}
	
	/* Store the resulting fragment in the slice: */
	for(int i=2;i<numPoints;++i)
		{
		Index* iPtr=slice->getNextTriangle();
		iPtr[0]=edgeVertexIndices[0];
		iPtr[1]=edgeVertexIndices[i-1];
		iPtr[2]=edgeVertexIndices[i];
		slice->addTriangle();
		}
	
	return caseIndex;
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::SliceExtractor(
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::DataSet* sDataSet,
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 scalarExtractor(sScalarExtractor),
	 slice(0),
	 vertexIndices(101),
	 cellQueue(101)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::~SliceExtractor(
	void)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractSlice(
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Plane& newSlicePlane,
	typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Slice& newSlice)
	{
	/* Set the isosurface extraction parameters: */
	slicePlane=newSlicePlane;
	slice=&newSlice;
	
	/* Extract slice fragments from all cells: */
	for(typename DataSet::CellIterator cIt=dataSet->beginCells();cIt!=dataSet->endCells();++cIt)
		{
		/* Extract the cell's slice fragment: */
		extractSliceFragment(*cIt);
		}
	
	/* Clean up: */
	slice->flush();
	slice=0;
	vertexIndices.clear();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractSeededSlice(
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Locator& seedLocator,
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Plane& newSlicePlane,
	typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Slice& newSlice)
	{
	/* Set the slice extraction parameters: */
	slicePlane=newSlicePlane;
	slice=&newSlice;
	
	/* Push the seed cell onto the queue: */
	cellQueue.push(seedLocator.getCellID());
	
	/* Extract slice fragments until the queue is empty: */
	while(!cellQueue.empty())
		{
		/* Get the next cell: */
		Cell cell=dataSet->getCell(cellQueue.front());
		cellQueue.pop();
		
		/* Extract the cell's slice fragment: */
		int caseIndex=extractSliceFragment(cell);
		
		/* Push all intersected neighbouring cells onto the queue: */
		for(int i=0;i<CellTopology::numFaces;++i)
			if(CaseTable::neighbourMasks[caseIndex]&(1<<i))
				{
				CellID neighbourID=cell.getNeighbourID(i);
				
				/* Push the neighbour onto the queue if it is valid: */
				if(neighbourID.isValid())
					cellQueue.push(neighbourID);
				}
		}
	
	/* Clean up: */
	slice->flush();
	slice=0;
	vertexIndices.clear();
	cellQueue.clear();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::startSeededSlice(
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Locator& seedLocator,
	const typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Plane& newSlicePlane,
	typename SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Slice& newSlice)
	{
	/* Set the slice extraction parameters: */
	slicePlane=newSlicePlane;
	slice=&newSlice;
	
	/* Push the seed cell onto the queue: */
	cellQueue.push(seedLocator.getCellID());
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
template <class ContinueFunctorParam>
inline
bool
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::continueSeededSlice(
	const ContinueFunctorParam& cf)
	{
	/* Extract slice fragments until the queue is empty: */
	while(!cellQueue.empty()&&cf())
		{
		/* Get the next cell: */
		Cell cell=dataSet->getCell(cellQueue.front());
		cellQueue.pop();
		
		/* Extract the cell's slice fragment: */
		int caseIndex=extractSliceFragment(cell);
		
		/* Push all intersected neighbouring cells onto the queue: */
		for(int i=0;i<CellTopology::numFaces;++i)
			if(CaseTable::neighbourMasks[caseIndex]&(1<<i))
				{
				CellID neighbourID=cell.getNeighbourID(i);
				
				/* Push the neighbour onto the queue if it is valid: */
				if(neighbourID.isValid())
					cellQueue.push(neighbourID);
				}
		}
	slice->flush();
	
	return cellQueue.empty();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
SliceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::finishSeededSlice(
	void)
	{
	/* Clean up: */
	slice=0;
	vertexIndices.clear();
	cellQueue.clear();
	}

}

}
