/***********************************************************************
ImageStack - Class to represent scalar-valued Cartesian data sets stored
as stacks of color or greyscale images.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Misc/Timer.h>
#include <Plugins/FactoryManager.h>
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

Visualization::Abstract::DataSet* ImageStack::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Parse arguments: */
	bool medianFilter=false;
	bool lowpassFilter=false;
	for(unsigned int i=1;i<args.size();++i)
		{
		if(strcasecmp(args[i].c_str(),"MEDIANFILTER")==0)
			medianFilter=true;
		else if(strcasecmp(args[i].c_str(),"LOWPASSFILTER")==0)
			lowpassFilter=true;
		}
	
	/* Open the metadata file: */
	Misc::File file(args[0].c_str(),"rt");
	
	/* Parse the image stack layout: */
	DS::Index numVertices;
	DS::Size cellSize;
	char* sliceDirectory=0;
	char* sliceFileNameTemplate=0;
	int sliceIndexStart=0;
	int sliceIndexFactor=1;
	int regionOrigin[2]={0,0};
	while(!file.eof())
		{
		/* Read the next line from the file: */
		char line[256];
		file.gets(line,sizeof(line));
		
		/* Search for the first equal sign in the line: */
		char* eqPtr=0;
		for(eqPtr=line;*eqPtr!='\0'&&*eqPtr!='=';++eqPtr)
			;
		
		if(*eqPtr=='=')
			{
			/* Extract the tag name to the left of the equal sign: */
			char* tagStart;
			for(tagStart=line;isspace(*tagStart);++tagStart)
				;
			char* tagEnd;
			for(tagEnd=eqPtr;isspace(tagEnd[-1])&&tagEnd>tagStart;--tagEnd)
				;
			
			/* Extract the value to the right of the equal sign: */
			char* valueStart;
			for(valueStart=eqPtr+1;*valueStart!='\0'&&isspace(*valueStart);++valueStart)
				;
			char* valueEnd;
			for(valueEnd=valueStart;*valueEnd!='\0';++valueEnd)
				;
			for(;isspace(valueEnd[-1])&&valueEnd>valueStart;--valueEnd)
				;
			
			if(tagEnd>tagStart&&valueEnd>valueStart)
				{
				*tagEnd='\0';
				*valueEnd='\0';
				if(strcasecmp(tagStart,"numSlices")==0)
					sscanf(valueStart,"%d",&numVertices[0]);
				else if(strcasecmp(tagStart,"imageSize")==0)
					sscanf(valueStart,"%d %d",&numVertices[2],&numVertices[1]);
				else if(strcasecmp(tagStart,"regionOrigin")==0)
					sscanf(valueStart,"%d %d",&regionOrigin[0],&regionOrigin[1]);
				else if(strcasecmp(tagStart,"sampleSpacing")==0)
					sscanf(valueStart,"%f %f %f",&cellSize[0],&cellSize[2],&cellSize[1]);
				else if(strcasecmp(tagStart,"sliceDirectory")==0)
					{
					delete[] sliceDirectory;
					sliceDirectory=new char[valueEnd-valueStart+1];
					strcpy(sliceDirectory,valueStart);
					}
				else if(strcasecmp(tagStart,"sliceFileNameTemplate")==0)
					{
					delete[] sliceFileNameTemplate;
					sliceFileNameTemplate=new char[valueEnd-valueStart+1];
					strcpy(sliceFileNameTemplate,valueStart);
					}
				else if(strcasecmp(tagStart,"sliceIndexStart")==0)
					sscanf(valueStart,"%d",&sliceIndexStart);
				else if(strcasecmp(tagStart,"sliceIndexFactor")==0)
					sscanf(valueStart,"%d",&sliceIndexFactor);
				}
			}
		}
	
	/* Create the data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Load all image slices: */
	std::cout<<"Reading image slices...   0%"<<std::flush;
	DataSet::DS::Array& vertices=result->getDs().getVertices();
	unsigned char* vertexPtr=vertices.getArray();
	for(int i=0;i<numVertices[0];++i)
		{
		/* Generate the slice file name: */
		char sliceFileName[1024];
		snprintf(sliceFileName,sizeof(sliceFileName),sliceFileNameTemplate,i*sliceIndexFactor+sliceIndexStart);
		
		/* Load the slice as an RGB image: */
		Images::RGBImage slice=Images::readImageFile(sliceFileName);
		
		/* Check if the slice conforms: */
		if(slice.getSize(0)<(unsigned int)(regionOrigin[0]+numVertices[2])||slice.getSize(1)<(unsigned int)(regionOrigin[1]+numVertices[1]))
			Misc::throwStdErr("ImageStack::load: Size of slice file \"%s\" does not match image stack size",sliceFileName);
		
		/* Convert the slice's pixels to greyscale and copy them into the data set: */
		for(int y=regionOrigin[1];y<regionOrigin[1]+numVertices[1];++y)
			for(int x=regionOrigin[0];x<regionOrigin[0]+numVertices[2];++x,++vertexPtr)
				{
				const Images::RGBImage::Color& pixel=slice.getPixel(x,y);
				float value=float(pixel[0])*0.299f+float(pixel[1])*0.587+float(pixel[2])*0.114f;
				*vertexPtr=(unsigned char)(Math::floor(value+0.5f));
				}
		
		std::cout<<"\b\b\b\b"<<std::setw(3)<<((i+1)*100)/numVertices[0]<<"%"<<std::flush;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	
	if(medianFilter||lowpassFilter)
		{
		/* Run a median + lowpass filter on all slice triples to reduce random speckle: */
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
			
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((y+1)*100)/numVertices[1]<<"%"<<std::flush;
			}
		std::cout<<"\b\b\b\bdone"<<std::endl;
		
		delete[] filtered;
		}
	
	#if 0
	/* Filter the 3D image: */
	Misc::Timer t;
	// minimumFilter(vertices,3);
	// maximumFilter(vertices,3);
	// sphereFilter(vertices,5,200);
	t.elapse();
	std::cout<<"Filter time: "<<t.getTime()*1000.0<<" ms"<<std::endl;
	
	{
	/* Save filtered result to a .vol file: */
	Misc::File volFile("Filtered.vol","wb",Misc::File::BigEndian);
	volFile.write<int>(vertices.getSize().getComponents(),3);
	volFile.write<int>(0);
	float domainSize[3];
	for(int i=0;i<3;++i)
		domainSize[i]=float(cellSize[i])*float(vertices.getSize(i)-1);
	volFile.write<float>(domainSize,3);
	volFile.write<unsigned char>(vertices.getArray(),vertices.getNumElements());
	}
	#endif
	
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
