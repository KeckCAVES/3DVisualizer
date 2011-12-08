/***********************************************************************
AnalyzeFile - Class to encapsulate operations on scalar-valued data sets
stored in Analyze 7.5 format.
Copyright (c) 2006-2011 Oliver Kreylos

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

#include <string>
#include <IO/File.h>
#include <IO/SeekableFile.h>
#include <IO/OpenFile.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/AnalyzeFile.h>

namespace Visualization {

namespace Concrete {

namespace {

/*****************
Helper structures:
*****************/

struct HeaderKey
	{
	/* Elements: */
	public:
	int headerSize;
	char dataType[10];
	char dataName[18];
	int extents;
	short int sessionError;
	char regular;
	char hkeyUn0;
	
	/* Methods: */
	void read(IO::File& file)
		{
		file.read(headerSize);
		file.read(dataType,10);
		file.read(dataName,18);
		file.read(extents);
		file.read(sessionError);
		file.read(regular);
		file.read(hkeyUn0);
		}
	};

struct ImageDimension
	{
	/* Elements: */
	public:
	short int dim[8];
	short int unused[7];
	short int dataType;
	short int bitPix;
	short int dimUn0;
	float pixDim[8];
	float voxOffset;
	float fUnused[3];
	float calMax;
	float calMin;
	float compressed;
	float verified;
	int glMax;
	int glMin;
	
	/* Methods: */
	void read(IO::File& file)
		{
		file.read(dim,8);
		file.read(unused,7);
		file.read(dataType);
		file.read(bitPix);
		file.read(dimUn0);
		file.read(pixDim,8);
		file.read(voxOffset);
		file.read(fUnused,3);
		file.read(calMax);
		file.read(calMin);
		file.read(compressed);
		file.read(verified);
		file.read(glMax);
		file.read(glMin);
		}
	};

/****************
Helper functions:
****************/

template <class ScalarParam>
void readArray(IO::File& file,Misc::Array<float,3>& array)
	{
	/* Create a temporary array to read a slice of source data: */
	size_t sliceSize=array.getSize(1)*array.getSize(2);
	ScalarParam* slice=new ScalarParam[sliceSize];
	
	/* Read the data by slice in negative order to flip data orientation: */
	float* slicePtr=array.getArray()+(array.getSize(0)-1)*sliceSize;
	for(int z=array.getSize(0)-1;z>=0;--z,slicePtr-=sliceSize)
		{
		/* Read the data slice: */
		file.read<ScalarParam>(slice,sliceSize);
		
		/* Copy and convert the data: */
		const ScalarParam* sPtr=slice;
		float* dPtr=slicePtr;
		for(size_t i=0;i<sliceSize;++i,++sPtr,++dPtr)
			*dPtr=float(*sPtr);
		}
	
	delete[] slice;
	}

}

/****************************
Methods of class AnalyzeFile:
****************************/

AnalyzeFile::AnalyzeFile(void)
	:BaseModule("AnalyzeFile")
	{
	}

Visualization::Abstract::DataSet* AnalyzeFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the Analyze 7.5 header file: */
	std::string headerFileName=args[0];
	headerFileName.append(".hdr");
	IO::SeekableFilePtr headerFile(openSeekableFile(headerFileName,pipe));
	
	/* Read the header key as little-endian: */
	HeaderKey hk;
	Misc::Endianness endianness=Misc::LittleEndian;
	headerFile->setEndianness(endianness);
	hk.read(*headerFile);
	
	/* Check the header size: */
	if(hk.headerSize!=348)
		{
		/* Must be wrong endianness; re-read: */
		endianness=Misc::BigEndian;
		headerFile->setEndianness(endianness);
		headerFile->setReadPosAbs(0);
		hk.read(*headerFile);
		
		/* Re-check header size: */
		if(hk.headerSize!=348)
			Misc::throwStdErr("AnalyzeFile::load: Illegal header size in input file %s",headerFileName.c_str());
		}
	
	/* Read the image dimensions: */
	ImageDimension imageDim;
	imageDim.read(*headerFile);
	
	/* Create the data set: */
	DS::Index numVertices;
	DS::Size cellSize;
	for(int i=0;i<3;++i)
		{
		numVertices[i]=imageDim.dim[3-i];
		cellSize[i]=imageDim.pixDim[3-i];
		}
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Open the image file: */
	std::string imageFileName=args[0];
	imageFileName.append(".img");
	IO::FilePtr imageFile(openFile(imageFileName,pipe));
	imageFile->setEndianness(endianness);
	
	/* Read the vertex values from file: */
	switch(imageDim.dataType)
		{
		case 2: // unsigned char
			readArray<unsigned char>(*imageFile,result->getDs().getVertices());
			break;
		
		case 4: // signed short
			readArray<signed short int>(*imageFile,result->getDs().getVertices());
			break;
		
		case 8: // signed int
			readArray<signed int>(*imageFile,result->getDs().getVertices());
			break;
		
		case 16: // float
			readArray<float>(*imageFile,result->getDs().getVertices());
			break;
		
		case 64: // double
			readArray<double>(*imageFile,result->getDs().getVertices());
			break;
		
		default:
			Misc::throwStdErr("AnalyzeFile::load: Unsupported data type %d in input file %s",imageDim.dataType,imageFileName.c_str());
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
	Visualization::Concrete::AnalyzeFile* module=new Visualization::Concrete::AnalyzeFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
