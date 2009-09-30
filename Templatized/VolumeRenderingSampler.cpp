/***********************************************************************
VolumeRenderingSampler - Helper class to create shader- or texture-based
volume renderers for arbitrary data set types.
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

#define VISUALIZATION_TEMPLATIZED_VOLUMERENDERINGSAMPLER_IMPLEMENTATION

#include <Templatized/VolumeRenderingSampler.h>

#include <Misc/Utility.h>
#include <Comm/MulticastPipe.h>

#include <Abstract/Algorithm.h>

namespace Visualization {

namespace Templatized {

/***************************************
Methods of class VolumeRenderingSampler:
***************************************/

template <class DataSetParam>
inline
VolumeRenderingSampler<DataSetParam>::VolumeRenderingSampler(
	const typename VolumeRenderingSampler<DataSetParam>::DataSet& sDataSet)
	:dataSet(sDataSet)
	{
	/* Calculate the optimal Cartesian volume size: */
	samplerOrigin=dataSet.getDomainBox().getOrigin();
	Size boxSize=dataSet.getDomainBox().getSize();
	Scalar avgCellSize=dataSet.calcAverageCellSize();
	for(int i=0;i<3;++i)
		{
		/* Find a power-of-two grid size that approximates the data set's average cell size: */
		Scalar optSize=Scalar(2)*boxSize[i]/avgCellSize;
		for(samplerSize[i]=2;samplerSize[i]<512&&Scalar(samplerSize[i])*Math::sqrt(Scalar(2))<optSize;samplerSize[i]<<=1)
			;
		samplerCellSize[i]=boxSize[i]/Scalar(samplerSize[i]-1);
		}
	}

template <class DataSetParam>
template <class ScalarExtractorParam,class VoxelParam>
inline
void
VolumeRenderingSampler<DataSetParam>::sample(
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
	
	/* Sort the voxel block's dimensions according to their stride values: */
	int dims[3];
	for(int i=0;i<3;++i)
		dims[i]=i;
	if(voxelStrides[dims[0]]<voxelStrides[dims[1]])
		Misc::swap(dims[0],dims[1]);
	if(voxelStrides[dims[1]]<voxelStrides[dims[2]])
		Misc::swap(dims[1],dims[2]);
	if(voxelStrides[dims[0]]<voxelStrides[dims[1]])
		Misc::swap(dims[0],dims[1]);
	
	/* Sample volume data on the master, and receive results on the slaves: */
	Voxel* spanBuffer=0;
	if(pipe!=0)
		spanBuffer=new Voxel[samplerSize[dims[2]]];
	if(pipe==0||pipe->isMaster())
		{
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
		
		/* Sample the data set's scalar values into the voxel block: */
		typename DataSet::Locator sampleLocator=dataSet.getLocator();
		bool sampleValid=false;
		unsigned int index[3];
		Point samplePos;
		Voxel* base0;
		for(index[dims[0]]=0,samplePos[dims[0]]=samplerOrigin[dims[0]],base0=voxels;index[dims[0]]<samplerSize[dims[0]];++index[dims[0]],samplePos[dims[0]]+=samplerCellSize[dims[0]],base0+=voxelStrides[dims[0]])
			{
			Voxel* base1;
			for(index[dims[1]]=0,samplePos[dims[1]]=samplerOrigin[dims[1]],base1=base0;index[dims[1]]<samplerSize[dims[1]];++index[dims[1]],samplePos[dims[1]]+=samplerCellSize[dims[1]],base1+=voxelStrides[dims[1]])
				{
				Voxel* base2;
				for(index[dims[2]]=0,samplePos[dims[2]]=samplerOrigin[dims[2]],base2=base1;index[dims[2]]<samplerSize[dims[2]];++index[dims[2]],samplePos[dims[2]]+=samplerCellSize[dims[2]],base2+=voxelStrides[dims[2]])
					{
					/* Locate the grid point: */
					sampleValid=sampleLocator.locatePoint(samplePos,sampleValid);
					if(sampleValid)
						{
						/* Get the vertex' scalar value: */
						VScalar value=sampleLocator.calcValue(scalarExtractor);
						*base2=Voxel((value-minValue)*VScalar(255)/(maxValue-minValue)+VScalar(0.5));
						}
					else
						{
						/* Assign a default value: */
						*base2=Voxel(0);
						}
					}
				
				if(pipe!=0)
					{
					/* Write the most recent span of voxels to the pipe: */
					base2=base1;
					for(unsigned int i=0;i<samplerSize[dims[2]];++i,base2+=voxelStrides[dims[2]])
						spanBuffer[i]=*base2;
					pipe->write<Voxel>(spanBuffer,samplerSize[dims[2]]);
					}
				}
			
			/* Update the busy dialog: */
			algorithm->callBusyFunction(float(index[dims[0]]+1)*percentageScale/float(samplerSize[dims[0]])+percentageOffset);
			}
		}
	else
		{
		/* Receive the resampled data set from the multicast pipe: */
		unsigned int index[3];
		Voxel* base0;
		for(index[dims[0]]=0,base0=voxels;index[dims[0]]<samplerSize[dims[0]];++index[dims[0]],base0+=voxelStrides[dims[0]])
			{
			Voxel* base1;
			for(index[dims[1]]=0,base1=base0;index[dims[1]]<samplerSize[dims[1]];++index[dims[1]],base1+=voxelStrides[dims[1]])
				{
				/* Read a span of voxels: */
				pipe->read<VoxelParam>(spanBuffer,samplerSize[dims[2]]);
				Voxel* base2=base1;
				for(unsigned int i=0;i<samplerSize[dims[2]];++i,base2+=voxelStrides[dims[2]])
					*base2=spanBuffer[i];
				}
			
			/* Update the busy dialog: */
			algorithm->callBusyFunction(float(index[dims[0]]+1)*percentageScale/float(samplerSize[dims[0]])+percentageOffset);
			}
		}
	if(pipe!=0)
		delete[] spanBuffer;
	}

}

}
