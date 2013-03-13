/***********************************************************************
VolumeRenderingSamplerCartesian - Specialized volume rendering sampler
class for Cartesian data sets.
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

#ifndef VISUALIZATION_TEMPLATIZED_VOLUMERENDERINGSAMPLERCARTESIAN_INCLUDED
#define VISUALIZATION_TEMPLATIZED_VOLUMERENDERINGSAMPLERCARTESIAN_INCLUDED

#include <Templatized/VolumeRenderingSampler.h>

/* Forward declarations: */
namespace Comm {
class MulticastPipe;
}
namespace Visualization {
namespace Abstract {
class Algorithm;
}
namespace Templatized {
template <class ScalarParam,int dimensionParam,class ValueParam>
class Cartesian;
}
}

namespace Visualization {

namespace Templatized {

template <class ScalarParam,class ValueParam>
class VolumeRenderingSampler<Cartesian<ScalarParam,3,ValueParam> >
	{
	/* Embedded classes: */
	public:
	typedef Cartesian<ScalarParam,3,ValueParam> DataSet; // Type of data sets on which the sampler works
	typedef typename DataSet::Scalar Scalar; // Scalar type of domain
	typedef typename DataSet::Point Point; // Type for domain points
	typedef typename DataSet::Box Box; // Type for domain boxes
	typedef typename Box::Size Size; // Type for domain sizes
	
	/* Elements: */
	private:
	const DataSet& dataSet; // The data set from which the sampler samples
	unsigned int samplerSize[3]; // Size of the Cartesian volume
	
	/* Constructors and destructors: */
	public:
	VolumeRenderingSampler(const DataSet& sDataSet); // Creates a sampler for the given data set
	
	/* Methods: */
	const unsigned int* getSamplerSize(void) const // Returns the size of the Cartesian volume
		{
		return samplerSize;
		}
	template <class ScalarExtractorParam,class VoxelParam>
	void sample(const ScalarExtractorParam& scalarExtractor,VoxelParam* voxels,const ptrdiff_t voxelStrides[3],Comm::MulticastPipe* pipe,float percentageScale,float percentageOffset,Visualization::Abstract::Algorithm* algorithm) const; // Samples scalar values from the given scalar extractor into the given voxel block
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_VOLUMERENDERINGSAMPLERCARTESIAN_IMPLEMENTATION
#include <Templatized/VolumeRenderingSamplerCartesian.cpp>
#endif

#endif
