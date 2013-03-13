/***********************************************************************
SlicedSlicedMultiCurvilinear - Base class for vertex-centered multi-block
curvilinear data sets containing arbitrary numbers of independent scalar
fields, combined into vector and/or tensor fields using special value
extractors.
Copyright (c) 2008-2009 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEDMULTICURVILINEAR_IMPLEMENTATION

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Matrix.h>

#include <Templatized/LinearInterpolator.h>
#include <Templatized/FindClosestPointFunctor.h>

#include <Templatized/SlicedMultiCurvilinear.h>

namespace Visualization {

namespace Templatized {

/*********************************************
Methods of class SlicedMultiCurvilinear::Grid:
*********************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Grid::Grid(
	void)
	:numVertices(0),
	 numCells(0)
	{
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=0;
	
	/* Initialize vertex offset array: */
	for(int i=0;i<CellTopology::numVertices;++i)
		{
		/* Vertex indices are, as usual, bit masks of a vertex' position in cell coordinates: */
		vertexOffsets[i]=0;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Grid::setGrid(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Index& sNumVertices,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point* sVertexPositions)
	{
	/* Initialize the vertex storage: */
	numVertices=sNumVertices;
	grid.resize(numVertices);
	
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
	
	/* Copy source vertex positions, if present: */
	if(sVertexPositions!=0)
		{
		/* Copy all grid vertex positions: */
		size_t totalNumVertices=size_t(numVertices.calcIncrement(-1));
		Point* vPtr=grid.getArray();
		for(size_t i=0;i<totalNumVertices;++i)
			vPtr[i]=sVertexPositions[i];
		}
	}

/*********************************************
Methods of class SlicedMultiCurvilinear::Cell:
*********************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Vertex
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getVertex(
	int vertexIndex) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	return Vertex(ds,gridIndex,cellVertexIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcVertexGradient(
	int vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	/* Return the vertex gradient: */
	return ds->calcVertexGradient(gridIndex,cellVertexIndex,extractor);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::EdgeID
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getEdgeID(
	int edgeIndex) const
	{
	EdgeID::Index index=EdgeID::Index(baseVertexIndex+ds->grids[gridIndex].vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]]);
	index*=dimension;
	index+=edgeIndex>>(dimension-1);
	return EdgeID(index);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcEdgePosition(
	int edgeIndex,
	SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Scalar weight) const
	{
	const Grid& grid=ds->grids[gridIndex];
	const int* vos=grid.vertexOffsets;
	const int* fvis=CellTopology::edgeVertexIndices[edgeIndex];
	const Point& v0=grid.getVertexPosition(baseVertexIndex+vos[fvis[0]]);
	const Point& v1=grid.getVertexPosition(baseVertexIndex+vos[fvis[1]]);
	return Geometry::affineCombination(v0,v1,weight);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcFaceCenter(
	int faceIndex) const
	{
	const Grid& grid=ds->grids[gridIndex];
	const int* vos=grid.vertexOffsets;
	const int* fvis=CellTopology::faceVertexIndices[faceIndex];
	typename Point::AffineCombiner fc;
	for(int j=0;j<CellTopology::numFaceVertices;++j)
		fc.addPoint(grid.getVertexPosition(baseVertexIndex+vos[fvis[j]]));
	return fc.getPoint();
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::CellID
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell::getNeighbourID(
	int neighbourIndex) const
	{
	const Grid& grid=ds->grids[gridIndex];
	int direction=neighbourIndex>>1;
	if(neighbourIndex&0x1)
		{
		if(index[direction]<grid.numCells[direction]-1)
			return CellID(CellID::Index(baseVertexIndex+grid.vertexStrides[direction]));
		else
			return ds->retrieveGridConnector(*this,neighbourIndex);
		}
	else
		{
		if(index[direction]>0)
			return CellID(CellID::Index(baseVertexIndex-grid.vertexStrides[direction]));
		else
			return ds->retrieveGridConnector(*this,neighbourIndex);
		}
	}

/************************************************
Methods of class SlicedMultiCurvilinear::Locator:
************************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::newtonRaphsonStep(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point& position)
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	/* Transform the current cell position to domain space: */
	const Grid& grid=ds->grids[gridIndex];
	const Point* baseVertex=grid.grid.getArray()+(baseVertexIndex-grid.gridBaseLinearIndex);
	
	/* Perform multilinear interpolation: */
	Point p[CellTopology::numVertices>>1]; // Array of intermediate interpolation points
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	for(int pi=0;pi<numSteps;++pi)
		{
		const Point* vPtr=baseVertex+grid.vertexOffsets[pi];
		p[pi]=Geometry::affineCombination(vPtr[0],vPtr[1],cellPos[interpolationDimension]);
		}
	for(int i=1;i<dimension;++i)
		{
		--interpolationDimension;
		numSteps>>=1;
		for(int pi=0;pi<numSteps;++pi)
			p[pi]=Geometry::affineCombination(p[pi],p[pi+numSteps],cellPos[interpolationDimension]);
		}
	
	/* Calculate f(x_i): */
	Vector fi=p[0]-position;
	
	/* Check for convergence: */
	if(fi.sqr()<epsilon2)
		return true;
	
	/* Calculate f'(x_i): */
	Matrix fpi=Matrix::zero;
	for(int i=0;i<dimension;++i)
		{
		/* Calculate cell's edge vectors for current dimension: */
		int iMask=1<<i;
		for(int v0=0;v0<CellTopology::numVertices;++v0)
			if((v0&iMask)==0)
				{
				/* Calculate edge vector and convex combination weight: */
				const Point* vPtr=baseVertex+grid.vertexOffsets[v0];
				Vector d=vPtr[grid.vertexStrides[i]]-vPtr[0];
				Scalar weight=Scalar(1);
				for(int j=0;j<dimension;++j)
					if(j!=i)
						{
						int jMask=1<<j;
						if(v0&jMask)
							weight*=cellPos[j];
						else
							weight*=Scalar(1)-cellPos[j];
						}
				
				/* Add weighted vector to Jacobian matrix: */
				for(int j=0;j<dimension;++j)
					fpi(j,i)+=d[j]*weight;
				}
		}
	
	/* Calculate the step vector as f(x_i) / f'(x_i): */
	CellPosition stepi=fi/fpi;
	
	/* Adjust the cell position: */
	for(int i=0;i<dimension;++i)
		cellPos[i]-=stepi[i];
	
	return false;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	void)
	:cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	const SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>* sDs,
	typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Scalar sEpsilon)
	:Cell(sDs),
	 epsilon(sEpsilon),epsilon2(Math::sqr(epsilon)),
	 cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::setEpsilon(
	typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	epsilon2=Math::sqr(epsilon);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::locatePoint(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point& position,
	bool traceHint)
	{
	/* If traceHint parameter is false or locator can't trace, start searching from scratch: */
	if(!traceHint||cantTrace)
		{
		/* Start searching from cell whose cell center is closest to query position: */
		FindClosestPointFunctor<CellCenter> f(position,ds->maxCellRadius2);
		ds->cellCenterTree.traverseTreeDirected(f);
		if(f.getClosestPoint()==0) // Bail out if no cell is close enough
			return false;
		
		/* Go to the found cell: */
		Cell::operator=(ds->getCell(f.getClosestPoint()->value));
		
		/* Initialize local cell position: */
		for(int i=0;i<dimension;++i)
			cellPos[i]=Scalar(0.5);
		
		/* Now we can trace: */
		cantTrace=false;
		}
	
	/* Perform Newton-Raphson iteration until it converges and the current cell contains the query point: */
	Scalar maxOut;
	CellID previousCellID; // Cell ID to detect "thrashing" between cells
	CellID currentCellID=getCellID(); // Ditto
	Scalar previousMaxMove=Scalar(0); // Reason we went into the current cell
	int iteration=0;
	for(iteration=0;iteration<10;++iteration)
		{
		/* Perform Newton-Raphson iteration in the current cell until it converges, or goes really bad: */
		while(true)
			{
			/* Do one step: */
			bool converged=newtonRaphsonStep(position);
			
			/* Check for signs of convergence failure: */
			maxOut=Scalar(0);
			for(int i=0;i<dimension;++i)
				{
				if(maxOut<-cellPos[i])
					maxOut=-cellPos[i];
				else if(maxOut<cellPos[i]-Scalar(1))
					maxOut=cellPos[i]-Scalar(1);
				}
			if(converged||maxOut>Scalar(1)) // Tolerate at most one cell size out (this is somewhat ad-hoc)
				break;
			}
		
		/* Check if the current cell contains the query position: */
		if(maxOut==Scalar(0))
			return true;
		
		/* Check if this was the first step, and we're way off: */
		if(iteration==0&&maxOut>Scalar(5))
			{
			/* We had a tracing failure; just start searching from scratch: */
			FindClosestPointFunctor<CellCenter> f(position,ds->maxCellRadius2);
			ds->cellCenterTree.traverseTreeDirected(f);
			if(f.getClosestPoint()==0) // Bail out if no cell is close enough
				{
				/* At this point, the locator is borked. Better not trace next time: */
				cantTrace=true;
				
				/* And we're outside the grid, too: */
				return false;
				}
			
			/* Go to the found cell: */
			Cell::operator=(ds->getCell(f.getClosestPoint()->value));
			previousCellID=currentCellID;
			currentCellID=f.getClosestPoint()->value;
			previousMaxMove=maxOut;
			
			/* Initialize the local cell position: */
			for(int i=0;i<dimension;++i)
				cellPos[i]=Scalar(0.5);
			
			/* Start over: */
			continue;
			}
		
		/* Otherwise, try moving to a different cell: */
		Scalar maxMove=Scalar(0);
		int moveDim=0;
		int moveDir=0;
		CellID moveCellID; // Cleverly keep track of this ID to reduce work later!
		for(int i=0;i<dimension;++i)
			{
			if(maxMove<-cellPos[i])
				{
				/* Check if we can actually move in this direction: */
				moveCellID=CellID();
				if(index[i]>0||(moveCellID=ds->retrieveGridConnector(*this,i*2+0)).isValid())
					{
					maxMove=-cellPos[i];
					moveDim=i;
					moveDir=-1;
					}
				}
			else if(maxMove<cellPos[i]-Scalar(1))
				{
				/* Check if we can actually move in this direction: */
				moveCellID=CellID();
				if(index[i]<ds->grids[gridIndex].numCells[i]-1||(moveCellID=ds->retrieveGridConnector(*this,i*2+1)).isValid())
					{
					maxMove=cellPos[i]-Scalar(1);
					moveDim=i;
					moveDir=1;
					}
				}
			}
		
		/* If we can move somewhere, do it: */
		if(moveCellID.isValid())
			{
			/* Move to another grid: */
			Cell::operator=(ds->getCell(moveCellID));
			for(int i=0;i<dimension;++i)
				cellPos[i]=Scalar(0.5);
			}
		else if(moveDir==-1)
			{
			/* Move in the same grid: */
			cellPos[moveDim]+=Scalar(1);
			--index[moveDim];
			baseVertexIndex-=ds->grids[gridIndex].vertexStrides[moveDim];
			}
		else if(moveDir==1)
			{
			/* Move in the same grid: */
			cellPos[moveDim]-=Scalar(1);
			++index[moveDim];
			baseVertexIndex+=ds->grids[gridIndex].vertexStrides[moveDim];
			}
		else
			{
			/* At this point, the locator is borked. Better not trace next time: */
			cantTrace=true;
			
			/* We're not in the current cell, and can't move anywhere else -- we're outside the grid: */
			return false;
			}
		
		/* Check if we've just moved back into the cell we just came from: */
		CellID nextCellID=getCellID();
		if(nextCellID==previousCellID&&maxMove<=previousMaxMove)
			return true;
		
		/* Check for thrashing on the next iteration step: */
		previousCellID=currentCellID;
		currentCellID=nextCellID;
		previousMaxMove=maxMove;
		}
	
	/* Just to be safe, don't trace on the next step: */
	cantTrace=true;
	
	/* Return true if the final cell contains the query position, with some fudge: */
	return maxOut<Scalar(1.0e-4);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ValueExtractorParam>
inline
typename ValueExtractorParam::DestValue
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcValue(
	const ValueExtractorParam& extractor) const
	{
	typedef typename ValueExtractorParam::DestValue DestValue;
	typedef LinearInterpolator<DestValue,Scalar> Interpolator;
	
	const Grid& grid=ds->grids[gridIndex];
	
	/* Perform multilinear interpolation: */
	DestValue v[CellTopology::numVertices>>1]; // Array of intermediate interpolation values
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	Scalar w1=cellPos[interpolationDimension];
	Scalar w0=Scalar(1)-w1;
	for(int vi=0;vi<numSteps;++vi)
		{
		ptrdiff_t vIndex=baseVertexIndex+grid.vertexOffsets[vi];
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
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcGradient(
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
		Vector v0=ds->calcVertexGradient(gridIndex,vertexIndex,extractor);
		++vertexIndex[interpolationDimension];
		Vector v1=ds->calcVertexGradient(gridIndex,vertexIndex,extractor);
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

/***************************************
Methods of class SlicedMultiCurvilinear:
***************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
template <class ScalarExtractorParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::calcVertexGradient(
	int gridIndex,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Index& vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	const Grid& grid=grids[gridIndex];
	
	/* Calculate the (transposed) Jacobian matrix of the grid transformation function and the gradient of the grid function at the vertex: */
	Matrix gridJacobian;
	Vector valueGradient;
	const Point* gridPtr=grid.grid.getArray()-grid.gridBaseLinearIndex;
	ptrdiff_t vertex=grid.getVertexLinearIndex(vertexIndex);
	for(int i=0;i<dimension;++i)
		{
		if(vertexIndex[i]==0)
			{
			ptrdiff_t left=vertex+grid.vertexStrides[i];
			ptrdiff_t right=left+grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(Scalar(-3)*gridPtr[vertex][j]+Scalar(4)*gridPtr[left][j]-gridPtr[right][j]);
			Scalar f0=Scalar(extractor.getValue(vertex));
			Scalar f1=Scalar(extractor.getValue(left));
			Scalar f2=Scalar(extractor.getValue(right));
			valueGradient[i]=Math::div2(Scalar(-3)*f0+Scalar(4)*f1-f2);
			}
		else if(vertexIndex[i]==grid.numVertices[i]-1)
			{
			ptrdiff_t right=vertex-grid.vertexStrides[i];
			ptrdiff_t left=right-grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(gridPtr[left][j]-Scalar(4)*gridPtr[right][j]+Scalar(3)*gridPtr[vertex][j]);
			Scalar f0=Scalar(extractor.getValue(left));
			Scalar f1=Scalar(extractor.getValue(right));
			Scalar f2=Scalar(extractor.getValue(vertex));
			valueGradient[i]=Math::div2(f0-Scalar(4)*f1+Scalar(3)*f2);
			}
		else
			{
			ptrdiff_t left=vertex-grid.vertexStrides[i];
			ptrdiff_t right=vertex+grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(gridPtr[right][j]-gridPtr[left][j]);
			Scalar f0=Scalar(extractor.getValue(left));
			Scalar f2=Scalar(extractor.getValue(right));
			valueGradient[i]=Math::div2(f2-f0);
			}
		}
	
	/* Return the result of applying the chain rule to the partial derivatives: */
	return Vector(valueGradient/gridJacobian);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::storeGridConnector(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell& cell,
	int faceIndex,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::CellID& otherCell)
	{
	/* Calculate the index of the grid connector: */
	int connectorIndex=cell.gridIndex*dimension*2+faceIndex;
	CellID* gc=gridConnectors[connectorIndex];
	
	const Grid& grid=grids[cell.gridIndex];
	int faceDimension=faceIndex>>1;
	
	/* Allocate the grid connector if necessary: */
	if(gc==0)
		{
		int numFaces=1;
		for(int i=0;i<dimension;++i)
			if(i!=faceDimension)
				numFaces*=grid.numCells[i];
		gc=gridConnectors[connectorIndex]=new CellID[numFaces];
		}
	
	/* Store the other cell's ID: */
	int gcIndex=0;
	for(int i=0;i<dimension;++i)
		if(i!=faceDimension)
			gcIndex=gcIndex*grid.numCells[i]+cell.index[i];
	gc[gcIndex]=otherCell;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::CellID
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::retrieveGridConnector(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Cell& cell,
	int faceIndex) const
	{
	/* Calculate the index of the grid connector: */
	int connectorIndex=cell.gridIndex*dimension*2+faceIndex;
	CellID* gc=gridConnectors[connectorIndex];
	
	if(gc==0)
		return CellID();
	else
		{
		const Grid& grid=grids[cell.gridIndex];
		int faceDimension=faceIndex>>1;

		/* Retrieve the other cell's ID: */
		int gcIndex=0;
		for(int i=0;i<dimension;++i)
			if(i!=faceDimension)
				gcIndex=gcIndex*grid.numCells[i]+cell.index[i];
		return gc[gcIndex];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::SlicedMultiCurvilinear(
	void)
	:numGrids(0),grids(0),
	 totalNumVertices(0),totalNumCells(0),
	 numSlices(0),slices(0),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::SlicedMultiCurvilinear(
	int sNumGrids)
	:numGrids(sNumGrids),grids(new Grid[numGrids]),
	 totalNumVertices(0),totalNumCells(0),
	 numSlices(0),slices(0),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	/* Initialize the grids: */
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		grids[gridIndex].gridBaseLinearIndex=0;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::SlicedMultiCurvilinear(
	int sNumGrids,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Index sNumGridVertices[],
	int sNumSlices)
	:numGrids(sNumGrids),grids(new Grid[numGrids]),
	 totalNumVertices(0),totalNumCells(0),
	 numSlices(sNumSlices),slices(new ValueScalar*[numSlices]),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	/* Initialize all grids: */
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		setGrid(gridIndex,sNumGridVertices[gridIndex]);
	
	/* Initialize the grid value slices: */
	for(int i=0;i<numSlices;++i)
		slices[i]=new ValueScalar[totalNumVertices];
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::~SlicedMultiCurvilinear(
	void)
	{
	delete[] grids;
	for(int i=0;i<numSlices;++i)
		delete[] slices[i];
	delete[] slices;
	if(gridConnectors!=0)
		{
		for(int i=0;i<numGrids*dimension*2;++i)
			delete[] gridConnectors[i];
		delete[] gridConnectors;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::setNumGrids(
	int sNumGrids)
	{
	if(sNumGrids!=numGrids)
		{
		/* Delete the previous grid structures: */
		delete[] grids;
		
		/* Allocate the new grids: */
		numGrids=sNumGrids;
		grids=new Grid[numGrids];
		
		/* Initialize the grid structures: */
		totalNumVertices=0;
		totalNumCells=0;
		
		/* Resize all value slices: */
		for(int sliceIndex=0;sliceIndex<numSlices;++sliceIndex)
			{
			delete[] slices[sliceIndex];
			slices[sliceIndex]=0;
			}
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::setGrid(
	int gridIndex,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Index& sNumVertices,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point* sVertexPositions)
	{
	/* Calculate the size change of the given grid: */
	int oldGridNumVertices=grids[gridIndex].numVertices.calcIncrement(-1);
	int newGridNumVertices=sNumVertices.calcIncrement(-1);
	totalNumVertices=totalNumVertices+newGridNumVertices-oldGridNumVertices;
	
	if(oldGridNumVertices!=newGridNumVertices&&numSlices>0)
		{
		/* Calculate the cumulative sizes of grids before and after the changed grid: */
		int preSize=0;
		for(int gi=0;gi<gridIndex;++gi)
			preSize+=grids[gi].numVertices.calcIncrement(-1);
		int postSize=0;
		for(int gi=gridIndex+1;gi<numGrids;++gi)
			postSize+=grids[gi].numVertices.calcIncrement(-1);
		
		/* Resize all existing value slices: */
		for(int sliceIndex=0;sliceIndex<numSlices;++sliceIndex)
			{
			/* Allocate the new slice: */
			ValueScalar* newSlice=totalNumVertices>0?new ValueScalar[totalNumVertices]:0;
			
			/* Copy values from the old slice: */
			ValueScalar* nPtr=newSlice;
			ValueScalar* oPtr=slices[sliceIndex];
			for(int i=0;i<preSize;++i,++nPtr,++oPtr)
				*nPtr=*oPtr;
			
			/* Skip the changed grid: */
			oPtr+=oldGridNumVertices;
			nPtr+=newGridNumVertices;
			
			for(int i=0;i<postSize;++i,++nPtr,++oPtr)
				*nPtr=*oPtr;
			
			/* Install the new slice: */
			delete[] slices[sliceIndex];
			slices[sliceIndex]=newSlice;
			}
		}
	
	/* Initialize the changed grid: */
	int oldGridNumCells=grids[gridIndex].numCells.calcIncrement(-1);
	grids[gridIndex].setGrid(sNumVertices,sVertexPositions);
	int newGridNumCells=grids[gridIndex].numCells.calcIncrement(-1);
	totalNumCells=totalNumCells+newGridNumCells-oldGridNumCells;
	
	/* Update the grid structures: */
	ptrdiff_t linearIndex=0;
	for(int gi=0;gi<numGrids;++gi)
		{
		grids[gi].gridBaseLinearIndex=linearIndex;
		linearIndex+=grids[gi].numVertices.calcIncrement(-1);
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
int
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::addGrid(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Index& sNumVertices,
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Point* sVertexPositions)
	{
	/* Increase the number of grids and copy the existing grids: */
	Grid* newGrids=new Grid[numGrids+1];
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		Grid& ng=newGrids[gridIndex];
		Grid& g=grids[gridIndex];
		ng.numVertices=g.numVertices;
		ng.grid.ownArray(g.grid.getSize(),g.grid.getArray());
		g.grid.disownArray();
		ng.gridBaseLinearIndex=g.gridBaseLinearIndex;
		for(int i=0;i<dimension;++i)
			ng.vertexStrides[i]=g.vertexStrides[i];
		ng.numCells=g.numCells;
		for(int i=0;i<CellTopology::numVertices;++i)
			ng.vertexOffsets[i]=g.vertexOffsets[i];
		}
	delete[] grids;
	++numGrids;
	grids=newGrids;
	
	/* Initialize the new grid: */
	setGrid(numGrids-1,sNumVertices,sVertexPositions);
	
	return numGrids-1;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
int
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::addSlice(
	const typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::ValueScalar* sSliceValues)
	{
	/* Create a new slice array and copy over the old slices and initialize the new slice: */
	ValueScalar** newSlices=new ValueScalar*[numSlices+1];
	for(int sliceIndex=0;sliceIndex<numSlices;++sliceIndex)
		newSlices[sliceIndex]=slices[sliceIndex];
	ValueScalar* newSlice=newSlices[numSlices]=totalNumVertices>0?new ValueScalar[totalNumVertices]:0;
	
	if(sSliceValues!=0)
		{
		/* Copy the given slice values: */
		for(size_t i=0;i<totalNumVertices;++i)
			newSlice[i]=sSliceValues[i];
		}
	
	/* Install the new slice array: */
	delete[] slices;
	++numSlices;
	slices=newSlices;
	
	return numSlices-1;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::finalizeGrid(
	void)
	{
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=Vertex(this,0,vertexIndex);
	lastVertex=Vertex(this,numGrids,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=Cell(this,0,cellIndex);
	for(int i=0;i<dimension;++i)
		cellIndex[i]=grids[numGrids-1].numCells[i]-1;
	lastCell=Cell(this,numGrids-1,cellIndex);
	++lastCell;
	
	/* Calculate bounding box of all grid vertices: */
	domainBox=Box::empty;
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		size_t numGridVertices=grids[gridIndex].grid.getNumElements();
		const Point* vPtr=grids[gridIndex].grid.getArray();
		for(size_t i=0;i<numGridVertices;++i,++vPtr)
			domainBox.addPoint(*vPtr);
		}
	
	/* Create array containing all cell centers and cell indices: */
	CellCenter* ccPtr=cellCenterTree.createTree(totalNumCells);
	
	/* Calculate all cell centers: */
	Scalar minCellRadius2(Math::Constants<Scalar>::max);
	double cellRadiusSum=0.0;
	maxCellRadius2=Scalar(0);
	for(CellIterator cIt=beginCells();cIt!=endCells();++cIt,++ccPtr)
		{
		/* Calculate cell's center point: */
		typename Point::AffineCombiner cc;
		for(int i=0;i<CellTopology::numVertices;++i)
			cc.addPoint(cIt->getVertexPosition(i));
		
		/* Calculate the cell's radius: */
		Point center=cc.getPoint();
		Scalar maxDist2=Geometry::sqrDist(center,cIt->getVertexPosition(0));
		for(int i=1;i<CellTopology::numVertices;++i)
			{
			Scalar dist2=Geometry::sqrDist(center,cIt->getVertexPosition(i));
			if(maxDist2<dist2)
				maxDist2=dist2;
			}
		if(minCellRadius2>maxDist2)
			minCellRadius2=maxDist2;
		cellRadiusSum+=Math::sqrt(double(maxDist2));
		if(maxCellRadius2<maxDist2)
			maxCellRadius2=maxDist2;
		
		/* Store cell center and pointer: */
		*ccPtr=CellCenter(center,cIt->getID());
		}
	
	/* Create the cell center tree: */
	cellCenterTree.releasePoints(4); // Let's just go ahead and use the multithreaded version
	
	/* Calculate the average cell radius: */
	avgCellRadius=Scalar(cellRadiusSum/double(totalNumCells));
	
	/* Calculate the initial locator epsilon based on the minimal cell size: */
	setLocatorEpsilon(Math::sqrt(minCellRadius2)*Scalar(1.0e-4));
	
	/* Create the array of grid connectors: */
	gridConnectors=new CellID*[numGrids*dimension*2];
	for(int i=0;i<numGrids*dimension*2;++i)
		gridConnectors[i]=0;
	
	/*********************************************************************
	Create a kd-tree of all grid boundary faces to automatically stitch
	matching grids:
	*********************************************************************/
	
	{
	/* Count the number of boundary faces to create a fixed-size kd-tree: */
	size_t totalNumBoundaryFaces=0;
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		const Grid& grid=grids[gridIndex];
		for(int i=0;i<dimension;++i)
			{
			size_t numBoundaryFaces=1;
			for(int j=0;j<dimension;++j)
				if(i!=j)
					numBoundaryFaces*=size_t(grid.numVertices[j]-1);
			totalNumBoundaryFaces+=numBoundaryFaces*2;
			}
		}
	
	/* These are the same types used for the cell center tree, but re-defining them is better: */
	typedef Geometry::ValuedPoint<Point,CellID> BoundaryFaceCenter;
	typedef Geometry::ArrayKdTree<BoundaryFaceCenter> BoundaryFaceCenterTree;
	
	/* Create the kd-tree of grid boundary faces: */
	BoundaryFaceCenterTree bfct(totalNumBoundaryFaces);
	BoundaryFaceCenter* bfcPtr=bfct.accessPoints();
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		/* Iterate through all cells in this grid: */
		const Grid& grid=grids[gridIndex];
		for(Index cellIndex(0);cellIndex[0]<grid.numCells[0];cellIndex.preInc(grid.numCells))
			{
			/* Store a face center for each grid boundary touched by this cell: */
			for(int i=0;i<dimension;++i)
				{
				if(cellIndex[i]==0)
					{
					/* Store the cell's "front" face: */
					Cell cell(this,gridIndex,cellIndex);
					(*bfcPtr)=cell.calcFaceCenter(i*2+0);
					bfcPtr->value=cell.getID();
					++bfcPtr;
					}
				if(cellIndex[i]==grid.numCells[i]-1)
					{
					/* Store the cell's "back" face: */
					Cell cell(this,gridIndex,cellIndex);
					(*bfcPtr)=cell.calcFaceCenter(i*2+1);
					bfcPtr->value=cell.getID();
					++bfcPtr;
					}
				}
			}
		}
	bfct.releasePoints(4);
	
	/* Go through all grid boundary cells again and try stitching them with opposite cells: */
	typename BoundaryFaceCenterTree::ClosePointSet cfcs(3,minCellRadius2*Scalar(1.0e-2));
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		/* Iterate through all cells in this grid: */
		const Grid& grid=grids[gridIndex];
		for(Index cellIndex(0);cellIndex[0]<grid.numCells[0];cellIndex.preInc(grid.numCells))
			{
			/* Process all grid boundary faces of this cell: */
			for(int i=0;i<dimension;++i)
				{
				if(cellIndex[i]==0)
					{
					/* Find a match for the cell's "front" face: */
					Cell cell(this,gridIndex,cellIndex);
					bfct.findClosestPoints(cell.calcFaceCenter(i*2+0),cfcs);
					if(cfcs.getNumPoints()==2)
						{
						CellID thisCellID=cell.getID();
						for(int j=0;j<2;++j)
							if(cfcs.getPoint(j).value!=thisCellID)
								{
								/* We have a winner! */
								storeGridConnector(cell,i*2+0,cfcs.getPoint(j).value);
								}
						}
					cfcs.clear();
					}
				if(cellIndex[i]==grid.numCells[i]-1)
					{
					/* Find a match for the cell's "back" face: */
					Cell cell(this,gridIndex,cellIndex);
					bfct.findClosestPoints(cell.calcFaceCenter(i*2+1),cfcs);
					if(cfcs.getNumPoints()==2)
						{
						CellID thisCellID=cell.getID();
						for(int j=0;j<2;++j)
							if(cfcs.getPoint(j).value!=thisCellID)
								{
								/* We have a winner! */
								storeGridConnector(cell,i*2+1,cfcs.getPoint(j).value);
								}
						}
					cfcs.clear();
					}
				}
			}
		}
	}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::setLocatorEpsilon(
	typename SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::Scalar newLocatorEpsilon)
	{
	/* Check the desired locator epsilon against the minimal achievable, given Scalar's limited accuracy: */
	Scalar maxAbsCoordinate=Scalar(0);
	for(int i=0;i<dimension;++i)
		{
		if(maxAbsCoordinate<Math::abs(domainBox.min[i]))
			maxAbsCoordinate=Math::abs(domainBox.min[i]);
		if(maxAbsCoordinate<Math::abs(domainBox.max[i]))
			maxAbsCoordinate=Math::abs(domainBox.max[i]);
		}
	Scalar minLocatorEpsilon=maxAbsCoordinate*Scalar(4)*Math::Constants<Scalar>::epsilon;
	if(newLocatorEpsilon<minLocatorEpsilon)
		newLocatorEpsilon=minLocatorEpsilon;
	
	/* Set the locator epsilon: */
	locatorEpsilon=newLocatorEpsilon;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::isBoundaryFace(
	int gridIndex,
	int faceIndex) const
	{
	return gridConnectors[gridIndex*dimension*2+faceIndex]==0;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam>::isInteriorFace(
	int gridIndex,
	int faceIndex) const
	{
	const CellID* gc=gridConnectors[gridIndex*dimension*2+faceIndex];
	if(gc!=0)
		{
		const Grid& grid=grids[gridIndex];
		int faceDimension=faceIndex>>1;
		size_t numFaces=1;
		for(int i=0;i<dimension;++i)
			if(i!=faceDimension)
				numFaces*=size_t(grid.numCells[i]);
		size_t numConnectedFaces=0;
		for(size_t i=0;i<numFaces;++i)
			if(gc[i].isValid())
				++numConnectedFaces;
		
		return numConnectedFaces==numFaces;
		}
	else
		return false;
	}

}

}
