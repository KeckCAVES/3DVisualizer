/***********************************************************************
ParticleAdvector - Generic class to advect particles in data sets.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_PARTICLEADVECTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_PARTICLEADVECTOR_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLVertex.h>

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class VectorExtractorParam,class ScalarExtractorParam>
class ParticleAdvector
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the streamline extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef typename DataSet::Locator Locator; // Type of data set locators
	typedef VectorExtractorParam VectorExtractor; // Type to extract vector values from a data set (to trace the streamline)
	typedef typename VectorExtractor::Vector VVector; // Value type of vector extractor
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set (to color the streamline)
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	
	private:
	struct Particle // Structure to represent advected particles
		{
		/* Elements: */
		public:
		Point position; // Particle position
		Locator locator; // Locator to evaluate the data set at the particle's position
		VScalar value; // Scalar value at particle position
		Scalar lifeTime; // Remaining life time of the particle
		};
	
	typedef GLVertex<GLfloat,1,void,0,void,GLfloat,3> Vertex; // Data type for graphical representation of particles
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the streamline extractor works on
	VectorExtractor vectorExtractor; // Vector extractor working on data set
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	Scalar stepSize; // The fixed particle advection step size
	Scalar lifeTime; // The life time for new particles
	
	/* Particle advection state: */
	std::vector<Particle> particles; // Vector of currently advected particles
	
	/* Constructors and destructors: */
	public:
	ParticleAdvector(const DataSet* sDataSet,const VectorExtractor& sVectorExtractor,const ScalarExtractor& sScalarExtractor); // Creates a particle advector for the given data set and vector and scalar extractors
	private:
	ParticleAdvector(const ParticleAdvector& source); // Prohibit copy constructor
	ParticleAdvector& operator=(const ParticleAdvector& source); // Prohibit assignment operator
	public:
	~ParticleAdvector(void); // Destroys the particle advector
	
	/* Methods: */
	const DataSet* getDataSet(void) const // Returns the data set
		{
		return dataSet;
		}
	const VectorExtractor& getVectorExtractor(void) const // Returns the vector extractor
		{
		return vectorExtractor;
		}
	VectorExtractor& getVectorExtractor(void) // Ditto
		{
		return vectorExtractor;
		}
	const ScalarExtractor& getScalarExtractor(void) const // Returns the scalar extractor
		{
		return scalarExtractor;
		}
	ScalarExtractor& getScalarExtractor(void) // Ditto
		{
		return scalarExtractor;
		}
	Scalar getStepSize(void) const // Returns the advection step size
		{
		return stepSize;
		}
	Scalar getLifeTime(void) const // Returns the particle life time
		{
		return lifeTime;
		}
	void setStepSize(Scalar newStepSize); // Sets the advection step size
	void setLifeTime(Scalar newLifeTime); // Sets the life time for new particles
	void addParticle(const Point& newPosition,const Locator& newLocator); // Adds a new particle to the advector
	void advect(void); // Advects all current particles
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_PARTICLEADVECTOR_IMPLEMENTATION
#include <Templatized/ParticleAdvector.icpp>
#endif

#endif
