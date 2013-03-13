/***********************************************************************
IsosurfaceExtractorIndexedTriangleSet - Specialized version of
IsosurfaceExtractor class for indexed triangle sets.
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

#define VISUALIZATION_TEMPLATIZED_ISOSURFACEEXTRACTORINDEXEDTRIANGLESET_IMPLEMENTATION

#include <Templatized/IsosurfaceExtractorIndexedTriangleSet.h>

namespace Visualization {

namespace Templatized {

/************************************
Methods of class IsosurfaceExtractor:
************************************/

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
int
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractFlatIsosurfaceFragment(
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Cell& cell)
	{
	/* Determine cell vertex values and case index: */
	VScalar cvvs[CellTopology::numVertices];
	int caseIndex=0x0;
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		cvvs[i]=cell.getVertexValue(i,scalarExtractor);
		if(cvvs[i]>=isovalue)
			caseIndex|=1<<i;
		}
	
	/* Calculate the edge intersection points: */
	Point edgeVertices[CellTopology::numEdges];
	int cem=CaseTable::edgeMasks[caseIndex];
	for(int edge=0;edge<CellTopology::numEdges;++edge)
		if(cem&(1<<edge))
			{
			/* Calculate intersection point on the edge: */
			int vi0=CellTopology::edgeVertexIndices[edge][0];
			VScalar d0=cvvs[vi0];
			int vi1=CellTopology::edgeVertexIndices[edge][1];
			VScalar d1=cvvs[vi1];
			Scalar w1=Scalar((isovalue-d0)/(d1-d0));
			edgeVertices[edge]=cell.calcEdgePosition(edge,w1);
			}
	
	/* Store the resulting fragment in the isosurface: */
	for(const int* ctei=CaseTable::triangleEdgeIndices[caseIndex];*ctei>=0;ctei+=3)
		{
		Index* iPtr=isosurface->getNextTriangle();
		Vector normal=Geometry::cross(edgeVertices[ctei[1]]-edgeVertices[ctei[0]],edgeVertices[ctei[2]]-edgeVertices[ctei[0]]);
		for(int i=0;i<3;++i)
			{
			Vertex* vertex=isosurface->getNextVertex();
			vertex->normal=normal.getComponents();
			vertex->position=edgeVertices[ctei[i]].getComponents();
			iPtr[i]=isosurface->addVertex();
			}
		isosurface->addTriangle();
		}
	
	return caseIndex;
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
int
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractSmoothIsosurfaceFragment(
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Cell& cell)
	{
	/* Determine cell vertex values and case index: */
	VScalar cvvs[CellTopology::numVertices];
	int caseIndex=0x0;
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		cvvs[i]=cell.getVertexValue(i,scalarExtractor);
		if(cvvs[i]>=isovalue)
			caseIndex|=1<<i;
		}
	
	int cem=CaseTable::edgeMasks[caseIndex];
	
	/* Get the indices of all vertices that have already been computed, and determine which gradients to compute: */
	Index edgeVertexIndices[CellTopology::numEdges];
	bool cvgns[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		cvgns[i]=false;
	for(int edge=0;edge<CellTopology::numEdges;++edge)
		if(cem&(1<<edge))
			{
			/* Get the ID of the current edge: */
			EdgeID edgeID=cell.getEdgeID(edge);
			
			/* Check if the edge already has a vertex in the isosurface: */
			typename VertexIndexHasher::Iterator vIt=vertexIndices.findEntry(edgeID);
			if(!vIt.isFinished())
				{
				/* Store the vertex index: */
				edgeVertexIndices[edge]=vIt->getDest();
				}
			else
				{
				/* Mark the vertex as invalid: */
				edgeVertexIndices[edge]=~Index(0);
				
				/* Mark the edge's gradients as required: */
				for(int i=0;i<2;++i)
					cvgns[CellTopology::edgeVertexIndices[edge][i]]=true;
				}
			}
	
	/* Calculate the required cell vertex gradients: */
	Vector cvgs[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		if(cvgns[i])
			cvgs[i]=cell.calcVertexGradient(i,scalarExtractor);
	
	/* Calculate the edge intersection points: */
	for(int edge=0;edge<CellTopology::numEdges;++edge)
		if((cem&(1<<edge))&&edgeVertexIndices[edge]==~Index(0))
			{
			/* Create a new vertex: */
			Vertex* vertex=isosurface->getNextVertex();
			
			/* Calculate the intersection point on the edge: */
			int vi0=CellTopology::edgeVertexIndices[edge][0];
			VScalar d0=cvvs[vi0];
			int vi1=CellTopology::edgeVertexIndices[edge][1];
			VScalar d1=cvvs[vi1];
			Scalar w1=Scalar((isovalue-d0)/(d1-d0));
			Vector v=cvgs[vi0]*(Scalar(1)-w1)+cvgs[vi1]*w1;
			v/=-v.mag();
			vertex->normal=v.getComponents();
			vertex->position=cell.calcEdgePosition(edge,w1).getComponents();
			
			/* Store the vertex in the isosurface, and its index in the hash table: */
			edgeVertexIndices[edge]=isosurface->addVertex();
			vertexIndices.setEntry(typename VertexIndexHasher::Entry(cell.getEdgeID(edge),edgeVertexIndices[edge]));
			}
	
	/* Store the resulting isosurface fragment in the isosurface: */
	for(const int* ctei=CaseTable::triangleEdgeIndices[caseIndex];*ctei>=0;ctei+=3)
		{
		Index* iPtr=isosurface->getNextTriangle();
		for(int i=0;i<3;++i)
			iPtr[i]=edgeVertexIndices[ctei[i]];
		isosurface->addTriangle();
		}
	
	return caseIndex;
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::IsosurfaceExtractor(
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::DataSet* sDataSet,
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 scalarExtractor(sScalarExtractor),
	 extractionMode(FLAT),
	 isosurface(0),
	 vertexIndices(101),
	 cellQueue(101)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::~IsosurfaceExtractor(
	void)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::setExtractionMode(
	typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::ExtractionMode newExtractionMode)
	{
	extractionMode=newExtractionMode;
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractIsosurface(
	typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::VScalar newIsovalue,
	typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Isosurface& newIsosurface)
	{
	/* Set the isosurface extraction parameters: */
	isovalue=newIsovalue;
	isosurface=&newIsosurface;
	
	/* Extract isosurface fragments from all cells: */
	if(extractionMode==FLAT)
		{
		for(typename DataSet::CellIterator cIt=dataSet->beginCells();cIt!=dataSet->endCells();++cIt)
			{
			/* Extract the cell's isosurface fragment: */
			extractFlatIsosurfaceFragment(*cIt);
			}
		}
	else
		{
		for(typename DataSet::CellIterator cIt=dataSet->beginCells();cIt!=dataSet->endCells();++cIt)
			{
			/* Extract the cell's isosurface fragment: */
	  	extractSmoothIsosurfaceFragment(*cIt);
			}
		}
	isosurface->flush();
	
	/* Clean up: */
	isosurface=0;
	vertexIndices.clear();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::extractSeededIsosurface(
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Locator& seedLocator,
	typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Isosurface& newIsosurface)
	{
	/* Set the isosurface extraction parameters: */
	isovalue=seedLocator.calcValue(scalarExtractor);
	isosurface=&newIsosurface;
	
	/* Push the seed cell onto the queue: */
	cellQueue.clear();
	cellQueue.push(seedLocator.getCellID());
	
	/* Extract isosurface fragments until the queue is empty: */
	while(!cellQueue.empty())
		{
		/* Get the next cell: */
		Cell cell=dataSet->getCell(cellQueue.front());
		cellQueue.pop();
		
		/* Extract the cell's isosurface fragment: */
		int caseIndex;
		if(extractionMode==FLAT)
			caseIndex=extractFlatIsosurfaceFragment(cell);
		else
		  caseIndex=extractSmoothIsosurfaceFragment(cell);
		
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
	isosurface->flush();
	
	/* Clean up: */
	isosurface=0;
	vertexIndices.clear();
	cellQueue.clear();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::startSeededIsosurface(
	const typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Locator& seedLocator,
	typename IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::Isosurface& newIsosurface)
	{
	/* Set the isosurface extraction parameters: */
	isovalue=seedLocator.calcValue(scalarExtractor);
	isosurface=&newIsosurface;
	
	/* Push the seed cell onto the queue: */
	cellQueue.clear();
	cellQueue.push(seedLocator.getCellID());
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
template <class ContinueFunctorParam>
inline
bool
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::continueSeededIsosurface(
	const ContinueFunctorParam& cf)
	{
	/* Extract isosurface fragments until the queue is empty: */
	while(!cellQueue.empty()&&cf())
		{
		/* Get the next cell: */
		Cell cell=dataSet->getCell(cellQueue.front());
		cellQueue.pop();
		
		/* Extract the cell's isosurface fragment: */
		int caseIndex;
		if(extractionMode==FLAT)
			caseIndex=extractFlatIsosurfaceFragment(cell);
		else
		  caseIndex=extractSmoothIsosurfaceFragment(cell);
		
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
	isosurface->flush();
	
	return cellQueue.empty();
	}

template <class DataSetParam,class ScalarExtractorParam,class VertexParam>
inline
void
IsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IndexedTriangleSet<VertexParam> >::finishSeededIsosurface(
	void)
	{
	/* Clean up: */
	isosurface=0;
	vertexIndices.clear();
	cellQueue.clear();
	}

}

}
