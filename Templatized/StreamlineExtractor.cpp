/***********************************************************************
StreamlineExtractor - Generic class to extract stream lines from data
sets.
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

#define VISUALIZATION_TEMPLATIZED_STREAMLINEEXTRACTOR_IMPLEMENTATION

#include <Templatized/StreamlineExtractor.h>

namespace Visualization {

namespace Templatized {

/************************************
Methods of class StreamlineExtractor:
************************************/

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
bool
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::cashKarpStep(
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector& vfp1,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Scalar trialStepSize,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector& step,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Vector& error)
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
	
	Point pTemp;
	
	/* First step: */
	pTemp=p1+vfp1*(b21*trialStepSize);
	
	/* Second step: */
	if(!locator.locatePoint(pTemp,true))
		return false;
	Vector vfp2=Vector(locator.calcValue(vectorExtractor));
	pTemp=p1+(vfp1*b31+vfp2*b32)*trialStepSize;
	
	/* Third step: */
	if(!locator.locatePoint(pTemp,true))
		return false;
	Vector vfp3=Vector(locator.calcValue(vectorExtractor));
	pTemp=p1+(vfp1*b41+vfp2*b42+vfp3*b43)*trialStepSize;
	
	/* Fourth step: */
	if(!locator.locatePoint(pTemp,true))
		return false;
	Vector vfp4=Vector(locator.calcValue(vectorExtractor));
	pTemp=p1+(vfp1*b51+vfp2*b52+vfp3*b53+vfp4*b54)*trialStepSize;
	
	/* Fifth step: */
	if(!locator.locatePoint(pTemp,true))
		return false;
	Vector vfp5=Vector(locator.calcValue(vectorExtractor));
	pTemp=p1+(vfp1*b61+vfp2*b62+vfp3*b63+vfp4*b64+vfp5*b65)*trialStepSize;
	
	/* Sixth step: */
	if(!locator.locatePoint(pTemp,true))
		return false;
	Vector vfp6=Vector(locator.calcValue(vectorExtractor));
	
	/* Compute the error vector: */
	for(int i=0;i<dimension;++i)
		error[i]=(vfp1[i]*dc1+vfp3[i]*dc3+vfp4[i]*dc4+vfp5[i]*dc5+vfp6[i]*dc6)*trialStepSize;
	
	/* Compute the result step vector: */
	for(int i=0;i<dimension;++i)
		step[i]=(vfp1[i]*c1+vfp3[i]*c3+vfp4[i]*c4+vfp6[i]*c6)*trialStepSize;
	
	return true;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
bool
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::stepStreamline(
	void)
	{
	/* Define constants for the adaptive step: */
	static const Scalar safety=0.9;
	static const Scalar growExp=-0.2;
	static const Scalar shrinkExp=-0.25;
	static const Scalar errorCondition=1.89e-4; // Math::pow(5.0/safety,1.0/growExp);
	
	/* Calculate the vector and the auxiliary scalar value at the locator's current position: */
	if(!locator.locatePoint(p1,true))
		return false;
	Vector vfp1=Vector(locator.calcValue(vectorExtractor));
	VScalar scalar=locator.calcValue(scalarExtractor);
	
	/* Store the current vertex in the streamline: */
	Vertex* vPtr=streamline->getNextVertex();
	vPtr->texCoord[0]=scalar;
	vPtr->normal=typename Vertex::Normal(vfp1.getComponents());
	vPtr->position=typename Vertex::Position(p1.getComponents());
	streamline->addVertex();
	
	/*********************************************************************
	Integrate the streamline using an embedded adaptive-step size fourth-
	order Runge-Kutta method with Cash-Karp error correction factors:
	*********************************************************************/
	
	/* Calculate proper error scaling factors for this step: */
	Vector errorScale;
	for(int i=0;i<Vector::dimension;++i)
		errorScale[i]=Math::abs(p1[i])+Math::abs(vfp1[i])*stepSize+Scalar(1.0e-30);
	
	/* Initialize step size: */
	Scalar trialStepSize=stepSize;
	
	/* Perform trial steps until the step size is sufficiently small: */
	while(true)
		{
		/* Perform a trial step: */
		Vector step,error;
		if(!cashKarpStep(vfp1,trialStepSize,step,error)) // Trial step left the domain!
			{
			/* Try a few more times, and reduce the trial step size each time: */
			trialStepSize*=Scalar(0.5);
			int i;
			for(i=0;i<10&&!cashKarpStep(vfp1,trialStepSize,step,error);++i)
				trialStepSize*=Scalar(0.5);
			
			/* Bail out if the trial step still leaves the domain: */
			if(i>=10)
				return false;
			}
		
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
				stepSize=safety*trialStepSize*Math::pow(errorMax,growExp);
			else
				stepSize*=Scalar(5.0); // Don't increase by more than a factor of 5
			
			/* Go to the next streamline vertex: */
			p1+=step;
			
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

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::StreamlineExtractor(
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::DataSet* sDataSet,
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::VectorExtractor& sVectorExtractor,
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 vectorExtractor(sVectorExtractor),scalarExtractor(sScalarExtractor),
	 epsilon(1.0e-8),
	 streamline(0)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::~StreamlineExtractor(
	void)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
void
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::setEpsilon(
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Scalar newEpsilon)
	{
	epsilon=newEpsilon;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
void
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::extractStreamline(
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Point& startPoint,
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Locator& startLocator,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Scalar startStepSize,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Streamline& newStreamline)
	{
	/* Set the streamline extraction parameters: */
	p1=startPoint;
	locator=startLocator;
	stepSize=startStepSize;
	streamline=&newStreamline;
	
	/* Integrate the streamline until it leaves the data set's domain: */
	while(stepStreamline())
		;
	streamline->flush();
	
	/* Clean up: */
	streamline=0;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
void
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::startStreamline(
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Point& startPoint,
	const typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Locator& startLocator,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Scalar startStepSize,
	typename StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::Streamline& newStreamline)
	{
	/* Set the streamline extraction parameters: */
	p1=startPoint;
	locator=startLocator;
	stepSize=startStepSize;
	streamline=&newStreamline;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
template <class ContinueFunctorParam>
inline
bool
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::continueStreamline(
	const ContinueFunctorParam& cf)
	{
	/* Integrate the streamline until it leaves the domain or the functor interrupts: */
	bool inDomain;
	do
		{
		inDomain=stepStreamline();
		}
	while(inDomain&&cf());
	streamline->flush();
	
	return !inDomain;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam,class StreamlineParam>
inline
void
StreamlineExtractor<DataSetParam,VectorExtractorParam,ScalarExtractorParam,StreamlineParam>::finishStreamline(
	void)
	{
	/* Clean up: */
	streamline=0;
	}

}

}
