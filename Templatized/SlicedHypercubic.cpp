/***********************************************************************
SlicedHypercubic - Base class for vertex-centered unstructured
hypercubic data sets containing multiple scalar-valued slices.
etc.).
Copyright (c) 2009 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEDHYPERCUBIC_IMPLEMENTATION

#include <Misc/OneTimeQueue.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Matrix.h>

#include <Templatized/LinearInterpolator.h>
#include <Templatized/FindClosestPointFunctor.h>

#include <Templatized/SlicedHypercubic.h>

namespace Visualization {

namespace Templatized {

/*******************************************
Methods of class SlicedHypercubic::GridCell:
*******************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::GridCell::GridCell(
	void)
	{
	/* Initialize neighbour indices: */
	for(int i=0;i<CellTopology::numFaces;++i)
		neighbours[i]=~CellIndex(0);
	}

/***************************************
Methods of class SlicedHypercubic::Cell:
***************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Point
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Cell::calcEdgePosition(
	int edgeIndex,
	SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Scalar weight) const
	{
	const Point& v0=ds->gridVertices[cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][0]]];
	const Point& v1=ds->gridVertices[cell->vertices[CellTopology::edgeVertexIndices[edgeIndex][1]]];
	return Geometry::affineCombination(v0,v1,weight);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
template <class ScalarExtractorParam>
inline
typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::Vector
SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::Cell::calcVertexGradient(
	int vertexIndex,
	const ScalarExtractorParam& extractor) const
	{
	/* Gather a least-squares system of linear equations describing the gradient at the cell vertex: */
	Geometry::Matrix<double,dimension,dimension> a(0.0);
	Geometry::ComponentArray<double,dimension> b(0.0);
	VertexIndex centralVertex=cell->vertices[vertexIndex];
	Geometry::Point<double,dimension> c=ds->gridVertices[centralVertex];
	double fc=extractor.getValue(centralVertex);
	
	/* Add one linear equation for each vertex connected to the query vertex by an edge: */
	Misc::HashTable<VertexIndex,void> processedVertices(17);
	Misc::OneTimeQueue<CellIndex> cellQueue(17);
	cellQueue.push(index);
	while(!cellQueue.empty())
		{
		/* Get the next cell from the queue: */
		CellIndex qcellIndex=cellQueue.front();
		const GridCell* qcell=&ds->gridCells[qcellIndex];
		cellQueue.pop();
		
		/* Find the index of the central vertex in the cell: */
		int qvertexIndex;
		for(qvertexIndex=0;qvertexIndex<CellTopology::numVertices&&qcell->vertices[qvertexIndex]!=centralVertex;++qvertexIndex)
			;
		
		/* Process all neighbours of the vertex: */
		int dimensionMask=0x1;
		for(int dim=0;dim<dimension;++dim,dimensionMask<<=1)
			{
			/* Get the neighbour vertex: */
			VertexIndex neighbour=qcell->vertices[qvertexIndex^dimensionMask];
			
			/* Check if the neighbour needs to be processed (and mark it as processed either way): */
			if(!processedVertices.setEntry(neighbour))
				{
				/* Add a linear equation for the vertex: */
				const GridVertex& n=ds->gridVertices[neighbour];
				Geometry::Vector<double,dimension> d;
				for(int i=0;i<dimension;++i)
					d[i]=double(n[i])-c[i];
				double df=double(extractor.getValue(neighbour))-fc;
				for(int i=0;i<dimension;++i)
					{
					for(int j=0;j<dimension;++j)
						a(i,j)+=d[i]*d[j];
					b[i]+=d[i]*df;
					}
				}
			
			/* Add the cell adjacent to the central vertex to the cell queue, if it exists: */
			int faceIndex=dim*2;
			if(qvertexIndex&dimensionMask)
				++faceIndex;
			if(qcell->neighbours[faceIndex]!=~CellIndex(0))
				cellQueue.push(qcell->neighbours[faceIndex]);
			}
		}
	
	/* Solve the linear system and return the gradient: */
	return Vector(b/a);
	}

/******************************************
Methods of class SlicedHypercubic::Locator:
******************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::newtonRaphsonStep(
	const typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Point& position)
	{
	typedef Geometry::Matrix<Scalar,dimension,dimension> Matrix;
	
	/* Transform the current cell position to domain space: */

	/* Perform multilinear interpolation: */
	Point p[CellTopology::numVertices>>1]; // Array of intermediate interpolation points
	int interpolationDimension=dimension-1;
	int numSteps=CellTopology::numVertices>>1;
	for(int pi=0;pi<numSteps;++pi)
		{
		const Point& v0=ds->gridVertices[cell->vertices[pi]];
		const Point& v1=ds->gridVertices[cell->vertices[pi+numSteps]];
		p[pi]=Geometry::affineCombination(v0,v1,cellPos[interpolationDimension]);
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
				int v1=v0|iMask;
				Vector d=Cell::getVertexPosition(v1)-Cell::getVertexPosition(v0);
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
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	void)
	:cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::Locator(
	const SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>* sDs,
	typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Scalar sEpsilon)
	:Cell(sDs),
	 epsilon(sEpsilon),epsilon2(Math::sqr(epsilon)),
	 cantTrace(true)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::setEpsilon(
	typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	epsilon2=Math::sqr(epsilon);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
bool
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::locatePoint(
	const typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Point& position,
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
		CellIndex moveCellIndex=~CellIndex(0); // Cleverly keep track of this index to reduce work later!
		for(int i=0;i<dimension;++i)
			{
			if(maxMove<-cellPos[i])
				{
				/* Check if we can actually move in this direction: */
				if(cell->neighbours[i*2+0]!=~CellIndex(0))
					{
					maxMove=-cellPos[i];
					moveCellIndex=cell->neighbours[i*2+0];
					}
				}
			else if(maxMove<cellPos[i]-Scalar(1))
				{
				/* Check if we can actually move in this direction: */
				if(cell->neighbours[i*2+1]!=~CellIndex(0))
					{
					maxMove=cellPos[i]-Scalar(1);
					moveCellIndex=cell->neighbours[i*2+1];
					}
				}
			}
		
		/* If we can move somewhere, do it: */
		if(moveCellIndex!=~CellIndex(0))
			{
			/* Move to the next cell: */
			index=moveCellIndex;
			cell=&ds->gridCells[index];
			for(int i=0;i<dimension;++i)
				cellPos[i]=Scalar(0.5);
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
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcValue(
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
		v[vi]=Interpolator::interpolate(extractor.getValue(cell->vertices[vi]),w0,extractor.getValue(cell->vertices[vi+numSteps]),w1);
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
typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Vector
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Locator::calcGradient(
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
		Vector v0=calcVertexGradient(vi,extractor);
		Vector v1=calcVertexGradient(vi+numSteps,extractor);
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
Methods of class SlicedHypercubic:
*********************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::resizeSlices(
	size_t newAllocatedSize)
	{
	size_t numVertices=gridVertices.size();
	for(int sliceIndex=0;sliceIndex<numSlices;++sliceIndex)
		{
		/* Create a new slice: */
		ValueScalar* newSlice=new ValueScalar[newAllocatedSize];
		
		/* Copy all existing values: */
		ValueScalar* oldSlice=slices[sliceIndex];
		for(size_t i=0;i<numVertices;++i)
			newSlice[i]=oldSlice[i];
		
		/* Install the new slice: */
		delete[] slices[sliceIndex];
		slices[sliceIndex]=newSlice;
		}
	
	allocatedSliceSize=newAllocatedSize;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::SlicedHypercubic(
	void)
	:numSlices(0),allocatedSliceSize(0),slices(0),
	 domainBox(Box::empty),
	 locatorEpsilon(Scalar(1.0e-4)),
	 gridFaces(0)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::~SlicedHypercubic(
	void)
	{
	delete gridFaces;
	for(int i=0;i<numSlices;++i)
		delete[] slices[i];
	delete[] slices;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::reserveVertices(
	size_t numVertices)
	{
	/* Prepare the grid vertex list: */
	gridVertices.reserve(numVertices);
	
	/* Make room in all existing value slices: */
	if(allocatedSliceSize<numVertices)
		resizeSlices(numVertices);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::reserveCells(
	size_t numCells)
	{
	gridCells.reserve(numCells);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::VertexID
SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::addVertex(
	const typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::Point& vertexPosition)
	{
	/* Create a new vertex: */
	VertexIndex vertexIndex=gridVertices.size();
	gridVertices.push_back(vertexPosition);
	
	/* Return new vertex' ID: */
	return VertexID(vertexIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::CellID
SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::addCell(
	const typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::VertexID cellVertices[])
	{
	/* Create a new grid cell: */
	GridCell newCell;
	for(int i=0;i<CellTopology::numVertices;++i)
		newCell.vertices[i]=cellVertices[i].getIndex();
	CellIndex cellIndex=gridCells.size();
	
	/* Connect the new cell to all existing cells: */
	if(gridFaces==0)
		gridFaces=new GridFaceHasher(101);
	for(int faceIndex=0;faceIndex<CellTopology::numFaces;++faceIndex)
		{
		/* Create a face: */
		VertexIndex faceVertices[CellTopology::numFaceVertices];
		for(int i=0;i<CellTopology::numFaceVertices;++i)
			faceVertices[i]=newCell.vertices[CellTopology::faceVertexIndices[faceIndex][i]];
		GridFace face(faceVertices);
		
		/* Check if the face already exists in the data set: */
		typename GridFaceHasher::Iterator faceIt=gridFaces->findEntry(face);
		if(faceIt.isFinished())
			{
			/* Store this face to connect to future cells: */
			gridFaces->setEntry(typename GridFaceHasher::Entry(face,std::pair<CellIndex,int>(cellIndex,faceIndex)));
			}
		else
			{
			/* Connect the face to the existing cell: */
			newCell.neighbours[faceIndex]=faceIt->getDest().first;
			gridCells[faceIt->getDest().first].neighbours[faceIt->getDest().second]=cellIndex;
			
			/* Remove the face from the hash table: */
			gridFaces->removeEntry(faceIt);
			}
		}
	
	/* Store the new grid cell: */
	gridCells.push_back(newCell);
	
	/* Return new cell's ID: */
	return CellID(cellIndex);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
int
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::addSlice(
	const typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::ValueScalar* sSliceValues)
	{
	/* Create a new slice array and copy over the old slices and initialize the new slice: */
	ValueScalar** newSlices=new ValueScalar*[numSlices+1];
	for(int sliceIndex=0;sliceIndex<numSlices;++sliceIndex)
		newSlices[sliceIndex]=slices[sliceIndex];
	newSlices[numSlices]=allocatedSliceSize>0?new ValueScalar[allocatedSliceSize]:0;
	
	if(sSliceValues!=0)
		{
		/* Copy the given slice values: */
		size_t numVertices=gridVertices.size();
		ValueScalar* slicePtr=newSlices[numSlices];
		for(size_t i=0;i<numVertices;++i,++slicePtr,++sSliceValues)
			*slicePtr=*sSliceValues;
		}
	
	/* Install the new slice array: */
	delete[] slices;
	++numSlices;
	slices=newSlices;
	
	return numSlices-1;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::setVertexValue(
	int sliceIndex,
	typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::VertexIndex vertexIndex,
	typename SlicedHypercubic<ScalarParam,dimensionParam,ValueParam>::ValueScalar newValue)
	{
	/* Ensure that there is enough room in the slice arrays: */
	if(allocatedSliceSize<=vertexIndex)
		resizeSlices((size_t(vertexIndex)*5)/4+10);
	
	/* Store the vertex value: */
	slices[sliceIndex][vertexIndex]=newValue;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::finalizeGrid(
	void)
	{
	/* Delete the grid face hasher: */
	delete gridFaces;
	gridFaces=0;
	
	/* Initialize vertex list bounds: */
	VertexIndex numVertices=gridVertices.size();
	firstVertex=Vertex(this,0);
	lastVertex=Vertex(this,numVertices);
	
	/* Calculate bounding box of all grid vertices: */
	domainBox=Box::empty;
	for(typename GridVertexList::const_iterator gvIt=gridVertices.begin();gvIt!=gridVertices.end();++gvIt)
		domainBox.addPoint(*gvIt);
	
	/* Initialize cell list bounds: */
	CellIndex numCells=gridCells.size();
	firstCell=Cell(this,0);
	lastCell=Cell(this,numCells);
	
	/* Create array containing all cell centers and cell indices: */
	CellCenter* ccPtr=cellCenterTree.createTree(numCells);
	
	/* Calculate all cell centers: */
	Scalar minCellRadius2=Math::Constants<Scalar>::max;
	double cellRadiusSum=0.0;
	maxCellRadius2=Scalar(0);
	for(CellIterator cIt=firstCell;cIt!=lastCell;++cIt)
		{
		/* Calculate cell's center point: */
		typename Point::AffineCombiner cc;
		for(int i=0;i<CellTopology::numVertices;++i)
			cc.addPoint(cIt->getVertexPosition(i));
		Point center=cc.getPoint();
		
		/* Calculate the cell's radius: */
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
		++ccPtr;
		}
	
	/* Create the cell center tree: */
	cellCenterTree.releasePoints(4); // Let's just go ahead and use the multithreaded version
	
	/* Calculate the average cell radius: */
	avgCellRadius=Scalar(cellRadiusSum/double(numCells));
	
	/* Calculate the initial locator epsilon based on the minimal cell size: */
	locatorEpsilon=Math::sqrt(minCellRadius2)*Scalar(1.0e-4);
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::setLocatorEpsilon(
	typename SlicedHypercubic<ScalarParam,dimensionParam,ValueScalarParam>::Scalar newLocatorEpsilon)
	{
	locatorEpsilon=newLocatorEpsilon;
	}

}

}
