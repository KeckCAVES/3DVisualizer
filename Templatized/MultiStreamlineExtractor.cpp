/***********************************************************************
MultiStreamlineExtractor - Generic class to extract multiple stream
lines from data sets in parallel.
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

#define VISUALIZATION_TEMPLATIZED_MULTISTREAMLINEEXTRACTOR_IMPLEMENTATION

#include <Templatized/MultiStreamlineExtractor.h>

namespace Visualization {

namespace Templatized {

/*****************************************
Methods of class MultiStreamlineExtractor:
*****************************************/

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::cashKarpStep(
	unsigned int index,
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector& vfp1,
	typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Scalar trialStepSize,
	typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector& error)
	{
	/* Define coefficients for the Cash-Karp step: */
	// static const Scalar a2=0.2,a3=0.3,a4=0.6,a5=1.0,a6=0.875;
	static const Scalar b21=1.0/5.0;
	static const Scalar b31=3.0/40.0,b32=9.0/40.0;
	static const Scalar b41=3.0/10.0,b42=-9.0/10.0,b43=6.0/5.0;
	static const Scalar b51=-11.0/54.0,b52=5.0/2.0,b53=-70.0/27.0,b54=35.0/27.0;
	static const Scalar b61=1631.0/55296.0,b62=175.0/512.0,b63=575.0/13824.0,b64=44275.0/110592.0,b65=253.0/4096.0;
	static const Scalar c1=37.0/378.0,c3=250.0/621.0,c4=125.0/594.0,c6=512.0/1771.0;
	static const Scalar dc1=c1-2825.0/27648.0,dc3=c3-18575.0/48384.0,dc4=c4-13525.0/55296.0,dc5=-277.0/14336.0,dc6=c6-1.0/4.0;
	
	StreamlineState& ss=streamlineStates[index];
	Point pTemp;
	
	/* First step: */
	pTemp=ss.p1+vfp1*(b21*trialStepSize);
	
	/* Second step: */
	ss.locator.locatePoint(pTemp,true);
	Vector vfp2=Vector(ss.locator.calcValue(vectorExtractor));
	pTemp=ss.p1+(vfp1*b31+vfp2*b32)*trialStepSize;
	
	/* Third step: */
	ss.locator.locatePoint(pTemp,true);
	Vector vfp3=Vector(ss.locator.calcValue(vectorExtractor));
	pTemp=ss.p1+(vfp1*b41+vfp2*b42+vfp3*b43)*trialStepSize;
	
	/* Fourth step: */
	ss.locator.locatePoint(pTemp,true);
	Vector vfp4=Vector(ss.locator.calcValue(vectorExtractor));
	pTemp=ss.p1+(vfp1*b51+vfp2*b52+vfp3*b53+vfp4*b54)*trialStepSize;
	
	/* Fifth step: */
	ss.locator.locatePoint(pTemp,true);
	Vector vfp5=Vector(ss.locator.calcValue(vectorExtractor));
	pTemp=ss.p1+(vfp1*b61+vfp2*b62+vfp3*b63+vfp4*b64+vfp5*b65)*trialStepSize;
	
	/* Sixth step: */
	ss.locator.locatePoint(pTemp,true);
	Vector vfp6=Vector(ss.locator.calcValue(vectorExtractor));
	
	/* Compute the error vector: */
	error=(vfp1*dc1+vfp3*dc3+vfp4*dc4+vfp5*dc5+vfp6*dc6)*trialStepSize;
	
	/* Return the result step vector: */
	return (vfp1*c1+vfp3*c3+vfp4*c4+vfp6*c6)*trialStepSize;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
bool
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::stepStreamline(
	unsigned int index)
	{
	/* Define constants for the adaptive step: */
	static const Scalar safety=0.9;
	static const Scalar growExp=-0.2;
	static const Scalar shrinkExp=-0.25;
	static const Scalar errorCondition=1.89e-4; // Math::pow(5.0/safety,1.0/growExp);
	
	StreamlineState& ss=streamlineStates[index];
	
	/* Calculate the vector and the auxiliary scalar value at the locator's current position: */
	if(!ss.locator.locatePoint(ss.p1,true))
		return false;
	Vector vfp1=Vector(ss.locator.calcValue(vectorExtractor));
	VScalar scalar=ss.locator.calcValue(scalarExtractor);
	
	/* Store the current vertex in the streamline: */
	Vertex* vPtr=multiStreamline->getNextVertex(index);
	vPtr->texCoord[0]=scalar;
	vPtr->normal=typename Vertex::Normal(vfp1.getComponents());
	vPtr->position=typename Vertex::Position(ss.p1.getComponents());
	multiStreamline->addVertex(index);
	
	/*********************************************************************
	Integrate the streamline using an embedded adaptive-step size fourth-
	order Runge-Kutta method with Cash-Karp error correction factors:
	*********************************************************************/
	
	/* Calculate proper error scaling factors for this step: */
	Vector errorScale;
	for(int i=0;i<Vector::dimension;++i)
		errorScale[i]=Math::abs(ss.p1[i])+Math::abs(vfp1[i])*ss.stepSize+Scalar(1.0e-30);
	
	/* Initialize step size: */
	Scalar trialStepSize=ss.stepSize;
	
	/* Perform trial steps until the step size is sufficiently small: */
	while(true)
		{
		/* Perform a trial step: */
		Vector error;
		Vector step=cashKarpStep(index,vfp1,trialStepSize,error);
		
		/* Evaluate accuracy: */
		Scalar errorMax(0);
		for(int i=0;i<Vector::dimension;++i)
			{
			Scalar err=Math::abs(error[i]/errorScale[i]);
			if(errorMax<err)
				errorMax=err;
			}
		errorMax/=epsilon;
		
		/* Check for accuracy threshold: */
		if(errorMax<Scalar(1))
			{
			/* Adapt the trial step size for the next step: */
			if(errorMax>errorCondition)
				ss.stepSize=safety*trialStepSize*Math::pow(errorMax,growExp);
			else
				ss.stepSize*=Scalar(5.0); // Don't increase by more than a factor of 5
			
			/* Go to the next streamline vertex: */
			ss.p1+=step;
			
			/* Done with the step: */
			return true;
			}
		
		/* Adapt the trial step size for the next trial step: */
		Scalar tempStepSize=safety*trialStepSize*Math::pow(errorMax,shrinkExp);
		trialStepSize*=Scalar(0.1); // Don't reduce by more than a factor of 10
		if(trialStepSize<tempStepSize)
			trialStepSize=tempStepSize;
		}
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::MultiStreamlineExtractor(
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::DataSet* sDataSet,
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::VectorExtractor& sVectorExtractor,
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 vectorExtractor(sVectorExtractor),scalarExtractor(sScalarExtractor),
	 epsilon(1.0e-8),
	 numStreamlines(0),
	 streamlineStates(0),
	 multiStreamline(0)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::~MultiStreamlineExtractor(
	void)
	{
	delete[] streamlineStates;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::setEpsilon(
	typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::setNumStreamlines(
	unsigned int newNumStreamlines)
	{
	if(numStreamlines!=newNumStreamlines)
		{
		/* Delete the old state array: */
		delete[] streamlineStates;
		
		/* Initialize the state array: */
		numStreamlines=newNumStreamlines;
		streamlineStates=numStreamlines!=0?new StreamlineState[numStreamlines]:0;
		}
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::setMultiStreamline(
	typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::MultiStreamline& newMultiStreamline)
	{
	/* Set the number of streamlines according to the given multi-streamline: */
	setNumStreamlines(newMultiStreamline.getNumPolylines());
	
	/* Store the multi-streamline: */
	multiStreamline=&newMultiStreamline;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::initializeStreamline(
	unsigned int index,
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::Point& startPoint,
	const typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::Locator& startLocator,
	typename MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::Scalar startStepSize)
	{
	/* Set the streamline extraction parameters: */
	streamlineStates[index].p1=startPoint;
	streamlineStates[index].locator=startLocator;
	streamlineStates[index].stepSize=startStepSize;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::extractStreamlines(
	void)
	{
	for(unsigned int i=0;i<numStreamlines;++i)
		streamlineStates[i].valid=true;
	
	/* Integrate the streamlines until all leave the data set's domain: */
	bool anyValid;
	do
		{
		anyValid=false;
		for(unsigned int i=0;i<numStreamlines;++i)
			if(streamlineStates[i].valid)
				{
				streamlineStates[i].valid=stepStreamline(i);
				anyValid=anyValid||streamlineStates[i].valid;
				}
		}
	while(anyValid);
	multiStreamline->flush();
	
	/* Clean up: */
	multiStreamline=0;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::startStreamlines(
	void)
	{
	for(unsigned int i=0;i<numStreamlines;++i)
		streamlineStates[i].valid=true;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
template <class ContinueFunctorParam>
inline
bool
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::continueStreamlines(
	const ContinueFunctorParam& cf)
	{
	/* Integrate the streamlines until all leave the domain or the functor interrupts: */
	bool anyValid;
	do
		{
		anyValid=false;
		for(unsigned int i=0;i<numStreamlines;++i)
			if(streamlineStates[i].valid)
				{
				anyValid=true;
				streamlineStates[i].valid=stepStreamline(i);
				}
		}
	while(anyValid&&cf());
	multiStreamline->flush();
	
	return !anyValid;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class MultiStreamlineParam>
inline
void
MultiStreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,MultiStreamlineParam>::finishStreamlines(
	void)
	{
	/* Clean up: */
	multiStreamline=0;
	}

}

}
