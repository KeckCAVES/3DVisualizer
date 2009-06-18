/***********************************************************************
Curvilinear - Base class for vertex-centered curvilinear data sets
containing arbitrary value types (scalars, vectors, tensors, etc.).
Copyright (c) 2004-2009 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_CURVILINEAR_IMPLEMENTATION

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Matrix.h>

#include <Templatized/LinearInterpolator.h>
#include <Templatized/FindClosestPointFunctor.h>

#include <Templatized/Curvilinear.h>

namespace Visualization {

namespace Templatized {

/**********************************
Methods of class Curvilinear::Cell:
**********************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::VertexID
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getVertexID(
	int vertexIndex) const
	{
	// TODO: Try which version is faster
	#if 0
	return VertexID(VertexID::Index((baseVertex-ds->vertices.getArray())+ds->vertexOffsets[vertexIndex]));
	#else
	return VertexID(VertexID::Index(ds->vertices.calcLinearIndex(index)+ds->vertexOffsets[vertexIndex]));
	#endif
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Vertex
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getVertex(
	int vertexIndex) const
	{
	/* Calculate the index of the cell vertex: */
	Index cellVertexIndex=index;
	for(int i=0;i<dimension;++i)
		if(vertexIndex&(1<<i))
			++cellVertexIndex[i];
	
	return Vertex(ds,cellVertexIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::calcVertexGradient(
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

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::EdgeID
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getEdgeID(
	int edgeIndex) const
	{
	EdgeID::Index index(baseVertex-ds->vertices.getArray());
	index+=ds->vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]];
	index*=dimension;
	index+=edgeIndex>>(dimension-1);
	return EdgeID(index);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Point
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::calcEdgePosition(
	int edgeIndex,
	Curvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar weight) const
	{
	const GridVertex* v0=baseVertex+ds->vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][0]];
	const GridVertex* v1=baseVertex+ds->vertexOffsets[CellTopology::edgeVertexIndices[edgeIndex][1]];
	return Geometry::affineCombination(v0->pos,v1->pos,weight);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::CellID
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Cell::getNeighbourID(
	int neighbourIndex) const
	{
	// TODO: Try which version is faster
	#if 0
	CellID::Index baseIndex(baseVertex-ds->vertices.getArray());
	#else
	CellID::Index baseIndex(ds->vertices.calcLinearIndex(index));
	#endif
	int direction=neighbourIndex>>1;
	if(neighbourIndex&0x1)
		{
		if(index[direction]<ds->numCells[direction]-1)
			return CellID(baseIndex+ds->vertexStrides[direction]);
		else
			return CellID();
		}
	else
		{
		if(index[direction]>0)
			return CellID(baseIndex-ds->vertexStrides[direction]);
		else
			return CellID();
		}
	}

/*************************************
Methods of class Curvilinear::Locator:
*************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::newtonRaphsonStep(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Point& position)
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	/* Transform the current cell position to domain space: */
	
	/* Perform multilinear interpolation: */
	Point p[CellTopology::numVertices>>1]; // Array of intermediate interpolation points
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	for(int pi=0;pi<numSteps;++pi)
		{
		const GridVertex* vPtr=baseVertex+ds->vertexOffsets[pi];
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
				const GridVertex* vPtr=baseVertex+ds->vertexOffsets[v0];
				Vector d=vPtr[ds->vertexStrides[i]].pos-vPtr[0].pos;
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
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::Locator(
	void)
	:cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::Locator(
	const Curvilinear<ScalarParam,dimensionParam,ValueParam>* sDs,
	typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar sEpsilon)
	:Cell(sDs),
	 epsilon(sEpsilon),epsilon2(Math::sqr(epsilon)),
	 cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::setEpsilon(
	typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	epsilon2=Math::sqr(epsilon);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
bool
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::locatePoint(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Point& position,
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
		for(int i=0;i<dimension;++i)
			{
			if(maxMove<-cellPos[i])
				{
				/* Check if we can actually move in this direction: */
				if(index[i]>0)
					{
					maxMove=-cellPos[i];
					moveDim=i;
					moveDir=-1;
					}
				}
			else if(maxMove<cellPos[i]-Scalar(1))
				{
				/* Check if we can actually move in this direction: */
				if(index[i]<ds->numCells[moveDim]-1)
					{
					maxMove=cellPos[i]-Scalar(1);
					moveDim=i;
					moveDir=1;
					}
				}
			}
		
		/* If we can move somewhere, do it: */
		if(moveDir==-1)
			{
			cellPos[moveDim]+=Scalar(1);
			--index[moveDim];
			baseVertex-=ds->vertexStrides[moveDim];
			}
		else if(moveDir==1)
			{
			cellPos[moveDim]-=Scalar(1);
			++index[moveDim];
			baseVertex+=ds->vertexStrides[moveDim];
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
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::calcValue(
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
		const GridVertex* vPtr=baseVertex+ds->vertexOffsets[vi];
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
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Locator::calcGradient(
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

/****************************
Methods of class Curvilinear:
****************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::initStructure(
	void)
	{
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
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Vector
Curvilinear<ScalarParam,dimensionParam,ValueParam>::calcVertexGradient(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Index& vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	/* Calculate the (transposed) Jacobian matrix of the grid transformation function and the gradient of the grid function at the vertex: */
	Matrix gridJacobian;
	Vector valueGradient;
	const GridVertex* vertex=vertices.getAddress(vertexIndex);
	for(int i=0;i<dimension;++i)
		{
		if(vertexIndex[i]==0)
			{
			const GridVertex* left=vertex+vertexStrides[i];
			const GridVertex* right=left+vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(Scalar(-3)*vertex->pos[j]+Scalar(4)*left->pos[j]-right->pos[j]);
			Scalar f0=Scalar(extractor.getValue(vertex->value));
			Scalar f1=Scalar(extractor.getValue(left->value));
			Scalar f2=Scalar(extractor.getValue(right->value));
			valueGradient[i]=Math::div2(Scalar(-3)*f0+Scalar(4)*f1-f2);
			}
		else if(vertexIndex[i]==numVertices[i]-1)
			{
			const GridVertex* right=vertex-vertexStrides[i];
			const GridVertex* left=right-vertexStrides[i];
			for(int j=0;j<dimension;++j)
				gridJacobian(i,j)=Math::div2(left->pos[j]-Scalar(4)*right->pos[j]+Scalar(3)*vertex->pos[j]);
			Scalar f0=Scalar(extractor.getValue(left->value));
			Scalar f1=Scalar(extractor.getValue(right->value));
			Scalar f2=Scalar(extractor.getValue(vertex->value));
			valueGradient[i]=Math::div2(f0-Scalar(4)*f1+Scalar(3)*f2);
			}
		else
			{
			const GridVertex* left=vertex-vertexStrides[i];
			const GridVertex* right=vertex+vertexStrides[i];
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
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Curvilinear(
	void)
	:numVertices(0),
	 numCells(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4))
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
	
	/* Initialize vertex list bounds: */
	Index vertexIndex(0);
	firstVertex=Vertex(this,vertexIndex);
	lastVertex=Vertex(this,vertexIndex);
	
	/* Initialize cell list bounds: */
	Index cellIndex(0);
	firstCell=Cell(this,cellIndex);
	lastCell=Cell(this,cellIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Curvilinear(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Point* sVertexPositions,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Value* sVertexValues)
	:numVertices(sNumVertices),vertices(sNumVertices),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	initStructure();
	
	/* Copy source vertex positions, if present: */
	if(sVertexPositions!=0)
		{
		/* Copy all grid vertex positions: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].pos=sVertexPositions[i];
		
		/* Finalize grid structure: */
		finalizeGrid();
		}
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		/* Copy all grid vertex values: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].value=sVertexValues[i];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Curvilinear<ScalarParam,dimensionParam,ValueParam>::Curvilinear(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::GridVertex* sVertices)
	:numVertices(sNumVertices),vertices(sNumVertices),
	 locatorEpsilon(Scalar(1.0e-4))
	{
	initStructure();
	
	/* Copy source vertices, if present: */
	if(sVertices!=0)
		{
		/* Copy all grid vertices: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i]=sVertices[i];
		
		/* Finalize grid structure: */
		finalizeGrid();
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
Curvilinear<ScalarParam,dimensionParam,ValueParam>::~Curvilinear(
	void)
	{
	/* Nothing to do, incidentally... */
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::setData(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Point* sVertexPositions,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Value* sVertexValues)
	{
	/* Resize the vertex array: */
	numVertices=sNumVertices;
	vertices.resize(numVertices);
	
	initStructure();
	
	/* Copy source vertex positions, if present: */
	if(sVertexPositions!=0)
		{
		/* Copy all grid vertex positions: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].pos=sVertexPositions[i];
		
		/* Finalize grid structure: */
		finalizeGrid();
		}
	
	/* Copy source vertex values, if present: */
	if(sVertexValues!=0)
		{
		/* Copy all grid vertex values: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i].value=sVertexValues[i];
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::setData(
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Index& sNumVertices,
	const typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::GridVertex* sVertices)
	{
	/* Resize the vertex array: */
	numVertices=sNumVertices;
	vertices.resize(numVertices);
	
	initStructure();
	
	/* Copy source vertices, if present: */
	if(sVertices!=0)
		{
		/* Copy all grid vertices: */
		int totalNumVertices=vertices.getNumElements();
		GridVertex* vPtr=vertices.getArray();
		for(int i=0;i<totalNumVertices;++i)
			vPtr[i]=sVertices[i];
		
		/* Finalize grid structure: */
		finalizeGrid();
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::finalizeGrid(
	void)
	{
	/* Calculate bounding box of all grid vertices: */
	domainBox=Box::empty;
	int totalNumVertices=vertices.getNumElements();
	const GridVertex* vPtr=vertices.getArray();
	for(int i=0;i<totalNumVertices;++i,++vPtr)
		domainBox.addPoint(vPtr->pos);
	
	/* Create array containing all cell centers and cell indices: */
	CellCenter* ccPtr=cellCenterTree.createTree(numCells.calcIncrement(-1));
	
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
	avgCellRadius=Scalar(cellRadiusSum/double(numCells.calcIncrement(-1)));
	
	/* Calculate the initial locator epsilon based on the minimal cell size: */
	setLocatorEpsilon(Math::sqrt(minCellRadius2)*Scalar(1.0e-4));
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
Curvilinear<ScalarParam,dimensionParam,ValueParam>::setLocatorEpsilon(
	typename Curvilinear<ScalarParam,dimensionParam,ValueParam>::Scalar newLocatorEpsilon)
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

}

}
