/***********************************************************************
DicomImageFileReader - Functions to extract image slices from DICOM
image files.
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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdexcept>
#include <vector>
#include <Misc/ThrowStdErr.h>
#include <Misc/Array.h>
#include <Misc/File.h>
#include <Math/Math.h>

#include <Concrete/DicomImageFile.h>

namespace Visualization {

namespace Concrete {

/**************************************
Methods of class DicomImageInformation:
**************************************/

DicomImageInformation::DicomImageInformation(void)
	:fileName(new char[1]),
	 stackIndex(-1),
	 sliceThickness(0.0f),
	 pixelSamples(0),
	 pixelSigned(false),
	 pixelBits(0),
	 pixelBitsUsed(0),
	 pixelBitsMSB(0),
	 imageOffset(0)
	{
	fileName[0]='\0';
	imagePos[0]=imagePos[1]=imagePos[2]=0.0f;
	imageSize[0]=imageSize[1]=0;
	pixelSize[0]=pixelSize[1]=0.0f;
	}

DicomImageInformation::DicomImageInformation(const char* sFileName)
	:fileName(new char[strlen(sFileName)+1]),
	 stackIndex(-1),
	 sliceThickness(0.0f),
	 pixelSamples(0),
	 pixelSigned(false),
	 pixelBits(0),
	 pixelBitsUsed(0),
	 pixelBitsMSB(0),
	 imageOffset(0)
	{
	strcpy(fileName,sFileName);
	imagePos[0]=imagePos[1]=imagePos[2]=0.0f;
	imageSize[0]=imageSize[1]=0;
	pixelSize[0]=pixelSize[1]=0.0f;
	}

DicomImageInformation::DicomImageInformation(const DicomImageInformation& source)
	:fileName(new char[strlen(source.fileName)+1]),
	 stackIndex(source.stackIndex),
	 sliceThickness(source.sliceThickness),
	 pixelSamples(source.pixelSamples),
	 pixelSigned(source.pixelSigned),
	 pixelBits(source.pixelBits),
	 pixelBitsUsed(source.pixelBitsUsed),
	 pixelBitsMSB(source.pixelBitsMSB),
	 imageOffset(source.imageOffset)
	{
	strcpy(fileName,source.fileName);
	for(int i=0;i<3;++i)
		imagePos[i]=source.imagePos[i];
	for(int i=0;i<2;++i)
		imageSize[i]=source.imageSize[i];
	for(int i=0;i<2;++i)
		pixelSize[i]=source.pixelSize[i];
	}

DicomImageInformation& DicomImageInformation::operator=(const DicomImageInformation& source)
	{
	if(this!=&source)
		{
		delete[] fileName;
		fileName=0;
		fileName=new char[strlen(source.fileName)+1];
		strcpy(fileName,source.fileName);
		stackIndex=source.stackIndex;
		sliceThickness=source.sliceThickness;
		pixelSamples=source.pixelSamples;
		pixelSigned=source.pixelSigned;
		pixelBits=source.pixelBits;
		pixelBitsUsed=source.pixelBitsUsed;
		pixelBitsMSB=source.pixelBitsMSB;
		imageOffset=source.imageOffset;
		for(int i=0;i<3;++i)
			imagePos[i]=source.imagePos[i];
		for(int i=0;i<2;++i)
			imageSize[i]=source.imageSize[i];
		for(int i=0;i<2;++i)
			pixelSize[i]=source.pixelSize[i];
		}
	return *this;
	}

DicomImageInformation::~DicomImageInformation(void)
	{
	delete[] fileName;
	}

/**************************************
Methods of class DicomImageInformation:
**************************************/

DicomImageStackInformation::DicomImageStackInformation(const DicomImageStackInformation::Index& arraySize)
	:array(arraySize)
	{
	}

/****************************************
Functions operating on DICOM image files:
****************************************/

inline char* readString(Misc::File& dicomFile,char* buffer,int valueLength)
	{
	dicomFile.read(buffer,valueLength);
	buffer[valueLength]='\0';
	return buffer;
	}

DicomImageInformation readDicomImageInformation(const char* dicomFileName)
	{
	enum TransferSyntax
		{
		UNKNOWN,
		IMPLICITVR_LITTLEENDIAN,
		EXPLICITVR_LITTLEENDIAN,
		EXPLICITVR_BIGENDIAN
		};
	
	DicomImageInformation result(dicomFileName);
	
	/* Open the DICOM file: */
	Misc::File dcmFile(dicomFileName,"rb",Misc::File::LittleEndian);
	
	/* Skip the preamble: */
	dcmFile.seekSet(128);
	
	/* Read the prefix: */
	char prefix[4];
	dcmFile.read(prefix,sizeof(prefix));
	if(strncasecmp(prefix,"DICM",4)!=0)
		Misc::throwStdErr("readDicomImageInformation: file \"%s\" is not a DICOM file",dicomFileName);
	
	/* Read all data elements in the DICOM file: */
	TransferSyntax transferSyntax=IMPLICITVR_LITTLEENDIAN;
	while(!dcmFile.eof())
		{
		/* Read the next tag: */
		unsigned short int tag[2];
		dcmFile.read(tag,2);
		
		/* Read the value length: */
		unsigned int valueLength;
		if(tag[0]==0x0002||transferSyntax==EXPLICITVR_LITTLEENDIAN)
			{
			/* Explicit VR, little endian transfer syntax: */
			
			/* Read the value representation: */
			char vrName[2];
			dcmFile.read(vrName,2);
			
			/* Read the value length: */
			if((toupper(vrName[0])=='O'&&(toupper(vrName[1])=='B'||toupper(vrName[1])=='F'||toupper(vrName[1])=='W'))
			 ||(toupper(vrName[0])=='S'&&toupper(vrName[1])=='Q')
			 ||(toupper(vrName[0])=='U'&&(toupper(vrName[1])=='N'||toupper(vrName[1])=='T')))
				{
				char pad[2];
				dcmFile.read(pad,2);
				valueLength=dcmFile.read<unsigned int>();
				}
			else
				valueLength=dcmFile.read<unsigned short int>();
			}
		else if(transferSyntax==IMPLICITVR_LITTLEENDIAN)
			{
			/* Implicit VR, little endian transfer syntax: */
			
			/* Read the value length: */
			valueLength=dcmFile.read<unsigned int>();
			}
		
		/* Extract image-relevant information: */
		char value[256];
		if(tag[0]==0x0018&&tag[1]==0x0050) // Slice thickness
			result.sliceThickness=float(atof(readString(dcmFile,value,valueLength)));
		else if(tag[0]==0x0020&&tag[1]==0x0013) // Instance number (index in image stack)
			result.stackIndex=atoi(readString(dcmFile,value,valueLength));
		else if(tag[0]==0x0020&&tag[1]==0x0032) // Image position
			{
			char* start=readString(dcmFile,value,valueLength);
			for(int i=0;i<3;++i)
				{
				/* Find the end of the current value: */
				char* end;
				for(end=start;*end!='\0'&&*end!='\\';++end)
					;
				*end='\0';
				
				/* Convert the current value: */
				result.imagePos[i]=float(atof(start));
				
				/* Go to the next value: */
				start=end+1;
				}
			}
		else if(tag[0]==0x0028&&tag[1]==0x0002) // Samples per pixel
			result.pixelSamples=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0010) // Number of image rows
			result.imageSize[1]=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0011) // Number of image columns
			result.imageSize[0]=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0030) // Pixel spacing
			{
			char* start=readString(dcmFile,value,valueLength);
			for(int i=0;i<2;++i)
				{
				/* Find the end of the current value: */
				char* end;
				for(end=start;*end!='\0'&&*end!='\\';++end)
					;
				*end='\0';
				
				/* Convert the current value: */
				result.pixelSize[i]=float(atof(start));
				
				/* Go to the next value: */
				start=end+1;
				}
			}
		else if(tag[0]==0x0028&&tag[1]==0x0100) // Bits per pixel
			result.pixelBits=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0101) // Bits per pixel used
			result.pixelBitsUsed=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0102) // High bit index
			result.pixelBitsMSB=dcmFile.read<unsigned short>();
		else if(tag[0]==0x0028&&tag[1]==0x0103) // Pixel representation
			result.pixelSigned=dcmFile.read<unsigned short>()!=0;
		else if(tag[0]==0x7fe0&&tag[1]==0x0010) // Pixel data
			{
			int pixelBytes=(result.pixelBits+7)/8;
			unsigned int imageLength=(unsigned int)(result.imageSize[0]*result.imageSize[1]*pixelBytes);
			if(valueLength==imageLength)
				result.imageOffset=dcmFile.tell();
			break;
			}
		else // Skip any other data
			dcmFile.seekCurrent(valueLength);
		}
	
	if(!result.isValid())
		Misc::throwStdErr("readDicomImageInformation: DICOM file \"%s\" does not contain an image",dicomFileName);
	return result;
	}

void readDicomImage(const DicomImageInformation& imageInformation,unsigned short* sliceBase,const ptrdiff_t increments[2])
	{
	/* Open the DICOM file: */
	Misc::File dcmFile(imageInformation.fileName,"rb",Misc::File::LittleEndian);
	
	/* Move to the start of the raw image data: */
	dcmFile.seekSet(imageInformation.imageOffset);
	
	/* Read the image data: */
	if(imageInformation.pixelBits==8)
		{
		/* Read unsigned char pixels: */
		int pixelShift=imageInformation.pixelBitsMSB+1-imageInformation.pixelBitsUsed;
		unsigned short pixelMask=(unsigned short)((0x1<<(imageInformation.pixelBitsUsed))-1);
		
		unsigned char* imageRow=new unsigned char[imageInformation.imageSize[0]];
		for(int y=0;y<imageInformation.imageSize[1];++y)
			{
			dcmFile.read(imageRow,imageInformation.imageSize[0]);
			unsigned short* rowPtr=sliceBase+increments[1]*y;
			for(int x=0;x<imageInformation.imageSize[0];++x,rowPtr+=increments[0])
				*rowPtr=(((unsigned short)imageRow[x])>>pixelShift)&pixelMask;
			}
		delete[] imageRow;
		}
	else if(imageInformation.pixelBits==16)
		{
		/* Read unsigned short pixels: */
		int pixelShift=imageInformation.pixelBitsMSB+1-imageInformation.pixelBitsUsed;
		unsigned short pixelMask=(unsigned short)((0x1<<(imageInformation.pixelBitsUsed))-1);
		
		unsigned short* imageRow=new unsigned short[imageInformation.imageSize[0]];
		for(int y=0;y<imageInformation.imageSize[1];++y)
			{
			dcmFile.read(imageRow,imageInformation.imageSize[0]);
			unsigned short* rowPtr=sliceBase+increments[1]*y;
			for(int x=0;x<imageInformation.imageSize[0];++x,rowPtr+=increments[0])
				*rowPtr=(imageRow[x]>>pixelShift)&pixelMask;
			}
		delete[] imageRow;
		}
	}

DicomImageStackInformation* readDicomImageStack(const char* dicomSliceDirectory)
	{
	/* Extract the image metadata from all DICOM images inside the given directory: */
	DIR* directory=opendir(dicomSliceDirectory);
	if(directory==0)
		Misc::throwStdErr("readDicomImageStack: Could not read directory \"%s\"",dicomSliceDirectory);
	
	std::vector<DicomImageInformation> images;
	while(true)
		{
		/* Read the next directory entry: */
		struct dirent* dirEntry=readdir(directory);
		if(dirEntry==0)
			break;
		
		/* Try extracting DICOM image metadata from the directory entry: */
		try
			{
			char fileName[1024];
			strcpy(fileName,dicomSliceDirectory);
			strcat(fileName,dirEntry->d_name);
			DicomImageInformation dii=readDicomImageInformation(fileName);
			images.push_back(dii);
			}
		catch(std::runtime_error)
			{
			/* Ignore this file */
			}
		}
	
	closedir(directory);
	
	/* Check if the slices form a consistent image stack: */
	std::vector<DicomImageInformation>::const_iterator iIt=images.begin();
	if(iIt==images.end())
		Misc::throwStdErr("readDicomImageStack: directory \"%s\" does not contain slices",dicomSliceDirectory);
	int stackIndexMin=iIt->stackIndex;
	int stackIndexMax=iIt->stackIndex;
	float slicePosMin=iIt->imagePos[2];
	int stackImageSize[2];
	stackImageSize[0]=iIt->imageSize[0];
	stackImageSize[1]=iIt->imageSize[1];
	float stackPixelSize[2];
	stackPixelSize[0]=iIt->pixelSize[0];
	stackPixelSize[1]=iIt->pixelSize[1];
	float stackSliceThickness=iIt->sliceThickness;
	bool stackValid=true;
	for(++iIt;iIt!=images.end();++iIt)
		{
		if(stackIndexMin>iIt->stackIndex)
			stackIndexMin=iIt->stackIndex;
		else if(stackIndexMax<iIt->stackIndex)
			stackIndexMax=iIt->stackIndex;
		if(slicePosMin>iIt->imagePos[2])
			slicePosMin=iIt->imagePos[2];
		
		if(iIt->imageSize[0]!=stackImageSize[0]||iIt->imageSize[1]!=stackImageSize[1])
			stackValid=false;
		if(iIt->pixelSize[0]!=stackPixelSize[0]||iIt->pixelSize[1]!=stackPixelSize[1])
			stackValid=false;
		if(iIt->sliceThickness!=stackSliceThickness)
			stackValid=false;
		}
	int numSlices=stackIndexMax-stackIndexMin+1;
	bool* haveSlices=new bool[numSlices];
	for(int i=0;i<numSlices;++i)
		haveSlices[i]=false;
	for(iIt=images.begin();iIt!=images.end();++iIt)
		{
		haveSlices[iIt->stackIndex-stackIndexMin]=true;
		#if 0
		float slicePos=slicePosMin+float(iIt->stackIndex-stackIndexMin)*stackSliceThickness;
		if(Math::abs(slicePos-iIt->imagePos[2])>stackSliceThickness*0.125f)
			stackValid=false;
		#endif
		}
	for(int i=0;i<numSlices;++i)
		if(!haveSlices[i])
			stackValid=false;
	delete[] haveSlices;
	
	if(!stackValid)
		Misc::throwStdErr("readDicomImageStack: slice images in \"%s\" do not form a consistent image stack",dicomSliceDirectory);
	
	/* Create the result structure: */
	DicomImageStackInformation* result=new DicomImageStackInformation(DicomImageStackInformation::Index(numSlices,stackImageSize[1],stackImageSize[0]));
	result->cellSize[0]=stackSliceThickness;
	result->cellSize[1]=stackPixelSize[1];
	result->cellSize[2]=stackPixelSize[0];
	
	/* Read each slice: */
	ptrdiff_t increments[2];
	increments[0]=result->array.getIncrement(2);
	increments[1]=result->array.getIncrement(1);
	for(iIt=images.begin();iIt!=images.end();++iIt)
		{
		unsigned short* sliceBase=result->array.getAddress(iIt->stackIndex-stackIndexMin,0,0);
		readDicomImage(*iIt,sliceBase,increments);
		}
	
	return result;
	}

}

}
