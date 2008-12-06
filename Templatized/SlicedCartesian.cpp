/***********************************************************************
SlicedSlicedCartesian - Base class for vertex-centered cartesian data sets
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

#define VISUALIZATION_TEMPLATIZED_SLICEDCARTESIAN_IMPLEMENTATION

#include <Math/Math.h>

#include <Templatized/LinearInterpolator.h>

#include <Templatized/SlicedCartesian.h>

namespace Visualization {

namespace Templatized {

/************************************************
Methods of class SlicedCartesian::VertexIterator:
************************************************/

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::VertexIterator::getVertexPos(
	void) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		result[i]=Scalar(index[i])*grid->cellSize[i];
	return result;
	}

/**************************************
Methods of class SlicedCartesian::Cell:
**************************************/

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell::getVertexPos(
	int vertexIndex) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		{
		int pos=index[i];
		if(vertexIndex&(1<<i))
			++pos;
		result[i]=Scalar(pos)*grid->cellSize[i];
		}
	return result;
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell::calcVertexGradient(
	int vertexIndex,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Array& slice) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	/* Return the vertex gradient: */
	return grid->calcVertexGradient(cellVertexIndex,slice);
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::CellEdge
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell::getEdge(
	int edgeIndex) const
	{
	return CellEdge(baseVertex+grid->vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]],edgeIndex>>(dimension-1));
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell::getEdgePos(
	int edgeIndex,
	SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Scalar weight) const
	{
	int edgeBaseIndex=CellTopology::edgeVertexIndices[edgeIndex][0];
	int edgeDirection=edgeIndex>>(dimension-1);
	Point result;
	for(int i=0;i<dimension;++i)
		{
		int pos=index[i];
		if(edgeBaseIndex&(1<<i))
			++pos;
		result[i]=Scalar(pos)*grid->cellSize[i];
		}
	result[edgeDirection]+=weight*grid->cellSize[edgeDirection];
	return result;
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Cell::getNeighbour(
	int neighbourIndex) const
	{
	Index neighbourCellIndex=index;
	int face=neighbourIndex>>1;
	if(neighbourIndex&0x1)
		{
		++neighbourCellIndex[face];
		if(neighbourCellIndex[face]<grid->numCells[face])
			return Cell(grid,neighbourCellIndex);
		else
			return Cell();
		}
	else
		{
		--neighbourCellIndex[face];
		if(neighbourCellIndex[face]>=0)
			return Cell(grid,neighbourCellIndex);
		else
			return Cell();
		}
	}

/*****************************************
Methods of class SlicedCartesian::Locator:
*****************************************/

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Locator::Locator(
	void)
	{
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Locator::Locator(
	const SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>* sGrid)
	:cell(sGrid)
	{
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
bool
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Locator::locatePoint(
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Point& position,
	bool traceHint)
	{
	/* Ignore traceHint parameter; it is cheaper to locate points from scratch each time */
	
	/* Locate the new position: */
	bool result=true;
	for(int i=0;i<dimension;++i)
		{
		/* Convert the position to canonical grid coordinates (cellSize == 1): */
		Scalar p=position[i]/cell.grid->cellSize[i];
		
		/* Find the index of the cell containing the position: */
		cell.index[i]=int(Math::floor(p));
		if(cell.index[i]<0)
			{
			cell.index[i]=0;
			result=false;
			}
		else if(cell.index[i]>cell.grid->numCells[i]-1)
			{
			cell.index[i]=cell.grid->numCells[i]-1;
			result=false;
			}
		
		/* Calculate the position's local coordinate inside its cell: */
		cellPos[i]=p-Scalar(cell.index[i]);
		}
	
	/* Update the cell's base address: */
	cell.baseVertex=cell.grid->vertices.getAddress(cell.index);
	
	return result;
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
template <class ValueExtractorParam>
inline
typename ValueExtractorParam::DestValue
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Locator::calcValue(
	const ValueExtractorParam& extractor) const
	{
	typedef typename ValueExtractorParam::DestValue DestValue;
	typedef LinearInterpolator<DestValue,Scalar> Interpolator;
	
	/* Perform multilinear interpolation: */
	DestValue v[CellTopology::numVertices>>1]; // Array of intermediate interpolation values
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	Scalar w1=cellPos[interpolationDimension];
	Scalar w0=Scalar(1)-w1;
	for(int vi=0;vi<numSteps;++vi)
		{
		ptrdiff_t vIndex=cell.baseVertexIndex+cell.grid->vertexOffsets[vi];
		v[vi]=Interpolator::interpolate(extractor.getValue(cell.getGrid(),vIndex+0),w0,extractor.getValue(cell.getGrid(),vIndex+1),w1);
		}
	for(int i=1;i<dimension;++i)
		{
		--interpolationDimension;
		numSteps>>=1;
		w1=cellPos[interpolationDimension];
		w0=Scalar(1)-w1;
		for(int vi=0;vi<numSteps;++vi)
			v[vi]=Interpolator::interpolate(v[vi],w0,v[vi+numSteps],w1);
		}
	
	/* Return final result: */
	return v[0];
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Locator::calcGradient(
	const ScalarExtractorParam& extractor) const
	{
	typedef LinearInterpolator<Vector,Scalar> Interpolator;
	
	/* Get the scalar extractor's scalar slice: */
	const Array& slice=extractor.getSlice(cell.getGrid());
	
	/* Perform multilinear interpolation: */
	Vector v[CellTopology::numVertices>>1]; // Array of intermediate interpolation values
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	Scalar w1=cellPos[interpolationDimension];
	Scalar w0=Scalar(1)-w1;
	for(int vi=0;vi<numSteps;++vi)
		{
		Index vertexIndex=cell.index;
		for(int i=0;i<interpolationDimension;++i)
			if(vi&(1<<i))
				++vertexIndex[i];
		Vector v0=cell.grid->calcVertexGradient(vertexIndex,slice);
		++vertexIndex[interpolationDimension];
		Vector v1=cell.grid->calcVertexGradient(vertexIndex,slice);
		v[vi]=Interpolator::interpolate(v0,w0,v1,w1);
		}
	for(int i=1;i<dimension;++i)
		{
		--interpolationDimension;
		numSteps>>=1;
		w1=cellPos[interpolationDimension];
		w0=Scalar(1)-w1;
		for(int vi=0;vi<numSteps;++vi)
			v[vi]=Interpolator::interpolate(v[vi],w0,v[vi+numSteps],w1);
		}
	
	/* Return final result: */
	return v[0];
	}

/********************************
Methods of class SlicedCartesian:
********************************/

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::calcVertexGradient(
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Index& vertexIndex,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Array& slice) const
	{
	Vector result;
	const Value* vertex=slice.getAddress(vertexIndex);
	for(int i=0;i<dimension;++i)
		{
		if(vertexIndex[i]==0)
			{
			const Value* left=vertex+vertexStrides[i];
			const Value* right=left+vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(*vertex));
			Scalar f1=Scalar(extractor.getValue(*left));
			Scalar f2=Scalar(extractor.getValue(*right));
			result[i]=(Scalar(-3)*f0+Scalar(4)*f1-f2)/(Scalar(2)*cellSize[i]);
			}
		else if(vertexIndex[i]==numVertices[i]-1)
			{
			const Value* right=vertex-vertexStrides[i];
			const Value* left=right-vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(*left));
			Scalar f1=Scalar(extractor.getValue(*right));
			Scalar f2=Scalar(extractor.getValue(*vertex));
			result[i]=(f0-Scalar(4)*f1+Scalar(3)*f2)/(Scalar(2)*cellSize[i]);
			}
		else
			{
			const Value* left=vertex-vertexStrides[i];
			const Value* right=vertex+vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(*left));
			Scalar f2=Scalar(extractor.getValue(*right));
			result[i]=(f2-f0)/(Scalar(2)*cellSize[i]);
			}
		}
	return result;
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::SlicedCartesian(
	void)
	:numVertices(0),
	 numSlices(0),
	 slices(0),
	 numCells(0),
	 cellSize(Scalar(0)),
	 domainBox(Box::empty)
	{
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=0;
	
	/* Initialize vertex offset array: */
	for(int i=0;i<CellTopology::numVertices;++i)
		vertexOffsets[i]=0;
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=VertexIterator(this,vertexIndex);
	lastVertex=VertexIterator(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=CellIterator(this,cellIndex);
	lastCell=CellIterator(this,cellIndex);
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::SlicedCartesian(
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Index& sNumVertices,
	int sNumSlices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Size& sCellSize,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::VScalar* sVertexValues)
	:numVertices(sNumVertices),
	 numSlices(sNumSlices),
	 slices(0),
	 cellSize(sCellSize)
	{
	/* Initialize the slice arrays: */
	slices=new Array[numSlices];
	for(int slice=0;slice<numSlices;++slice)
		slices[slice].resize(numVertices);
	
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=slices[0].getIncrement(i);
	
	/* Calculate number of cells: */
	for(int i=0;i<dimension;++i)
		numCells[i]=numVertices[i]-1;
	
	/* Initialize vertex offset array: */
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		/* Vertex indices are, as usual, bit masks of a vertex' position in cell coordinates: */
		vertexOffsets[i]=0;
		for(int j=0;j<dimension;++j)
			if(i&(1<<j))
				vertexOffsets[i]+=vertexStrides[j];
		}
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=VertexIterator(this,vertexIndex);
	vertexIndex[0]=numVertices[0];
	lastVertex=VertexIterator(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=CellIterator(this,cellIndex);
	cellIndex[0]=numCells[0];
	lastCell=CellIterator(this,cellIndex);
	
	/* Initialize domain bounding box: */
	Point domainMax;
	for(int i=0;i<dimension;++i)
		domainMax[i]=Scalar(numCells[i])*cellSize[i];
	domainBox=Box(Point::origin,domainMax);
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		const VScalar* sPtr=sVertexValues;
		size_t totalNumVertices=slices[slice].getNumElements();
		for(int slice=0;slice<numSlices;++slice)
			{
			/* Copy all slice vertex values: */
			VScalar* vPtr=slices[slice].getArray();
			for(size_t i=0;i<totalNumVertices;++i,++vPtr,++sPtr)
				*vPtr=*sPtr;
			}
		}
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::~SlicedCartesian(
	void)
	{
	/* Delete slice arrays: */
	delete[] slices;
	}

template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
void
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::setData(
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Index& sNumVertices,
	int sNumSlices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Size& sCellSize,
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Value* sVertexValues)
	{
	/* Destroy the slice arrays: */
	if(numSlices>0)
		delete[] slices;
	
	/* Re-initialize the slice arrays: */
	numVertices=sNumVertices;
	numSlices=sNumSlices;
	slices=new Array[numSlices];
	for(int slice=0;slice<numSlices;++slice)
		slices[slice].resize(numVertices);
	
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=slices[0].getIncrement(i);
	
	/* Initialize the cell size: */
	cellSize=sCellSize;
	
	/* Calculate number of cells: */
	for(int i=0;i<dimension;++i)
		numCells[i]=numVertices[i]-1;
	
	/* Initialize vertex offset array: */
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		/* Vertex indices are, as usual, bit masks of a vertex' position in cell coordinates: */
		vertexOffsets[i]=0;
		for(int j=0;j<dimension;++j)
			if(i&(1<<j))
				vertexOffsets[i]+=vertexStrides[j];
		}
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=VertexIterator(this,vertexIndex);
	vertexIndex[0]=numVertices[0];
	lastVertex=VertexIterator(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=CellIterator(this,cellIndex);
	cellIndex[0]=numCells[0];
	lastCell=CellIterator(this,cellIndex);
	
	/* Initialize domain bounding box: */
	Point domainMax;
	for(int i=0;i<dimension;++i)
		domainMax[i]=Scalar(numCells[i])*cellSize[i];
	domainBox=Box(Point::origin,domainMax);
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		const VScalar* sPtr=sVertexValues;
		size_t totalNumVertices=slices[slice].getNumElements();
		for(int slice=0;slice<numSlices;++slice)
			{
			/* Copy all slice vertex values: */
			VScalar* vPtr=slices[slice].getArray();
			for(size_t i=0;i<totalNumVertices;++i,++vPtr,++sPtr)
				*vPtr=*sPtr;
			}
		}
	}


template <class ScalarParam,int dimensionParam,class VScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::getVertexPos(
	const typename SlicedCartesian<ScalarParam,dimensionParam,VScalarParam>::Index& vertexIndex) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		result[i]=Scalar(vertexIndex[i])*cellSize[i];
	return result;
	}

}

}
