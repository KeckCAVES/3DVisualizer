/***********************************************************************
StreamsurfaceExtractor - Class to extract stream surfaces from data
sets.
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

#define VISUALIZATION_TEMPLATIZED_STREAMSURFACEEXTRACTOR_IMPLEMENTATION

#include <Templatized/StreamsurfaceExtractor.h>

namespace Visualization {

namespace Templatized {

/***************************************
Methods of class StreamsurfaceExtractor:
***************************************/

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
bool
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::stepStreamline(
	typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Streamline& s)
	{
	/* Calculate the vector and the auxiliary scalar value at the locator's current position: */
	s.pos0=s.pos1;
	if(!s.locator.locatePoint(s.pos0,true))
		return false;
	s.vec0=Vector(s.locator.calcValue(vectorExtractor));
	s.scalar0=s.locator.calcValue(scalarExtractor);
	
	/****************************************************************
	Integrate the streamline using a fourth-order Runge-Kutta method:
	****************************************************************/
	
	/* Calculate the first half-step vector: */
	Vector v0=s.vec0*(stepSize*Scalar(0.5));
	
	/* Move to the second evaluation point: */
	Point p1=s.pos0;
	p1+=v0;
	
	/* Calculate the second half-step vector: */
	s.locator.locatePoint(p1,true);
	Vector v1=Vector(s.locator.calcValue(vectorExtractor));
	v1*=stepSize*Scalar(0.5);
	
	/* Move to the third evaluation point: */
	Point p2=s.pos0;
	p2+=v1;
	
	/* Calculate the third half-step vector: */
	s.locator.locatePoint(p2,true);
	Vector v2=Vector(s.locator.calcValue(vectorExtractor));
	v2*=stepSize;
	
	/* Move to the fourth evaluation point: */
	Point p3=s.pos0;
	p3+=v2;
	
	/* Calculate the fourth half-step vector: */
	s.locator.locatePoint(p3,true);
	Vector v3=Vector(s.locator.calcValue(vectorExtractor));
	v3*=stepSize;
	
	/* Calculate the step vector: */
	v1*=Scalar(2);
	v2+=v1;
	v2+=v0;
	v2*=Scalar(2);
	v3+=v2;
	v3/=Scalar(6);
	
	/* Go to the next streamline vertex: */
	s.pos1+=v3;
	
	return true;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
bool
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::stepStreamsurface(
	void)
	{
	/* Advance all streamlines: */
	Streamline* s=streamlineHead;
	do
		{
		/* Advance the streamline: */
		stepStreamline(s);
		}
	while(s!=streamlineHead);
	
	
	
	
	
	
	
	
	
	
	
	
	if(allValid)
		{
		/* Store the new layer of streamline vertices in the stream surface: */
		Vertex* vPtr0=streamsurface->getNextVertex();
		vPtr0->texCoord[0]=scalars[0];
		Vector normal0;
		if(closed)
			normal0=Geometry::cross(positions[1]-positions[numStreamlines-1],tangents[0]);
		else
			normal0=Geometry::cross(positions[1]-positions[0],tangents[0]);
		normal0.normalize();
		vPtr0->normal=typename Vertex::Normal(normal0.getComponents());
		vPtr0->position=typename Vertex::Position(positions[0].getComponents());
		streamsurface->addVertex();
		for(int index=1;index<numStreamlines-1;++index)
			{
			Vertex* vPtr=streamsurface->getNextVertex();
			vPtr->texCoord[0]=scalars[index];
			Vector normal=Geometry::cross(positions[index+1]-positions[index-1],tangents[index]);
			normal.normalize();
			vPtr->normal=typename Vertex::Normal(normal.getComponents());
			vPtr->position=typename Vertex::Position(positions[index].getComponents());
			streamsurface->addVertex();
			}
		Vertex* vPtrn=streamsurface->getNextVertex();
		vPtrn->texCoord[0]=scalars[numStreamlines-1];
		Vector normaln;
		if(closed)
			normaln=Geometry::cross(positions[0]-positions[numStreamlines-2],tangents[numStreamlines-1]);
		else
			normaln=Geometry::cross(positions[numStreamlines-1]-positions[numStreamlines-2],tangents[numStreamlines-1]);
		normaln.normalize();
		vPtrn->normal=typename Vertex::Normal(normaln.getComponents());
		vPtrn->position=typename Vertex::Position(positions[numStreamlines-1].getComponents());
		streamsurface->addVertex();
		
		if(layerIndex>0)
			{
			/* Create indices for the triangle strips connecting the current streamlines positions to the previous ones: */
			int layer0=(layerIndex-1)*numStreamlines;
			int layer1=layerIndex*numStreamlines;
			for(int i=0;i<numStreamlines;++i)
				{
				streamsurface->addIndex(layer1+i);
				streamsurface->addIndex(layer0+i);
				}
			if(closed)
				{
				streamsurface->addIndex(layer1);
				streamsurface->addIndex(layer0);
				}
			streamsurface->addStrip();
			}
		}
	
	return allValid;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::StreamsurfaceExtractor(
	const typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::DataSet* sDataSet,
	const typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::VectorExtractor& sVectorExtractor,
	const typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 vectorExtractor(sVectorExtractor),scalarExtractor(sScalarExtractor),
	 stepSize(0.1),
	 numStreamlines(0),
	 p0s(0),locators(0),positions(0),tangents(0),scalars(0),
	 streamsurface(0)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::~StreamsurfaceExtractor(
	void)
	{
	delete[] p0s;
	delete[] locators;
	delete[] positions;
	delete[] tangents;
	delete[] scalars;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::setStepSize(
	typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Scalar newStepSize)
	{
	stepSize=newStepSize;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::setNumStreamlines(
	int newNumStreamlines)
	{
	if(newNumStreamlines!=numStreamlines)
		{
		/* Delete the old state arrays: */
		delete[] p0s;
		delete[] locators;
		delete[] positions;
		delete[] tangents;
		delete[] scalars;
		
		/* Set the new number of streamlines: */
		numStreamlines=newNumStreamlines;
		
		/* Initialize the state arrays: */
		if(newNumStreamlines!=0)
			{
			/* Allocate the state arrays: */
			p0s=new Point[numStreamlines];
			locators=new Locator[numStreamlines];
			positions=new Point[numStreamlines];
			tangents=new Vector[numStreamlines];
			scalars=new VScalar[numStreamlines];
			}
		else
			{
			/* Set the state arrays to invalid: */
			p0s=0;
			locators=0;
			positions=0;
			tangents=0;
			scalars=0;
			}
		}
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::setClosed(
	bool newClosed)
	{
	closed=newClosed;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::initializeStreamline(
	int index,
	const typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Point& startPoint,
	const typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Locator& startLocator)
	{
	/* Set the streamline extraction parameters: */
	p0s[index]=startPoint;
	locators[index]=startLocator;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::extractStreamsurface(
	typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Streamsurface& newStreamsurface)
	{
	streamsurface=&newStreamsurface;
	layerIndex=0;
	
	/* Integrate the streamlines until one leaves the data set's domain: */
	while(stepStreamsurface())
		++layerIndex;
	
	/* Clean up: */
	streamsurface=0;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::startStreamsurface(
	typename StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::Streamsurface& newStreamsurface)
	{
	streamsurface=&newStreamsurface;
	layerIndex=0;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
template <class ContinueFunctorParam>
inline
bool
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::continueStreamsurface(
	const ContinueFunctorParam& cf)
	{
	/* Integrate the streamlines until all leave the domain or the functor interrupts: */
	bool valid;
	while((valid=stepStreamsurface())&&cf())
		++layerIndex;
	
	return !valid;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamsurfaceParam>
inline
void
StreamsurfaceExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamsurfaceParam>::finishStreamsurface(
	void)
	{
	/* Clean up: */
	streamsurface=0;
	}

}

}
