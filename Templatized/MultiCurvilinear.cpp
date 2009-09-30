/***********************************************************************
MultiCurvilinear - Base class for vertex-centered multi-block
curvilinear data sets containing arbitrary value types (scalars,
vectors, tensors, etc.).
Copyright (c) 2007 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_MULTICURVILINEAR_IMPLEMENTATION

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Matrix.h>

#include <Templatized/LinearInterpolator.h>
#include <Templatized/FindClosestPointFunctor.h>

#include <Templatized/MultiCurvilinear.h>

namespace Visualization {

namespace Templatized {

/***************************************
Methods of class MultiCurvilinear::Grid:
***************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Grid::Grid(
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Grid::setNumVertices(
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices)
	{
	/* Initialize the vertex storage: */
	numVertices=sNumVertices;
	vertices.resize(numVertices);
	
	/* Initialize vertex stride array: */
	for(int i=0;i<dimension;++i)
		vertexStrides[i]=vertices.getIncrement(i);
	
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
	}

/***************************************
Methods of class MultiCurvilinear::Cell:
***************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::VertexID
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getVertexID(
	int vertexIndex) const
	{
	const Grid& grid=ds->grids[gridIndex];
	// TODO: Try which version is faster
	#if 0
	return VertexID(VertexID::Index((baseVertex-grid.vertices.getArray())+grid.vertexOffsets[vertexIndex]+ds->vertexIDBases[gridIndex]));
	#else
	return VertexID(VertexID::Index(grid.vertices.calcLinearIndex(index)+grid.vertexOffsets[vertexIndex]+ds->vertexIDBases[gridIndex]));
	#endif
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Vertex
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getVertex(
	int vertexIndex) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	return Vertex(ds,gridIndex,cellVertexIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::calcVertexGradient(
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::EdgeID
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getEdgeID(
	int edgeIndex) const
	{
	const Grid& grid=ds->grids[gridIndex];
	EdgeID::Index index(baseVertex-grid.vertices.getArray());
	index+=grid.vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]];
	index+=ds->edgeIDBases[gridIndex];
	index*=dimension;
	index+=edgeIndex>>(dimension-1);
	return EdgeID(index);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Point
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::calcEdgePosition(
	int edgeIndex,
	MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar weight) const
	{
	const Grid& grid=ds->grids[gridIndex];
	const GridVertex* v0=baseVertex+grid.vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]];
	const GridVertex* v1=baseVertex+grid.vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][1]];
	return Geometry::affineCombination(v0->pos,v1->pos,weight);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Point
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::calcFaceCenter(
	int faceIndex) const
	{
	const int* vos=ds->grids[gridIndex].vertexOffsets;
	const int* fvis=CellTopology::faceVertexIndices[faceIndex];
	typename Point::AffineCombiner fc;
	for(int j=0;j<CellTopology::numFaceVertices;++j)
		fc.addPoint(baseVertex[vos[fvis[j]]].pos);
	return fc.getPoint();
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::CellID
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getNeighbourID(
	int neighbourIndex) const
	{
	const Grid& grid=ds->grids[gridIndex];
	// TODO: Try which version is faster
	#if 0
	CellID::Index baseIndex(baseVertex-grid.vertices.getArray());
	#else
	CellID::Index baseIndex(grid.vertices.calcLinearIndex(index));
	#endif
	baseIndex+=ds->cellIDBases[gridIndex];
	int direction=neighbourIndex>>1;
	if(neighbourIndex&0x1)
		{
		if(index[direction]<grid.numCells[direction]-1)
			return CellID(baseIndex+grid.vertexStrides[direction]);
		else
			return ds->retrieveGridConnector(*this,neighbourIndex);
		}
	else
		{
		if(index[direction]>0)
			return CellID(baseIndex-grid.vertexStrides[direction]);
		else
			return ds->retrieveGridConnector(*this,neighbourIndex);
		}
	}

/******************************************
Methods of class MultiCurvilinear::Locator:
******************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::newtonRaphsonStep(
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Point& position)
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	/* Transform the current cell position to domain space: */
	const Grid& grid=ds->grids[gridIndex];
	
	/* Perform multilinear interpolation: */
	Point p[CellTopology::numVertices>>1]; // Array of intermediate interpolation points
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	for(int pi=0;pi<numSteps;++pi)
		{
		const GridVertex* vPtr=baseVertex+grid.vertexOffsets[pi];
		p[pi]=Geometry::affineCombination(vPtr[0].pos,vPtr[1].pos,cellPos[interpolationDimension]);
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
				const GridVertex* vPtr=baseVertex+grid.vertexOffsets[v0];
				Vector d=vPtr[grid.vertexStrides[i]].pos-vPtr[0].pos;
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::Locator(
	void)
	:cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::Locator(
	const MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>* sDs,
	typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar sEpsilon)
	:Cell(sDs),
	 epsilon(sEpsilon),epsilon2(Math::sqr(epsilon)),
	 cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::setEpsilon(
	typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	epsilon2=Math::sqr(epsilon);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::locatePoint(
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Point& position,
	bool traceHint)
	{
	/* If traceHint parameter is false or locator is invalid, start searching from scratch: */
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
			baseVertex-=ds->grids[gridIndex].vertexStrides[moveDim];
			}
		else if(moveDir==1)
			{
			/* Move in the same grid: */
			cellPos[moveDim]-=Scalar(1);
			++index[moveDim];
			baseVertex+=ds->grids[gridIndex].vertexStrides[moveDim];
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

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ValueExtractorParam>
inline
typename ValueExtractorParam::DestValue
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::calcValue(
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
		const GridVertex* vPtr=baseVertex+grid.vertexOffsets[vi];
		v[vi]=Interpolator::interpolate(extractor.getValue(vPtr[0].value),w0,extractor.getValue(vPtr[1].value),w1);
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

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::calcGradient(
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

/*********************************
Methods of class MultiCurvilinear:
*********************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::initStructure(
	void)
	{
	/* Calculate total number of vertices and cells: */
	totalNumVertices=0;
	totalNumCells=0;
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		totalNumVertices+=grids[gridIndex].numVertices.calcIncrement(-1);
		totalNumCells+=grids[gridIndex].numCells.calcIncrement(-1);
		}
	
	/* Calculate vertex, edge, and cell ID bases: */
	vertexIDBases[0]=0;
	edgeIDBases[0]=0;
	cellIDBases[0]=0;
	for(int gridIndex=1;gridIndex<numGrids;++gridIndex)
		{
		const Grid& prevGrid=grids[gridIndex-1];
		unsigned int prevNumVertices=prevGrid.numVertices.calcIncrement(-1);
		vertexIDBases[gridIndex]=vertexIDBases[gridIndex-1]+prevNumVertices;
		edgeIDBases[gridIndex]=edgeIDBases[gridIndex-1]+prevNumVertices*dimension;
		cellIDBases[gridIndex]=cellIDBases[gridIndex-1]+prevNumVertices;
		}
	
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
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::calcVertexGradient(
	int gridIndex,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Index& vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	const Grid& grid=grids[gridIndex];
	
	/* Calculate the (transposed) Jacobian matrix of the grid transformation function and the gradient of the grid function at the vertex: */
	Matrix gridJacobian;
	Vector valueGradient;
	const GridVertex* vertex=grid.vertices.getAddress(vertexIndex);
	for(int i=0;i<dimension;++i)
		{
		if(vertexIndex[i]==0)
			{
			const GridVertex* left=vertex+grid.vertexStrides[i];
			const GridVertex* right=left+grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(Scalar(-3)*vertex->pos[j]+Scalar(4)*left->pos[j]-right->pos[j]);
			Scalar f0=Scalar(extractor.getValue(vertex->value));
			Scalar f1=Scalar(extractor.getValue(left->value));
			Scalar f2=Scalar(extractor.getValue(right->value));
			valueGradient[i]=Math::div2(Scalar(-3)*f0+Scalar(4)*f1-f2);
			}
		else if(vertexIndex[i]==grid.numVertices[i]-1)
			{
			const GridVertex* right=vertex-grid.vertexStrides[i];
			const GridVertex* left=right-grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(left->pos[j]-Scalar(4)*right->pos[j]+Scalar(3)*vertex->pos[j]);
			Scalar f0=Scalar(extractor.getValue(left->value));
			Scalar f1=Scalar(extractor.getValue(right->value));
			Scalar f2=Scalar(extractor.getValue(vertex->value));
			valueGradient[i]=Math::div2(f0-Scalar(4)*f1+Scalar(3)*f2);
			}
		else
			{
			const GridVertex* left=vertex-grid.vertexStrides[i];
			const GridVertex* right=vertex+grid.vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(right->pos[j]-left->pos[j]);
			Scalar f0=Scalar(extractor.getValue(left->value));
			Scalar f2=Scalar(extractor.getValue(right->value));
			valueGradient[i]=Math::div2(f2-f0);
			}
		}
	
	/* Return the result of applying the chain rule to the partial derivatives: */
	return Vector(valueGradient/gridJacobian);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::storeGridConnector(
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell& cell,
	int faceIndex,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::CellID& otherCell)
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::CellID
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::retrieveGridConnector(
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Cell& cell,
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::MultiCurvilinear(
	void)
	:numGrids(0),
	 grids(0),
	 vertexIDBases(0),edgeIDBases(0),cellIDBases(0),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::MultiCurvilinear(
	int sNumGrids)
	:numGrids(sNumGrids),
	 grids(new Grid[numGrids]),
	 vertexIDBases(new VertexID::Index[numGrids]),
	 edgeIDBases(new EdgeID::Index[numGrids]),
	 cellIDBases(new CellID::Index[numGrids]),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::MultiCurvilinear(
	int sNumGrids,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Index sNumGridVertices[])
	:numGrids(sNumGrids),
	 grids(new Grid[numGrids]),
	 vertexIDBases(new VertexID::Index[numGrids]),
	 edgeIDBases(new EdgeID::Index[numGrids]),
	 cellIDBases(new CellID::Index[numGrids]),
	 gridConnectors(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	/* Initialize grid structures: */
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		/* Initialize the grid size: */
		grids[gridIndex].setNumVertices(sNumGridVertices[gridIndex]);
		}
	initStructure();
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::~MultiCurvilinear(
	void)
	{
	delete[] grids;
	delete[] vertexIDBases;
	delete[] edgeIDBases;
	delete[] cellIDBases;
	if(gridConnectors!=0)
		{
		for(int i=0;i<numGrids*dimension*2;++i)
			delete[] gridConnectors[i];
		delete[] gridConnectors;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::setGrids(
	int sNumGrids)
	{
	if(sNumGrids!=numGrids)
		{
		/* Delete the previous grid structures: */
		delete[] grids;
		delete[] vertexIDBases;
		delete[] edgeIDBases;
		delete[] cellIDBases;
		
		/* Allocate the new grids: */
		numGrids=sNumGrids;
		grids=new Grid[numGrids];
		vertexIDBases=new VertexID::Index[numGrids];
		edgeIDBases=new EdgeID::Index[numGrids];
		cellIDBases=new CellID::Index[numGrids];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::setGridData(
	int gridIndex,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Point* sVertexPositions,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Value* sVertexValues)
	{
	/* Initialize grid structures: */
	grids[gridIndex].setNumVertices(sNumVertices);
	
	/* Copy source vertex positions, if present: */
	if(sVertexPositions!=0)
		{
		/* Copy all grid vertex positions: */
		int totalNumVertices=grids[gridIndex].vertices.getNumElements();
		GridVertex* vPtr=grids[gridIndex].vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].pos=sVertexPositions[i];
		}
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		/* Copy all grid vertex values: */
		int totalNumVertices=grids[gridIndex].vertices.getNumElements();
		GridVertex* vPtr=grids[gridIndex].vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].value=sVertexValues[i];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::setGridData(
	int gridIndex,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::GridVertex* sVertices)
	{
	/* Initialize grid structures: */
	grids[gridIndex].setNumVertices(sNumVertices);
	
	/* Copy source vertices, if present: */
	if(sVertices!=0)
		{
		/* Copy all grid vertices: */
		int totalNumVertices=grids[gridIndex].vertices.getNumElements();
		GridVertex* vPtr=grids[gridIndex].vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i]=sVertices[i];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::finalizeGrid(
	void)
	{
	/* Initialize grid structures: */
	initStructure();
	
	/* Calculate bounding box of all grid vertices: */
	domainBox=Box::empty;
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		int numGridVertices=grids[gridIndex].vertices.getNumElements();
		const GridVertex* vPtr=grids[gridIndex].vertices.getArray();
		for(int i=0;i<numGridVertices;++i,++vPtr)
			domainBox.addPoint(vPtr->pos);
		}
	
	/* Create array containing all cell centers and cell indices: */
	CellCenter* ccPtr=cellCenterTree.createTree(totalNumCells);
	
	/* Calculate all cell centers: */
	Scalar minCellRadius2=Math::Constants<Scalar>::max;
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
	int totalNumBoundaryFaces=0;
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		const Grid& grid=grids[gridIndex];
		for(int i=0;i<dimension;++i)
			{
			int numBoundaryFaces=1;
			for(int j=0;j<dimension;++j)
				if(i!=j)
					numBoundaryFaces*=grid.numVertices[j]-1;
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::setLocatorEpsilon(
	typename MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar newLocatorEpsilon)
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::isBoundaryFace(
	int gridIndex,
	int faceIndex) const
	{
	return gridConnectors[gridIndex*dimension*2+faceIndex]==0;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
MultiCurvilinear<ScalarParam,dimensionParam,ValueParam>::isInteriorFace(
	int gridIndex,
	int faceIndex) const
	{
	const CellID* gc=gridConnectors[gridIndex*dimension*2+faceIndex];
	if(gc!=0)
		{
		const Grid& grid=grids[gridIndex];
		int faceDimension=faceIndex>>1;
		int numFaces=1;
		for(int i=0;i<dimension;++i)
			if(i!=faceDimension)
				numFaces*=grid.numCells[i];
		int numConnectedFaces=0;
		for(int i=0;i<numFaces;++i)
			if(gc[i].isValid())
				++numConnectedFaces;
		
		return numConnectedFaces==numFaces;
		}
	else
		return false;
	}

}

}
