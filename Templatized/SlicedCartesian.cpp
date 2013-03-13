/***********************************************************************
SlicedSlicedCartesian - Base class for vertex-centered cartesian data sets
containing multiple scalar-valued slices.
Copyright (c) 2006-2009 Oliver Kreylos

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

#include <Templatized/SlicedCartesian.h>

#include <Math/Math.h>

#include <Templatized/LinearInterpolator.h>

namespace Visualization {

namespace Templatized {

/****************************************
Methods of class SlicedCartesian::Vertex:
****************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Vertex::getPosition(
	void) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		result[i]=Scalar(index[i])*ds->cellSize[i];
	return result;
	}

/**************************************
Methods of class SlicedCartesian::Cell:
**************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Vertex
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getVertex(
	int vertexIndex) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	return Vertex(ds,cellVertexIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getVertexPosition(
	int vertexIndex) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		{
		int pos=index[i];
		if(vertexIndex&(1<<i))
			++pos;
		result[i]=Scalar(pos)*ds->cellSize[i];
		}
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcVertexGradient(
	int vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	/* Return the vertex gradient: */
	return ds->calcVertexGradient(cellVertexIndex,extractor);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::EdgeID
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getEdgeID(
	int edgeIndex) const
	{
	EdgeID::Index index(baseVertexIndex);
	index+=ds->vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]];
	index*=dimension;
	index+=edgeIndex>>(dimension-1);
	return EdgeID(index);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcEdgePosition(
	int edgeIndex,
	SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Scalar weight) const
	{
	int edgeBaseIndex=CellTopology::edgeVertexIndices[edgeIndex][0];
	int edgeDirection=edgeIndex>>(dimension-1);
	Point result;
	for(int i=0;i<dimension;++i)
		{
		int pos=index[i];
		if(edgeBaseIndex&(1<<i))
			++pos;
		result[i]=Scalar(pos)*ds->cellSize[i];
		}
	result[edgeDirection]+=weight*ds->cellSize[edgeDirection];
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::CellID
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getNeighbourID(
	int neighbourIndex) const
	{
	int direction=neighbourIndex>>1;
	if(neighbourIndex&0x1)
		{
		if(index[direction]<ds->numCells[direction]-1)
			return CellID(CellID::Index(baseVertexIndex+ds->vertexStrides[direction]));
		else
			return CellID();
		}
	else
		{
		if(index[direction]>0)
			return CellID(CellID::Index(baseVertexIndex-ds->vertexStrides[direction]));
		else
			return CellID();
		}
	}

/*****************************************
Methods of class SlicedCartesian::Locator:
*****************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	void)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	const SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>* sDs)
	:Cell(sDs)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Locator::locatePoint(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Point& position,
	bool traceHint)
	{
	/* Ignore traceHint parameter; it is cheaper to locate points from scratch each time */
	
	/* Locate the new position: */
	bool result=true;
	for(int i=0;i<dimension;++i)
		{
		/* Convert the position to canonical grid coordinates (cellSize == 1): */
		Scalar p=position[i]/ds->cellSize[i];
		
		/* Find the index of the cell containing the position: */
		index[i]=int(Math::floor(p));
		if(index[i]<0)
			{
			index[i]=0;
			result=false;
			}
		else if(index[i]>ds->numCells[i]-1)
			{
			index[i]=ds->numCells[i]-1;
			result=false;
			}
		
		/* Calculate the position's local coordinate inside its cell: */
		cellPos[i]=p-Scalar(index[i]);
		}
	
	/* Update the cell's base vertex index: */
	baseVertexIndex=ds->numVertices.calcOffset(index);
	
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ValueExtractorParam>
inline
typename ValueExtractorParam::DestValue
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcValue(
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
		ptrdiff_t vIndex=baseVertexIndex+ds->vertexOffsets[vi];
		v[vi]=Interpolator::interpolate(extractor.getValue(vIndex+0),w0,extractor.getValue(vIndex+1),w1);
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

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcGradient(
	const ScalarExtractorParam& extractor) const
	{
	typedef LinearInterpolator<Vector,Scalar> Interpolator;
	
	/* Perform multilinear interpolation: */
	Vector v[CellTopology::numVertices>>1]; // Array of intermediate interpolation values
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	Scalar w1=cellPos[interpolationDimension];
	Scalar w0=Scalar(1)-w1;
	for(int vi=0;vi<numSteps;++vi)
		{
		Index vertexIndex=index;
		for(int i=0;i<interpolationDimension;++i)
			if(vi&(1<<i))
				++vertexIndex[i];
		Vector v0=ds->calcVertexGradient(vertexIndex,extractor);
		++vertexIndex[interpolationDimension];
		Vector v1=ds->calcVertexGradient(vertexIndex,extractor);
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

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::calcVertexGradient(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Index& vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	Vector result;
	ptrdiff_t vertex=numVertices.calcOffset(vertexIndex);
	for(int i=0;i<dimension;++i)
		{
		if(vertexIndex[i]==0)
			{
			ptrdiff_t left=vertex+vertexStrides[i];
			ptrdiff_t right=left+vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(vertex));
			Scalar f1=Scalar(extractor.getValue(left));
			Scalar f2=Scalar(extractor.getValue(right));
			result[i]=(Scalar(-3)*f0+Scalar(4)*f1-f2)/(Scalar(2)*cellSize[i]);
			}
		else if(vertexIndex[i]==numVertices[i]-1)
			{
			ptrdiff_t right=vertex-vertexStrides[i];
			ptrdiff_t left=right-vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(left));
			Scalar f1=Scalar(extractor.getValue(right));
			Scalar f2=Scalar(extractor.getValue(vertex));
			result[i]=(f0-Scalar(4)*f1+Scalar(3)*f2)/(Scalar(2)*cellSize[i]);
			}
		else
			{
			ptrdiff_t left=vertex-vertexStrides[i];
			ptrdiff_t right=vertex+vertexStrides[i];
			Scalar f0=Scalar(extractor.getValue(left));
			Scalar f2=Scalar(extractor.getValue(right));
			result[i]=(f2-f0)/(Scalar(2)*cellSize[i]);
			}
		}
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::SlicedCartesian(
	void)
	:numVertices(0),
	 numCells(0),
	 cellSize(Scalar(0)),
	 domainBox(Box::empty),
	 numSlices(0),
	 slices(0)
	{
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=0;
	
	/* Initialize vertex offset array: */
	for(int i=0;i<CellTopology::numVertices;++i)
		vertexOffsets[i]=0;
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=Vertex(this,vertexIndex);
	lastVertex=Vertex(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=Cell(this,cellIndex);
	lastCell=Cell(this,cellIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::SlicedCartesian(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Index& sNumVertices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Size& sCellSize,
	int sNumSlices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::ValueScalar* sVertexValues)
	:slices(0)
	{
	setData(sNumVertices,sCellSize,sNumSlices,sVertexValues);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::~SlicedCartesian(
	void)
	{
	/* Delete slice arrays: */
	for(int slice=0;slice<numSlices;++slice)
		delete[] slices[slice];
	delete[] slices;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::setData(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Index& sNumVertices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Size& sCellSize,
	int sNumSlices,
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::ValueScalar* sVertexValues)
	{
	/* Set the number of vertices: */
	numVertices=sNumVertices;
	
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=numVertices.calcIncrement(i);
	
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
	
	/* Initialize the cell size: */
	cellSize=sCellSize;
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=Vertex(this,vertexIndex);
	vertexIndex[0]=numVertices[0];
	lastVertex=Vertex(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=Cell(this,cellIndex);
	cellIndex[0]=numCells[0];
	lastCell=Cell(this,cellIndex);
	
	/* Initialize domain bounding box: */
	Point domainMax;
	for(int i=0;i<dimension;++i)
		domainMax[i]=Scalar(numCells[i])*cellSize[i];
	domainBox=Box(Point::origin,domainMax);
	
	
	/* Re-initialize the slice arrays: */
	for(int slice=0;slice<numSlices;++slice)
		delete[] slices[slice];
	delete[] slices;
	numSlices=sNumSlices;
	slices=new ValueScalar*[numSlices];
	size_t totalNumVertices=size_t(numVertices.calcIncrement(-1));
	for(int slice=0;slice<numSlices;++slice)
		slices[slice]=new ValueScalar[totalNumVertices];
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		const ValueScalar* sPtr=sVertexValues;
		for(int slice=0;slice<numSlices;++slice)
			{
			/* Copy all slice vertex values: */
			ValueScalar* vPtr=slices[slice];
			for(size_t i=0;i<totalNumVertices;++i,++vPtr,++sPtr)
				*vPtr=*sPtr;
			}
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
int
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::addSlice(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::ValueScalar* sSliceValues)
	{
	/* Create a new slice array: */
	ValueScalar** newSlices=new ValueScalar*[numSlices+1];
	for(int slice=0;slice<numSlices;++slice)
		newSlices[slice]=slices[slice];
	
	/* Initialize the new slice: */
	size_t totalNumVertices=size_t(numVertices.calcIncrement(-1));
	newSlices[numSlices]=new ValueScalar[totalNumVertices];
	
	if(sSliceValues!=0)
		{
		/* Copy the given slice values: */
		ValueScalar* slicePtr=newSlices[numSlices];
		for(size_t i=0;i<totalNumVertices;++i,++slicePtr,++sSliceValues)
			*slicePtr=*sSliceValues;
		}
	
	/* Install the new slice array: */
	delete[] slices;
	++numSlices;
	slices=newSlices;
	
	return numSlices-1;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::getVertexPosition(
	const typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Index& vertexIndex) const
	{
	/* Compute vertex position on-the-fly: */
	Point result;
	for(int i=0;i<dimension;++i)
		result[i]=Scalar(vertexIndex[i])*cellSize[i];
	return result;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::Scalar
SlicedCartesian<ScalarParam,dimensionParam,ValueScalarParam>::calcAverageCellSize(
	void) const
	{
	/* Compute and return cell size: */
	Scalar size=cellSize[0];
	for(int i=1;i<dimension;++i)
		size*=cellSize[i];
	return Math::pow(size,Scalar(1)/Scalar(dimension));
	}

}

}
