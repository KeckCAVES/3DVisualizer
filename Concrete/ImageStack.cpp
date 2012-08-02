/***********************************************************************
ImageStack - Class to represent scalar-valued Cartesian data sets stored
as stacks of color or greyscale images.
Copyright (c) 2005-2011 Oliver Kreylos

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

#include <Concrete/ImageStack.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/ThrowStdErr.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>

namespace Visualization {

namespace Concrete {

#if 0

namespace {

void minimumFilter(Misc::Array<unsigned char,3>& data,int filterSize)
	{
	typedef Misc::Array<unsigned char,3> Array;
	typedef Array::Index Index;
	
	std::cout<<"Minimum filter...   0%"<<std::flush;
	int counter=0;
	int maxCounter=data.getSize(0)+data.getSize(1)+data.getSize(2);
	for(int direction=0;direction<3;++direction)
		{
		unsigned char* buffer=new unsigned char[data.getSize(direction)];
		
		for(int i1=0;i1<data.getSize((direction+1)%3);++i1,++counter)
			{
			for(int i2=0;i2<data.getSize((direction+2)%3);++i2)
				{
				Index baseIndex(0);
				baseIndex[(direction+1)%3]=i1;
				baseIndex[(direction+2)%3]=i2;
				unsigned char* base=data.getAddress(baseIndex);
				ptrdiff_t increment=data.getIncrement(direction);
				
				for(int i=0;i<filterSize;++i)
					{
					unsigned char min=255;
					for(int di=-i;di<=filterSize;++di)
						if(min>base[(i+di)*increment])
							min=base[(i+di)*increment];
					buffer[i]=min;
					}
				for(int i=filterSize;i<data.getSize(direction)-filterSize;++i)
					{
					unsigned char min=255;
					for(int di=-filterSize;di<=filterSize;++di)
						if(min>base[(i+di)*increment])
							min=base[(i+di)*increment];
					buffer[i]=min;
					}
				for(int i=data.getSize(direction)-filterSize;i<data.getSize(direction);++i)
					{
					unsigned char min=255;
					for(int di=-filterSize;di<data.getSize(direction)-i;++di)
						if(min>base[(i+di)*increment])
							min=base[(i+di)*increment];
					buffer[i]=min;
					}
				
				for(int i=0;i<data.getSize(direction);++i)
					base[i*increment]=buffer[i];
				}
			
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((counter+1)*100)/maxCounter<<"%"<<std::flush;
			}
		
		delete[] buffer;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	}

void maximumFilter(Misc::Array<unsigned char,3>& data,int filterSize)
	{
	typedef Misc::Array<unsigned char,3> Array;
	typedef Array::Index Index;
	
	std::cout<<"Maximum filter...   0%"<<std::flush;
	int counter=0;
	int maxCounter=data.getSize(0)+data.getSize(1)+data.getSize(2);
	for(int direction=0;direction<3;++direction)
		{
		unsigned char* buffer=new unsigned char[data.getSize(direction)];
		
		for(int i1=0;i1<data.getSize((direction+1)%3);++i1,++counter)
			{
			for(int i2=0;i2<data.getSize((direction+2)%3);++i2)
				{
				Index baseIndex(0);
				baseIndex[(direction+1)%3]=i1;
				baseIndex[(direction+2)%3]=i2;
				unsigned char* base=data.getAddress(baseIndex);
				ptrdiff_t increment=data.getIncrement(direction);
				
				for(int i=0;i<filterSize;++i)
					{
					unsigned char max=0;
					for(int di=-i;di<=filterSize;++di)
						if(max<base[(i+di)*increment])
							max=base[(i+di)*increment];
					buffer[i]=max;
					}
				for(int i=filterSize;i<data.getSize(direction)-filterSize;++i)
					{
					unsigned char max=0;
					for(int di=-filterSize;di<=filterSize;++di)
						if(max<base[(i+di)*increment])
							max=base[(i+di)*increment];
					buffer[i]=max;
					}
				for(int i=data.getSize(direction)-filterSize;i<data.getSize(direction);++i)
					{
					unsigned char max=0;
					for(int di=-filterSize;di<data.getSize(direction)-i;++di)
						if(max<base[(i+di)*increment])
							max=base[(i+di)*increment];
					buffer[i]=max;
					}
				
				for(int i=0;i<data.getSize(direction);++i)
					base[i*increment]=buffer[i];
				}
			
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((counter+1)*100)/maxCounter<<"%"<<std::flush;
			}
		
		delete[] buffer;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	}

void sphereFilter(Misc::Array<unsigned char,3>& data,int sphereRadius,unsigned char sphereValue)
	{
	typedef Misc::Array<unsigned char,3> Array;
	typedef Array::Index Index;
	
	/* Create an array of index offsets: */
	std::vector<Index> offsets;
	{
	Index d;
	for(d[0]=-sphereRadius;d[0]<=sphereRadius;++d[0])
		for(d[1]=-sphereRadius;d[1]<=sphereRadius;++d[1])
			for(d[2]=-sphereRadius;d[2]<=sphereRadius;++d[2])
				if(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]<=sphereRadius*sphereRadius+sphereRadius)
					offsets.push_back(d);
	}
	
	unsigned char* temp=new unsigned char[data.getNumElements()];
	
	std::cout<<"Sphere filter...   0%"<<std::flush;
	Index index;
	for(index[0]=0;index[0]<data.getSize(0);++index[0])
		{
		for(index[1]=0;index[1]<data.getSize(1);++index[1])
			for(index[2]=0;index[2]<data.getSize(2);++index[2])
				{
				double rms=0.0;
				int numSamples=0;
				for(std::vector<Index>::const_iterator oIt=offsets.begin();oIt!=offsets.end();++oIt)
					{
					Index sample=index+*oIt;
					if(data.isValid(sample))
						{
						rms+=Math::sqr(double(data(sample))-double(sphereValue));
						++numSamples;
						}
					}

				rms=Math::sqrt(rms/double(numSamples));
				temp[data.calcLinearIndex(index)]=(unsigned char)Math::floor(rms+0.5);
				}
		
		std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[0]+1)*100)/data.getSize(0)<<"%"<<std::flush;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	
	unsigned char* dPtr=data.getArray();
	unsigned char* sPtr=temp;
	for(int i=data.getNumElements();i>0;--i,++sPtr,++dPtr)
		*dPtr=*sPtr;
	delete[] temp;
	}

}

#endif

/***************************
Methods of class ImageStack:
***************************/

ImageStack::ImageStack(void)
	:BaseModule("ImageStack")
	{
	}

Visualization::Abstract::DataSet* ImageStack::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Parse arguments: */
	bool medianFilter=false;
	bool lowpassFilter=false;
	for(unsigned int i=1;i<args.size();++i)
		{
		if(args[i]=="MedianFilter")
			medianFilter=true;
		else if(args[i]=="LowpassFilter")
			lowpassFilter=true;
		}
	
	/* Open the meta file: */
	IO::ValueSource metaSource(openFile(args[0],pipe));
	metaSource.setPunctuation("#=");
	metaSource.setQuote('"',true);
	metaSource.setEscape('\\');
	metaSource.skipWs();
	
	/* Parse the image stack layout: */
	DS::Index numVertices(0,0,0);
	DS::Size cellSize(0,0,0);
	std::string sliceDirectory;
	std::string sliceFileNameTemplate;
	int sliceIndexStart=0;
	int sliceIndexFactor=1;
	int regionOrigin[2]={0,0};
	while(!metaSource.eof())
		{
		/* Read the tag: */
		std::string tag=metaSource.readString();
		
		/* Check for comments: */
		if(tag=="#")
			{
			/* Skip the rest of the line: */
			metaSource.skipLine();
			metaSource.skipWs();
			
			/* Read the next tag: */
			continue;
			}
		
		/* Check for the equal sign: */
		if(!metaSource.isLiteral('='))
			Misc::throwStdErr("ImageStack::load: Missing \"=\" in metafile %s",args[0].c_str());
		
		/* Process the tag: */
		if(tag=="numSlices")
			numVertices[0]=metaSource.readInteger();
		else if(tag=="imageSize")
			{
			numVertices[2]=metaSource.readInteger();
			numVertices[1]=metaSource.readInteger();
			}
		else if(tag=="regionOrigin")
			{
			regionOrigin[0]=metaSource.readInteger();
			regionOrigin[1]=metaSource.readInteger();
			}
		else if(tag=="sampleSpacing")
			{
			cellSize[0]=DS::Scalar(metaSource.readNumber());
			cellSize[2]=DS::Scalar(metaSource.readNumber());
			cellSize[1]=DS::Scalar(metaSource.readNumber());
			}
		else if(tag=="sliceDirectory")
			{
			sliceDirectory=metaSource.readString();
			if(!sliceDirectory.empty()&&sliceDirectory[sliceDirectory.size()-1]!='/')
				sliceDirectory.push_back('/');
			}
		else if(tag=="sliceFileNameTemplate")
			sliceFileNameTemplate=metaSource.readString();
		else if(tag=="sliceIndexStart")
			sliceIndexStart=metaSource.readInteger();
		else if(tag=="sliceIndexFactor")
			sliceIndexFactor=metaSource.readInteger();
		else
			Misc::throwStdErr("ImageStack::load: Unknown tag %s in metafile %s",tag.c_str(),args[0].c_str());
		}
	
	/* Create the data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Load all image slices: */
	if(master)
		std::cout<<"Reading image slices...   0%"<<std::flush;
	DataSet::DS::Array& vertices=result->getDs().getVertices();
	unsigned char* vertexPtr=vertices.getArray();
	for(int i=0;i<numVertices[0];++i)
		{
		/* Generate the slice file name: */
		std::string fullSliceFileName=sliceDirectory;
		char sliceFileName[1024];
		snprintf(sliceFileName,sizeof(sliceFileName),sliceFileNameTemplate.c_str(),i*sliceIndexFactor+sliceIndexStart);
		fullSliceFileName.append(sliceFileName);
		fullSliceFileName=getFullPath(fullSliceFileName);
		
		/* Load the slice as an RGB image: */
		Images::RGBImage slice=Images::readImageFile(fullSliceFileName.c_str(),openFile(fullSliceFileName,pipe));
		
		/* Check if the slice conforms: */
		if(slice.getSize(0)<(unsigned int)(regionOrigin[0]+numVertices[2])||slice.getSize(1)<(unsigned int)(regionOrigin[1]+numVertices[1]))
			Misc::throwStdErr("ImageStack::load: Size of slice file \"%s\" does not match image stack size",fullSliceFileName.c_str());
		
		/* Convert the slice's pixels to greyscale and copy them into the data set: */
		for(int y=regionOrigin[1];y<regionOrigin[1]+numVertices[1];++y)
			for(int x=regionOrigin[0];x<regionOrigin[0]+numVertices[2];++x,++vertexPtr)
				{
				const Images::RGBImage::Color& pixel=slice.getPixel(x,y);
				float value=float(pixel[0])*0.299f+float(pixel[1])*0.587+float(pixel[2])*0.114f;
				*vertexPtr=(unsigned char)(Math::floor(value+0.5f));
				}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((i+1)*100)/numVertices[0]<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	if(medianFilter||lowpassFilter)
		{
		/* Run a median + lowpass filter on all slice triples to reduce random speckle: */
		if(master)
			std::cout<<"Filtering image stack...   0%"<<std::flush;
		unsigned char* filtered=new unsigned char[numVertices[0]];
		ptrdiff_t vPtrInc=vertices.getIncrement(0);
		for(int y=0;y<numVertices[1];++y)
			{
			for(int x=0;x<numVertices[2];++x)
				{
				if(medianFilter)
					{
					/* Run median filter: */
					unsigned char* vPtr=vertices.getAddress(0,y,x);
					filtered[0]=*vPtr;
					vPtr+=vPtrInc;
					for(int z=1;z<numVertices[0]-1;++z,vPtr+=vPtrInc)
						{
						if(vPtr[-vPtrInc]<vPtr[0])
							{
							if(vPtr[0]<vPtr[vPtrInc])
								filtered[z]=vPtr[0];
							else
								filtered[z]=vPtr[-vPtrInc]<vPtr[vPtrInc]?vPtr[vPtrInc]:vPtr[-vPtrInc];
							}
						else
							{
							if(vPtr[-vPtrInc]<vPtr[vPtrInc])
								filtered[z]=vPtr[-vPtrInc];
							else
								filtered[z]=vPtr[0]<vPtr[vPtrInc]?vPtr[vPtrInc]:vPtr[0];
							}
						}
					filtered[numVertices[0]-1]=*vPtr;
					}
				else if(lowpassFilter)
					{
					/* Copy the source image data to allow lowpass filtering: */
					unsigned char* vPtr=vertices.getAddress(0,y,x);
					for(int z=0;z<numVertices[0];++z,vPtr+=vPtrInc)
						filtered[z]=*vPtr;
					}
				
				if(lowpassFilter)
					{
					/* Run lowpass filter: */
					unsigned char* vPtr=vertices.getAddress(0,y,x);
					*vPtr=(unsigned char)((int(filtered[0])*3+int(filtered[1])*2+int(filtered[2])+3)/6);
					vPtr+=vPtrInc;
					*vPtr=(unsigned char)((int(filtered[0])*2+int(filtered[1])*3+int(filtered[2])*2+int(filtered[3])+4)/8);
					vPtr+=vPtrInc;
					for(int z=2;z<numVertices[0]-2;++z,vPtr+=vPtrInc)
						*vPtr=(unsigned char)((int(filtered[z-2])+int(filtered[z-1])*2+int(filtered[z])*3+int(filtered[z+1])*2+int(filtered[z+2])+4)/9);
					*vPtr=(unsigned char)((int(filtered[numVertices[0]-4])+int(filtered[numVertices[0]-3])*2+int(filtered[numVertices[0]-2])*3+int(filtered[numVertices[0]-1])*2+4)/8);
					vPtr+=vPtrInc;
					*vPtr=(unsigned char)((int(filtered[numVertices[0]-3])+int(filtered[numVertices[0]-2])*2+int(filtered[numVertices[0]-1])*3+3)/6);
					}
				else if(medianFilter)
					{
					/* Copy the filtered data back to the volume: */
					unsigned char* vPtr=vertices.getAddress(0,y,x);
					for(int z=0;z<numVertices[0];++z,vPtr+=vPtrInc)
						*vPtr=filtered[z];
					}
				}
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<((y+1)*100)/numVertices[1]<<"%"<<std::flush;
			}
		if(master)
			std::cout<<"\b\b\b\bdone"<<std::endl;
		
		delete[] filtered;
		}
	
	return result;
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::ImageStack* module=new Visualization::Concrete::ImageStack();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
