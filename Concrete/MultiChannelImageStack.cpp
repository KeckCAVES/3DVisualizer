/***********************************************************************
MultiChannelImageStack - Class to represent multivariate scalaar-valued
Cartesian data sets stored as multiple matching stacks of color or
greyscale images.
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

#include <Concrete/MultiChannelImageStack.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#ifdef IMAGES_CONFIG_HAVE_TIFF
#include <tiffio.h>
#endif
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/FileNameExtensions.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>

namespace Visualization {

namespace Concrete {

namespace {

/**************
Helper classes:
**************/

struct StackDescriptor
	{
	/* Elements: */
	public:
	DS& dataSet;
	DS::Index numVertices;
	DS::Size cellSize;
	int dsPartsMask;
	bool haveDs;
	std::string imageDirectory;
	int imageIndexStart;
	int imageIndexStep;
	int regionOrigin[2];
	bool master;
	
	/* Constructors and destructors: */
	StackDescriptor(DS& sDataSet,bool sMaster)
		:dataSet(sDataSet),
		 numVertices(0,0,0),cellSize(0,0,0),dsPartsMask(0x0),haveDs(false),
		 imageIndexStart(0),imageIndexStep(1),
		 master(sMaster)
		{
		regionOrigin[0]=regionOrigin[1]=0;
		}
	
	/* Methods: */
	void update(int dsPart)
		{
		dsPartsMask|=dsPart;
		if(dsPartsMask==0x7&&!haveDs)
			{
			dataSet.setData(numVertices,cellSize,0);
			haveDs=true;
			}
		}
	};

/****************
Helper functions:
****************/

#ifdef IMAGES_HAVE_TIFF

template <class PixelParam>
inline
void
convertGreyscaleTiffImage(
	StackDescriptor& sd,
	Value* slicePtr,
	TIFF* image,
	int offset)
	{
	/* Read and convert the image row by row: */
	uint32 width;
	TIFFGetField(image,TIFFTAG_IMAGEWIDTH,&width);
	PixelParam* rowBuffer=new PixelParam[width];
	
	/* Copy the image's pixels into the data set: */
	Value* rowPtr=slicePtr;
	for(int y=sd.regionOrigin[1];y<sd.regionOrigin[1]+sd.numVertices[1];++y,rowPtr+=sd.dataSet.getVertexStride(1))
		{
		/* Read the pixel row: */
		TIFFReadScanline(image,rowBuffer,y);
		
		Value* vPtr=rowPtr;
		for(int x=sd.regionOrigin[0];x<sd.regionOrigin[0]+sd.numVertices[0];++x,vPtr+=sd.dataSet.getVertexStride(0))
			*vPtr=Value(int(rowBuffer[x])+offset);
		}
	
	delete[] rowBuffer;
	}

void loadGreyscaleTiffImage(StackDescriptor& sd,Value* slicePtr,const char* imageFileName)
	{
	/* Open the TIFF image: */
	TIFF* image=TIFFOpen(imageFileName,"r");
	if(image==0)
		Misc::throwStdErr("MultiChannelImageStack::load: Unable to open image file %s",imageFileName);
	
	/* Get the image size and type: */
	uint32 size[2];
	uint16 numBits,numSamples,sampleFormat;
	TIFFGetField(image,TIFFTAG_IMAGEWIDTH,&size[0]);
	TIFFGetField(image,TIFFTAG_IMAGELENGTH,&size[1]);
	TIFFGetField(image,TIFFTAG_BITSPERSAMPLE,&numBits);
	TIFFGetField(image,TIFFTAG_SAMPLESPERPIXEL,&numSamples);
	TIFFGetField(image,TIFFTAG_SAMPLEFORMAT,&sampleFormat);
	
	/* Check if the image conforms: */
	if(numSamples!=1)
		{
		TIFFClose(image);
		Misc::throwStdErr("MultiChannelImageStack::load: Image file %s is not a greyscale image",imageFileName);
		}
	if(size[0]<(unsigned int)(sd.regionOrigin[0]+sd.numVertices[0])||size[1]<(unsigned int)(sd.regionOrigin[1]+sd.numVertices[1]))
		{
		TIFFClose(image);
		Misc::throwStdErr("MultiChannelImageStack::load: Size of image file \"%s\" does not match image stack size",imageFileName);
		}
	
	/* Load the image based on its pixel format: */
	if(numBits==16)
		{
		if(sampleFormat==SAMPLEFORMAT_INT)
			convertGreyscaleTiffImage<signed short int>(sd,slicePtr,image,32768);
		else
			convertGreyscaleTiffImage<unsigned short int>(sd,slicePtr,image,0);
		}
	else if(numBits==8)
		{
		if(sampleFormat==SAMPLEFORMAT_INT)
			convertGreyscaleTiffImage<signed char>(sd,slicePtr,image,128);
		else
			convertGreyscaleTiffImage<unsigned char>(sd,slicePtr,image,0);
		}
	else
		{
		TIFFClose(image);
		Misc::throwStdErr("MultiChannelImageStack::load: Image file \"%s\" has an unsupported pixel format",imageFileName);
		}
	
	/* Close the TIFF image: */
	TIFFClose(image);
	}

#endif

void loadGreyscaleImage(StackDescriptor& sd,Value* slicePtr,const char* imageFileName)
	{
	/* Load the image as an RGB image: */
	Images::RGBImage image=Images::readImageFile(imageFileName);
	
	/* Check if the image conforms: */
	if(image.getSize(0)<(unsigned int)(sd.regionOrigin[0]+sd.numVertices[0])||image.getSize(1)<(unsigned int)(sd.regionOrigin[1]+sd.numVertices[1]))
		Misc::throwStdErr("MultiChannelImageStack::load: Size of image file \"%s\" does not match image stack size",imageFileName);
	
	/* Convert the image's pixels to greyscale and copy them into the data set: */
	Value* rowPtr=slicePtr;
	for(int y=sd.regionOrigin[1];y<sd.regionOrigin[1]+sd.numVertices[1];++y,rowPtr+=sd.dataSet.getVertexStride(1))
		{
		Value* vPtr=rowPtr;
		for(int x=sd.regionOrigin[0];x<sd.regionOrigin[0]+sd.numVertices[0];++x,vPtr+=sd.dataSet.getVertexStride(0))
			{
			const Images::RGBImage::Color& pixel=image.getPixel(x,y);
			float value=float(pixel[0])*0.299f+float(pixel[1])*0.587+float(pixel[2])*0.114f;
			*vPtr=Value(value+0.5f);
			}
		}
	}

void loadGreyscaleImageStack(StackDescriptor& sd,int newSliceIndex,const char* imageFileNameTemplate)
	{
	#ifdef IMAGES_HAVE_TIFF
	/* Check if the image file name template matches TIFF images: */
	const char* ext=Misc::getExtension(imageFileNameTemplate);
	bool isTiff=strcasecmp(ext,".tif")==0||strcasecmp(ext,".tiff")==0;
	#endif
	
	/* Get a pointer to the slice: */
	Value* slicePtr=sd.dataSet.getSliceArray(newSliceIndex);
	if(sd.master)
		std::cout<<"Reading greyscale image stack "<<imageFileNameTemplate<<"...   0%"<<std::flush;
	Misc::Timer loadTimer;
	for(int imageIndex=0;imageIndex<sd.numVertices[2];++imageIndex,slicePtr+=sd.dataSet.getVertexStride(2))
		{
		/* Generate the image file name: */
		char imageFileNameBuffer[1024];
		snprintf(imageFileNameBuffer,sizeof(imageFileNameBuffer),imageFileNameTemplate,imageIndex*sd.imageIndexStep+sd.imageIndexStart);
		std::string imageFileName=sd.imageDirectory;
		imageFileName.append(imageFileNameBuffer);
		
		/* Load the image: */
		#ifdef IMAGES_HAVE_TIFF
		if(isTiff)
			loadGreyscaleTiffImage(sd,slicePtr,imageFileName.c_str());
		else
			loadGreyscaleImage(sd,slicePtr,imageFileName.c_str());
		#else
		loadGreyscaleImage(sd,slicePtr,imageFileName.c_str());
		#endif
		
		if(sd.master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((imageIndex+1)*100)/sd.numVertices[2]<<"%"<<std::flush;
		}
	loadTimer.elapse();
	if(sd.master)
		std::cout<<"\b\b\b\bdone in "<<loadTimer.getTime()*1000.0<<" ms"<<std::endl;
	}

void loadColorImageStack(StackDescriptor& sd,const int newSliceIndices[3],const char* imageFileNameTemplate)
	{
	/* Get a pointer to the slice: */
	Value* slices[3];
	for(int i=0;i<3;++i)
		slices[i]=sd.dataSet.getSliceArray(newSliceIndices[i]);
	ptrdiff_t sliceIndex=0;
	if(sd.master)
		std::cout<<"Reading color image stack "<<imageFileNameTemplate<<"...   0%"<<std::flush;
	Misc::Timer loadTimer;
	for(int imageIndex=0;imageIndex<sd.numVertices[2];++imageIndex,sliceIndex+=sd.dataSet.getVertexStride(2))
		{
		/* Generate the image file name: */
		char imageFileName[1024];
		snprintf(imageFileName,sizeof(imageFileName),imageFileNameTemplate,imageIndex*sd.imageIndexStep+sd.imageIndexStart);
		
		/* Load the image: */
		Images::RGBImage image=Images::readImageFile(sd.imageDirectory.empty()?imageFileName:(sd.imageDirectory+imageFileName).c_str());
		
		/* Check if the image conforms: */
		if(image.getSize(0)<(unsigned int)(sd.regionOrigin[0]+sd.numVertices[0])||image.getSize(1)<(unsigned int)(sd.regionOrigin[1]+sd.numVertices[1]))
			Misc::throwStdErr("MultiChannelImageStack::load: Size of image file \"%s\" does not match image stack size",imageFileName);
		
		/* Copy the image's pixels into the data set: */
		ptrdiff_t rowIndex=sliceIndex;
		for(int y=sd.regionOrigin[1];y<sd.regionOrigin[1]+sd.numVertices[1];++y,rowIndex+=sd.dataSet.getVertexStride(1))
			{
			ptrdiff_t vIndex=rowIndex;
			for(int x=sd.regionOrigin[0];x<sd.regionOrigin[0]+sd.numVertices[0];++x,vIndex+=sd.dataSet.getVertexStride(0))
				{
				const Images::RGBImage::Color& pixel=image.getPixel(x,y);
				for(int i=0;i<3;++i)
					slices[i][vIndex]=Value(pixel[i]);
				}
			}
		if(sd.master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((imageIndex+1)*100)/sd.numVertices[2]<<"%"<<std::flush;
		}
	loadTimer.elapse();
	if(sd.master)
		std::cout<<"\b\b\b\bdone in "<<loadTimer.getTime()*1000.0<<" ms"<<std::endl;
	}

void filterImageStack(StackDescriptor& sd,int sliceIndex,bool medianFilter,bool lowpassFilter)
	{
	/* Get a pointer to the slice: */
	Value* slicePtr=sd.dataSet.getSliceArray(sliceIndex);
	if(sd.master)
		std::cout<<"Filtering image stack...   0%"<<std::flush;
	Misc::Timer filterTimer;
	
	/* Create a buffer for a single voxel pile: */
	Value* pileBuffer=new Value[sd.numVertices[2]];
	
	/* Filter all pixels through all images: */
	Value* columnPtr=slicePtr;
	for(int x=0;x<sd.numVertices[0];++x,columnPtr+=sd.dataSet.getVertexStride(0))
		{
		Value* pilePtr=columnPtr;
		for(int y=0;y<sd.numVertices[1];++y,pilePtr+=sd.dataSet.getVertexStride(1))
			{
			Value* vPtr=pilePtr;
			int vInc=sd.dataSet.getVertexStride(2);
			Value* pPtr=pileBuffer;
			
			if(medianFilter)
				{
				/* Run a median filter over the pixel pile: */
				*pPtr=*vPtr;
				vPtr+=vInc;
				++pPtr;
				for(int z=2;z<sd.numVertices[2];++z,vPtr+=vInc,++pPtr)
					{
					if(vPtr[-vInc]<vPtr[0])
						{
						if(vPtr[0]<vPtr[vInc])
							*pPtr=vPtr[0];
						else
							*pPtr=vPtr[-vInc]<vPtr[vInc]?vPtr[vInc]:vPtr[-vInc];
						}
					else
						{
						if(vPtr[-vInc]<vPtr[vInc])
							*pPtr=vPtr[-vInc];
						else
							*pPtr=vPtr[0]<vPtr[vInc]?vPtr[vInc]:vPtr[0];
						}
					}
				*pPtr=*vPtr;
				}
			else
				{
				/* Copy the voxel pile into the pile buffer for the lowpass filter: */
				for(int z=0;z<sd.numVertices[2];++z,vPtr+=vInc,++pPtr)
					*pPtr=*vPtr;
				}
			
			vPtr=pilePtr;
			pPtr=pileBuffer;
			
			if(lowpassFilter)
				{
				/* Run a lowpass filter over the pixel pile: */
				*vPtr=Value((int(pPtr[0])*3+int(pPtr[1])*2+int(pPtr[2])+3)/6);
				vPtr+=vInc;
				++pPtr;
				*vPtr=Value((int(pPtr[-1])*2+int(pPtr[0])*3+int(pPtr[1])*2+int(pPtr[2])+4)/8);
				vPtr+=vInc;
				++pPtr;
				for(int z=4;z<sd.numVertices[2];++z,vPtr+=vInc,++pPtr)
					*vPtr=Value((int(pPtr[-2])+int(pPtr[-1])*2+int(pPtr[0])*3+int(pPtr[1])*2+int(pPtr[2])+4)/9);
				*vPtr=Value((int(pPtr[-2])+int(pPtr[-1])*2+int(pPtr[0])*3+int(pPtr[1])*2+4)/8);
				vPtr+=vInc;
				++pPtr;
				*vPtr=Value((int(pPtr[-2])+int(pPtr[-1])*2+int(pPtr[0])*3+3)/6);
				}
			else
				{
				/* Copy the pile buffer back into the voxel pile: */
				for(int z=0;z<sd.numVertices[2];++z,vPtr+=vInc,++pPtr)
					*vPtr=*pPtr;
				}
			}
		if(sd.master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((x+1)*100)/sd.numVertices[0]<<"%"<<std::flush;
		}
	
	delete[] pileBuffer;
	
	filterTimer.elapse();
	if(sd.master)
		std::cout<<"\b\b\b\bdone in "<<filterTimer.getTime()*1000.0<<" ms"<<std::endl;
	}

}

/***************************************
Methods of class MultiChannelImageStack:
***************************************/

MultiChannelImageStack::MultiChannelImageStack(void)
	:BaseModule("MultiChannelImageStack")
	{
	}

Visualization::Abstract::DataSet* MultiChannelImageStack::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Parse the module arguments: */
	StackDescriptor sd(dataSet,master);
	bool medianFilter=false;
	bool lowpassFilter=false;
	for(size_t i=0;i<args.size();++i)
		{
		if(strcasecmp(args[i].c_str(),"-imageSize")==0)
			{
			for(int j=0;j<2&&i+1<args.size();++j)
				{
				++i;
				sd.numVertices[j]=atoi(args[i].c_str());
				}
			sd.update(0x1);
			}
		else if(strcasecmp(args[i].c_str(),"-numImages")==0)
			{
			++i;
			if(i<args.size())
				sd.numVertices[2]=atoi(args[i].c_str());
			sd.update(0x2);
			}
		else if(strcasecmp(args[i].c_str(),"-sampleSpacing")==0)
			{
			for(int j=0;j<3&&i+1<args.size();++j)
				{
				++i;
				sd.cellSize[j]=DS::Scalar(atof(args[i].c_str()));
				}
			sd.update(0x4);
			}
		else if(strcasecmp(args[i].c_str(),"-regionOrigin")==0)
			{
			for(int j=0;j<2&&i+1<args.size();++j)
				{
				++i;
				sd.regionOrigin[j]=atoi(args[i].c_str());
				}
			}
		else if(strcasecmp(args[i].c_str(),"-imageDirectory")==0)
			{
			++i;
			if(i<args.size())
				{
				sd.imageDirectory=getFullPath(args[i]);
				if(!sd.imageDirectory.empty()&&sd.imageDirectory[sd.imageDirectory.size()-1]!='/')
					sd.imageDirectory.push_back('/');
				}
			}
		else if(strcasecmp(args[i].c_str(),"-imageIndexStart")==0)
			{
			++i;
			if(i<args.size())
				sd.imageIndexStart=atoi(args[i].c_str());
			}
		else if(strcasecmp(args[i].c_str(),"-imageIndexStep")==0)
			{
			++i;
			if(i<args.size())
				sd.imageIndexStep=atoi(args[i].c_str());
			}
		else if(strcasecmp(args[i].c_str(),"-median")==0)
			medianFilter=true;
		else if(strcasecmp(args[i].c_str(),"-lowpass")==0)
			lowpassFilter=true;
		else if(strcasecmp(args[i].c_str(),"-greyscale")==0)
			{
			if(i+2<args.size()&&sd.haveDs)
				{
				/* Add a new slice to the data set: */
				int newSliceIndex=dataSet.addSlice();
				
				/* Add a new scalar variable to the data value: */
				dataValue.addScalarVariable(args[i+1].c_str());
				
				/* Load the greyscale image stack: */
				loadGreyscaleImageStack(sd,newSliceIndex,args[i+2].c_str());
				
				/* Filter the image stack if requested: */
				if(medianFilter||lowpassFilter)
					filterImageStack(sd,newSliceIndex,medianFilter,lowpassFilter);
				medianFilter=false;
				lowpassFilter=false;
				}
			i+=2;
			}
		else if(strcasecmp(args[i].c_str(),"-color")==0)
			{
			if(i+4<args.size()&&sd.haveDs)
				{
				/* Add three new slices to the data set: */
				int newSliceIndices[3];
				for(int j=0;j<3;++j)
					newSliceIndices[j]=dataSet.addSlice();
				
				/* Add three new scalar variables to the data value: */
				for(int j=0;j<3;++j)
					dataValue.addScalarVariable(args[i+1+j].c_str());
				
				/* Load the color image stack: */
				loadColorImageStack(sd,newSliceIndices,args[i+4].c_str());
				
				/* Filter the image stack if requested: */
				if(medianFilter||lowpassFilter)
					for(int j=0;j<3;++j)
						filterImageStack(sd,newSliceIndices[j],medianFilter,lowpassFilter);
				medianFilter=false;
				lowpassFilter=false;
				}
			i+=4;
			}
		}
	
	return result.releaseTarget();
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::MultiChannelImageStack* module=new Visualization::Concrete::MultiChannelImageStack();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
