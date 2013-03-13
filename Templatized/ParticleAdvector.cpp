/***********************************************************************
ParticleAdvector - Generic class to advect particles in data sets.
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

#define VISUALIZATION_TEMPLATIZED_PARTICLEADVECTOR_IMPLEMENTATION

#include <Templatized/ParticleAdvector.h>

namespace Visualization {

namespace Templatized {

/*********************************
Methods of class ParticleAdvector:
*********************************/

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::ParticleAdvector(
	const typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::DataSet* sDataSet,
	const typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::VectorExtractor& sVectorExtractor,
	const typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::ScalarExtractor& sScalarExtractor)
	:dataSet(sDataSet),
	 vectorExtractor(sVectorExtractor),scalarExtractor(sScalarExtractor),
	 stepSize(1.0e-4),lifeTime(1.0)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::~ParticleAdvector(
	void)
	{
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
void
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::setStepSize(
	typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::Scalar newStepSize)
	{
	stepSize=newStepSize;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
void
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::setLifeTime(
	typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::Scalar newLifeTime)
	{
	lifeTime=newLifeTime;
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
void
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::addParticle(
	const typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::Point& newPosition,
	const typename ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::Locator& newLocator)
	{
	/* Create a new particle: */
	Particle newP;
	newP.position=newPosition;
	newP.locator=newLocator;
	
	/* Locate the particle and check whether it is inside the domain: */
	if(newP.locator.locate(newP.position,true))
		{
		/* Get the particle's initial scalar value: */
		newP.value=newP.locator.calcValue(scalarExtractor);
		
		/* Set the particle's life time: */
		newP.lifeTime=lifeTime;
		
		/* Store the new particle: */
		particles.push_back(newP);
		}
	}

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
inline
void
ParticleAdvector<DataSetParam,VectorExtractorParam,ScalarExtractorParam>::advectParticles(
	void)
	{
	for(size_t i=0;i<particles.size();++i)
		{
		/* Advect the particle and check whether it is still inside the domain: */
		Particle& p=particles[i];
		bool valid=stepSize>p.lifeTime;
		if(valid)
			{
			/* Calculate first half-step vector: */
			Vector v0=Vector(p.locator.calcValue(vectorExtractor));
			v0*=stepSize*Scalar(0.5);
			
			/* Move to second evaluation point: */
			Point p1=p.position;
			p1+=v0;
			valid=p.locator.locatePoint(p1,true);
			if(valid)
				{
				/* Calculate second half-step vector: */
				Vector v1=Vector(p.locator.calcValue(vectorExtractor));
				v1*=stepSize*Scalar(0.5);
				
				/* Move to third evaluation point: */
				Point p2=p.position;
				p2+=v1;
				valid=p.locator.locatePoint(p2,true);
				if(valid)
					{
					Vector v2=Vector(p.locator.calcValue(vectorExtractor));
					v2*=stepSize;
					
					/* Move to fourth evaluation point: */
					Point p3=p.position;
					p3+=v2;
					valid=p.locator.locatePoint(p3,true);
					if(valid)
						{
						Vector v3=Vector(p.locator.calcValue(vectorExtractor));
						v3*=stepSize;
						
						/* Calculate final step vector: */
						v1*=Scalar(2);
						v2+=v1;
						v2+=v0;
						v2*=Scalar(2);
						v3+=v2;
						v3/=Scalar(6);
						
						/* Move the particle to the final position: */
						p.position+=v3;
						valid=p.locator.locatePoint(p.position,true);
						if(valid)
							{
							/* Calculate the particle's new scalar value: */
							p.value=p.locator.calcValue(scalarExtractor);
							
							/* Update the particles' life time: */
							p.lifeTime-=stepSize;
							}
						}
					}
				}
			}
		
		if(!valid)
			{
			/* Remove the particle from the list: */
			particles[i]=particles[particles.size()-1];
			particles.pop_back();
			--i;
			}
		}
	}

}

}
