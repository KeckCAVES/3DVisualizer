/***********************************************************************
ColoredIsosurfaceExtractor - Generic class to extract isosurfaces color-
mapped by a secondary scalar extractor from data sets.
Copyright (c) 2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_COLOREDISOSURFACEEXTRACTOR_IMPLEMENTATION

#include <Templatized/ColoredIsosurfaceExtractor.h>

namespace Visualization {

namespace Templatized {

/*******************************************
Methods of class ColoredIsosurfaceExtractor:
*******************************************/

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
int
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::extractFlatIsosurfaceFragment(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Cell& cell)
	{
	/* Determine cell vertex values and case index: */
	VScalar cvvs[CellTopology::numVertices]; // Vertex values of primary scalar extractor
	VScalar colorCvvs[CellTopology::numVertices]; // Vertex values of secondary scalar extractor
	int caseIndex=0x0;
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		cvvs[i]=cell.getVertexValue(i,scalarExtractor);
		if(cvvs[i]>=isovalue)
			caseIndex|=1<<i;
		colorCvvs[i]=cell.getVertexValue(i,colorScalarExtractor);
		}
	
	/* Calculate the edge intersection points: */
	Point edgeVertices[CellTopology::numEdges];
	VScalar edgeColorValues[CellTopology::numEdges];
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
			edgeColorValues[edge]=colorCvvs[vi0]*(VScalar(1)-VScalar(w1))+colorCvvs[vi1]*VScalar(w1);
			}
	
	/* Store the resulting fragment in the isosurface: */
	for(const int* ctei=CaseTable::triangleEdgeIndices[caseIndex];*ctei>=0;ctei+=3)
		{
		Vertex* vPtr=isosurface->getNextTriangleVertices();
		Vector normal=Geometry::cross(edgeVertices[ctei[1]]-edgeVertices[ctei[0]],edgeVertices[ctei[2]]-edgeVertices[ctei[0]]);
		for(int i=0;i<3;++i)
			{
			vPtr[i].texCoord[0]=typename Vertex::TexCoord::Scalar(edgeColorValues[ctei[i]]);
			vPtr[i].normal=normal.getComponents();
			vPtr[i].position=edgeVertices[ctei[i]].getComponents();
			}
		isosurface->addTriangle();
		}
	
	return caseIndex;
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
int
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::extractSmoothIsosurfaceFragment(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Cell& cell)
	{
	/* Determine cell vertex values and case index: */
	VScalar cvvs[CellTopology::numVertices]; // Vertex values of primary scalar extractor
	VScalar colorCvvs[CellTopology::numVertices]; // Vertex values of secondary scalar extractor
	int caseIndex=0x0;
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		cvvs[i]=cell.getVertexValue(i,scalarExtractor);
		if(cvvs[i]>=isovalue)
			caseIndex|=1<<i;
		colorCvvs[i]=cell.getVertexValue(i,colorScalarExtractor);
		}
	
	int cem=CaseTable::edgeMasks[caseIndex];
	
	/* Calculate the cell vertex gradients: */
	bool cvgns[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		cvgns[i]=false;
	for(int edge=0;edge<CellTopology::numEdges;++edge)
		if(cem&(1<<edge))
			for(int i=0;i<2;++i)
				cvgns[CellTopology::edgeVertexIndices[edge][i]]=true;
	Vector cvgs[CellTopology::numVertices];
	for(int i=0;i<CellTopology::numVertices;++i)
		if(cvgns[i])
			cvgs[i]=cell.calcVertexGradient(i,scalarExtractor);
	
	/* Calculate the edge intersection points: */
	typename Vertex::Position edgeVertices[CellTopology::numEdges];
	typename Vertex::Normal edgeNormals[CellTopology::numEdges];
	VScalar edgeColorValues[CellTopology::numEdges];
	for(int edge=0;edge<CellTopology::numEdges;++edge)
		if(cem&(1<<edge))
			{
			/* Calculate intersection point on the edge: */
			int vi0=CellTopology::edgeVertexIndices[edge][0];
			VScalar d0=cvvs[vi0];
			int vi1=CellTopology::edgeVertexIndices[edge][1];
			VScalar d1=cvvs[vi1];
			Scalar w1=Scalar((isovalue-d0)/(d1-d0));
			edgeVertices[edge]=cell.calcEdgePosition(edge,w1).getComponents();
			Vector v=cvgs[vi0]*(Scalar(1)-w1)+cvgs[vi1]*w1;
			v/=-v.mag();
			edgeNormals[edge]=v.getComponents();
			edgeColorValues[edge]=colorCvvs[vi0]*(VScalar(1)-VScalar(w1))+colorCvvs[vi1]*VScalar(w1);
			}
	
	/* Render the resulting isosurface fragment: */
	for(const int* ctei=CaseTable::triangleEdgeIndices[caseIndex];*ctei>=0;ctei+=3)
		{
		Vertex* vPtr=isosurface->getNextTriangleVertices();
		for(int i=0;i<3;++i)
			{
			vPtr[i].texCoord[0]=typename Vertex::TexCoord::Scalar(edgeColorValues[ctei[i]]);
			vPtr[i].normal=edgeNormals[ctei[i]];
			vPtr[i].position=edgeVertices[ctei[i]];
			}
		isosurface->addTriangle();
		}
	
	return caseIndex;
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::ColoredIsosurfaceExtractor(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::DataSet* sDataSet,
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::ScalarExtractor& sScalarExtractor,
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::ScalarExtractor& sColorScalarExtractor)
	:dataSet(sDataSet),
	 scalarExtractor(sScalarExtractor),
	 colorScalarExtractor(sColorScalarExtractor),
	 extractionMode(FLAT),
	 isosurface(0),
	 cellQueue(101)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::~ColoredIsosurfaceExtractor(
	void)
	{
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::setColorScalarExtractor(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::ScalarExtractor& newColorScalarExtractor)
	{
	colorScalarExtractor=newColorScalarExtractor;
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::setExtractionMode(
	typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::ExtractionMode newExtractionMode)
	{
	extractionMode=newExtractionMode;
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::extractIsosurface(
	typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::VScalar newIsovalue,
	typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Isosurface& newIsosurface)
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
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::extractSeededIsosurface(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Locator& seedLocator,
	typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Isosurface& newIsosurface)
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
	cellQueue.clear();
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::startSeededIsosurface(
	const typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Locator& seedLocator,
	typename ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::Isosurface& newIsosurface)
	{
	/* Set the isosurface extraction parameters: */
	isovalue=seedLocator.calcValue(scalarExtractor);
	isosurface=&newIsosurface;
	
	/* Push the seed cell onto the queue: */
	cellQueue.clear();
	cellQueue.push(seedLocator.getCellID());
	}

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
template <class ContinueFunctorParam>
inline
bool
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::continueSeededIsosurface(
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

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
inline
void
ColoredIsosurfaceExtractor<DataSetParam,ScalarExtractorParam,IsosurfaceParam>::finishSeededIsosurface(
	void)
	{
	/* Clean up: */
	isosurface=0;
	cellQueue.clear();
	}

}

}
