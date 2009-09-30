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

#define VISUALIZATION_TEMPLATIZED_VOLUMERENDERINGSAMPLERCARTESIAN_IMPLEMENTATION

#include <Templatized/VolumeRenderingSamplerCartesian.h>

#include <Abstract/Algorithm.h>
#include <Templatized/Cartesian.h>

namespace Visualization {

namespace Templatized {

/************************************************
Methods of class VolumeRenderingSamplerCartesian:
************************************************/

template <class ScalarParam,class ValueParam>
inline
VolumeRenderingSampler<Cartesian<ScalarParam,3,ValueParam> >::VolumeRenderingSampler(
	const typename VolumeRenderingSampler<Cartesian<ScalarParam,3,ValueParam> >::DataSet& sDataSet)
	:dataSet(sDataSet)
	{
	/* Copy the original Cartesian volume size: */
	for(int i=0;i<3;++i)
		samplerSize[i]=(unsigned int)(dataSet.getNumVertices()[i]);
	}

template <class ScalarParam,class ValueParam>
template <class ScalarExtractorParam,class VoxelParam>
inline
void
VolumeRenderingSampler<Cartesian<ScalarParam,3,ValueParam> >::sample(
	const ScalarExtractorParam& scalarExtractor,
	VoxelParam* voxels,
	const ptrdiff_t voxelStrides[3],
	Comm::MulticastPipe* pipe,
	float percentageScale,
	float percentageOffset,
	Visualization::Abstract::Algorithm* algorithm) const
	{
	typedef typename ScalarExtractorParam::Scalar VScalar;
	typedef VoxelParam Voxel;
	
	/* Determine the data set's value range: */
	VScalar minValue,maxValue;
	typename DataSet::VertexIterator vIt=dataSet.beginVertices();
	minValue=maxValue=vIt->getValue(scalarExtractor);
	for(++vIt;vIt!=dataSet.endVertices();++vIt)
		{
		VScalar value=vIt->getValue(scalarExtractor);
		if(minValue>value)
			minValue=value;
		if(maxValue<value)
			maxValue=value;
		}
	
	typename DataSet::Index index;
	Voxel* vPtr0=voxels;
	for(index[0]=0;index[0]<dataSet.getNumVertices()[0];++index[0],vPtr0+=voxelStrides[0])
		{
		Voxel* vPtr1=vPtr0;
		for(index[1]=0;index[1]<dataSet.getNumVertices()[1];++index[1],vPtr1+=voxelStrides[1])
			{
			Voxel* vPtr2=vPtr1;
			for(index[2]=0;index[2]<dataSet.getNumVertices()[2];++index[2],vPtr2+=voxelStrides[2])
				{
				/* Get the vertex' scalar value: */
				VScalar value=scalarExtractor.getValue(dataSet.getVertexValue(index));
				
				/* Convert the value to unsigned char: */
				*vPtr2=Voxel((value-minValue)*VScalar(255)/(maxValue-minValue)+VScalar(0.5));
				}
			}
		
		/* Update the busy dialog: */
		algorithm->callBusyFunction(float(index[0]+1)*percentageScale/float(dataSet.getNumVertices()[0])+percentageOffset);
		}
	}

}

}
