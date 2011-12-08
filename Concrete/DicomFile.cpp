/***********************************************************************
DicomFile - Class to represent and extract images from DICOM interchange
files.
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

#include <Concrete/DicomFile.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <Misc/SizedTypes.h>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/HashTable.h>
#include <IO/FixedMemoryFile.h>

#include <Concrete/JPEGImageWriter.h>
#include <Concrete/JPEGDecompressor.h>

namespace Visualization {

namespace Concrete {

namespace {

/**************
Helper classes:
**************/

struct DicomDataElement
	{
	/* Embedded classes: */
	public:
	enum ValueRepresentation // Codes for DICOM value representations
		{
		INVALID=0x0000,
		AE=0x4145,AS=0x4153,AT=0x4154,
		CS=0x4353,
		DA=0x4441,DS=0x4453,DT=0x4454,
		FD=0x4644,FL=0x464c,
		IS=0x4953,
		LO=0x4c4f,LT=0x4c54,
		OB=0x4f42,OF=0x4f46,OW=0x4f57,
		PN=0x504e,
		SH=0x5348,SL=0x534c,SQ=0x5351,SS=0x5353,ST=0x5354,
		TM=0x544d,
		UI=0x5549,UL=0x554c,UN=0x554e,US=0x5553,UT=0x5554
		};
	
	/* Elements: */
	static const Misc::UInt16 sequenceTags[][2]; // Array of tags that imply a sequence
	Misc::UInt16 tag[2]; // The data element's tag
	unsigned int vr; // The data element's value representation
	unsigned int valueSize; // The data element's value size
	
	/* Methods: */
	bool read(IO::File& dcmFile,DicomFile::VrMode vrMode); // Reads the next data element from a DICOM file; returns false at end of file
	bool isSequence(void) const // Returns true if the data element is an explicit or implicit (darn that!) sequence
		{
		/* Check for explicit type information: */
		if(vr==SQ)
			return true;
		
		/* Check dictionary for implicit type information: */
		for(int i=0;sequenceTags[i][0]!=0xffffU;++i)
			if(sequenceTags[i][0]==tag[0]&&sequenceTags[i][1]==tag[1])
				return true;
		
		return false;
		};
	bool is(unsigned short int group,unsigned short int element) const // Checks if the data element is the given tag
		{
		return tag[0]==group&&tag[1]==element;
		};
	void skipValue(IO::File& dcmFile) const; // Skips the data element's value in the DICOM file
	char* readValue(IO::File& dcmFile) const; // Reads the data element's value into a newly created char array
	int readIntValues(IO::File& dcmFile,int numValues,int values[]) const; // Reads an array of string-encoded integer values; returns number of values in string
	int readFloatValues(IO::File& dcmFile,int numValues,float values[]) const; // Reads an array of string-encoded floating-point values; returns number of values in string
	};

/*****************************************
Static elements of class DicomDataElement:
*****************************************/

const Misc::UInt16 DicomDataElement::sequenceTags[][2]=
	{
	{0x0004,0x1220},
	{0x0008,0x0082},{0x0008,0x0096},{0x0008,0x0110},{0x0008,0x1032},
	{0x0008,0x1049},{0x0008,0x1052},{0x0008,0x1062},{0x0008,0x1072},
	{0x0008,0x1084},{0x0008,0x1100},{0x0008,0x1110},{0x0008,0x1111},
	{0x0008,0x1115},{0x0008,0x1120},{0x0008,0x1125},{0x0008,0x1130},
	{0x0008,0x113a},{0x0008,0x1140},{0x0008,0x1145},{0x0008,0x114a},
	{0x0008,0x1198},{0x0008,0x1199},{0x0008,0x2112},{0x0008,0x2218},
	{0x0008,0x2220},{0x0008,0x2228},{0x0008,0x2229},{0x0008,0x2230},
	{0x0008,0x2240},{0x0008,0x2242},{0x0008,0x2244},{0x0008,0x2246},
	{0x0008,0x9092},{0x0008,0x9121},{0x0008,0x9124},{0x0008,0x9154},
	{0x0008,0x9215},{0x0008,0x9237},
	{0x0010,0x0050},{0x0010,0x0101},{0x0010,0x0102},
	{0x0018,0x0012},{0x0018,0x0014},{0x0018,0x0026},{0x0018,0x0029},
	{0x0018,0x002a},{0x0018,0x0036},{0x0018,0x5104},{0x0018,0x6011},
	{0x0018,0x9006},{0x0018,0x9042},{0x0018,0x9045},{0x0018,0x9049},
	{0x0018,0x9076},{0x0018,0x9084},{0x0018,0x9103},{0x0018,0x9107},
	{0x0018,0x9112},{0x0018,0x9114},{0x0018,0x9115},{0x0018,0x9117},
	{0x0018,0x9118},{0x0018,0x9119},{0x0018,0x9125},{0x0018,0x9126},
	{0x0018,0x9152},{0x0018,0x9176},{0x0018,0x9197},{0x0018,0x9226},
	{0x0018,0x9227},{0x0018,0x9239},{0x0018,0xa001},
	{0x0020,0x9071},{0x0020,0x9111},{0x0020,0x9113},{0x0020,0x9116},
	{0x0020,0x9221},{0x0020,0x9222},
	{0x0028,0x3000},{0x0028,0x3010},{0x0028,0x3110},{0x0028,0x5000},
	{0x0028,0x6100},{0x0028,0x9110},{0x0028,0x9132},{0x0028,0x9145},
	{0x0032,0x1031},{0x0032,0x1064},
	{0x0038,0x0004},{0x0038,0x0044},
	{0x003a,0x0200},{0x003a,0x0208},{0x003a,0x0209},{0x003a,0x020a},
	{0x003a,0x0211},
	{0x0040,0x0008},{0x0040,0x000a},{0x0040,0x000b},{0x0040,0x0100},
	{0x0040,0x0220},{0x0040,0x0260},{0x0040,0x0270},{0x0040,0x0275},
	{0x0040,0x0281},{0x0040,0x0293},{0x0040,0x0295},{0x0040,0x0296},
	{0x0040,0x030e},{0x0040,0x0320},{0x0040,0x0321},{0x0040,0x0324},
	{0x0040,0x0330},{0x0040,0x0340},{0x0040,0x0550},{0x0040,0x0555},
	{0x0040,0x059a},{0x0040,0x071a},{0x0040,0x08d8},{0x0040,0x08da},
	{0x0040,0x08ea},{0x0040,0x1011},{0x0040,0x1101},{0x0040,0x4004},
	{0x0040,0x4007},{0x0040,0x4009},{0x0040,0x4015},{0x0040,0x4016},
	{0x0040,0x4018},{0x0040,0x4019},{0x0040,0x4021},{0x0040,0x4022},
	{0x0040,0x4025},{0x0040,0x4026},{0x0040,0x4027},{0x0040,0x4028},
	{0x0040,0x4029},{0x0040,0x4030},{0x0040,0x4031},{0x0040,0x4032},
	{0x0040,0x4033},{0x0040,0x4034},{0x0040,0x4035},{0x0040,0x9096},
	{0x0040,0xa043},{0x0040,0xa073},{0x0040,0xa088},{0x0040,0xa168},
	{0x0040,0xa170},{0x0040,0xa195},{0x0040,0xa300},{0x0040,0xa301},
	{0x0040,0xa360},{0x0040,0xa370},{0x0040,0xa372},{0x0040,0xa375},
	{0x0040,0xa385},{0x0040,0xa504},{0x0040,0xa525},{0x0040,0xa730},
	{0x0040,0xb020},
	{0x0050,0x0010},
	{0x0054,0x0012},{0x0054,0x0013},{0x0054,0x0016},{0x0054,0x0022},
	{0x0054,0x0032},{0x0054,0x0052},{0x0054,0x0062},{0x0054,0x0063},
	{0x0054,0x0072},{0x0054,0x0220},{0x0054,0x0222},{0x0054,0x0300},
	{0x0054,0x0302},{0x0054,0x0304},{0x0054,0x0306},{0x0054,0x0410},
	{0x0054,0x0412},{0x0054,0x0414},
	{0x0060,0x3000},
	{0x0070,0x0001},{0x0070,0x0008},{0x0070,0x0009},{0x0070,0x005a},
	{0x0070,0x0060},
	{0x0088,0x0200},
	{0x0400,0x0500},{0x0400,0x0550},
	{0x2000,0x001e},{0x2000,0x00a2},{0x2000,0x00a4},{0x2000,0x00a8},
	{0x2000,0x0500},{0x2000,0x0510},
	{0x2010,0x0500},{0x2010,0x0510},{0x2010,0x0520},
	{0x2020,0x0110},{0x2020,0x0111},{0x2020,0x0130},{0x2020,0x0140},
	{0x2040,0x0010},{0x2040,0x0020},{0x2040,0x0500},
	{0x2050,0x0010},{0x2050,0x0500},
	{0x2100,0x0500},
	{0x2120,0x0050},{0x2120,0x0070},
	{0x2130,0x0010},{0x2130,0x0015},{0x2130,0x0030},{0x2130,0x0040},
	{0x2130,0x0050},{0x2130,0x0060},{0x2130,0x0080},{0x2130,0x00a0},
	{0x2130,0x00c0},
	{0x3002,0x0030},
	{0x3004,0x0010},{0x3004,0x0050},{0x3004,0x0060},
	{0x3006,0x0010},{0x3006,0x0012},{0x3006,0x0014},{0x3006,0x0016},
	{0x3006,0x0020},{0x3006,0x0030},{0x3006,0x0039},{0x3006,0x0040},
	{0x3006,0x0080},{0x3006,0x0086},{0x3006,0x00a0},{0x3006,0x00b0},
	{0x3006,0x00c0},
	{0x3008,0x0010},{0x3008,0x0020},{0x3008,0x0030},{0x3008,0x0040},
	{0x3008,0x0050},{0x3008,0x0060},{0x3008,0x0070},{0x3008,0x0080},
	{0x3008,0x0090},{0x3008,0x00a0},{0x3008,0x00b0},{0x3008,0x00c0},
	{0x3008,0x00d0},{0x3008,0x00e0},{0x3008,0x0100},{0x3008,0x0110},
	{0x3008,0x0120},{0x3008,0x0130},{0x3008,0x0140},{0x3008,0x0150},
	{0x3008,0x0160},{0x3008,0x0220},{0x3008,0x0240},
	{0x300a,0x0010},{0x300a,0x0040},{0x300a,0x0048},{0x300a,0x0070},
	{0x300a,0x00b0},{0x300a,0x00b6},{0x300a,0x00ca},{0x300a,0x00d1},
	{0x300a,0x00e3},{0x300a,0x00f4},{0x300a,0x0107},{0x300a,0x0111},
	{0x300a,0x0116},{0x300a,0x011a},{0x300a,0x0180},{0x300a,0x0190},
	{0x300a,0x01a0},{0x300a,0x01b4},{0x300a,0x0206},{0x300a,0x0210},
	{0x300a,0x0230},{0x300a,0x0260},{0x300a,0x0280},{0x300a,0x02b0},
	{0x300a,0x02d0},
	{0x300c,0x0002},{0x300c,0x0004},{0x300c,0x000a},{0x300c,0x0020},
	{0x300c,0x0040},{0x300c,0x0042},{0x300c,0x0050},{0x300c,0x0055},
	{0x300c,0x0060},{0x300c,0x0080},{0x300c,0x00b0},
	{0x4008,0x0050},{0x4008,0x0111},{0x4008,0x0117},{0x4008,0x0118},
	{0x4ffe,0x0001},
	{0x5000,0x2600}, // Actually replicated for all 0x50xx
	{0x5200,0x9229},{0x5200,0x9230},
	{0x5400,0x0100},
	{0xfffa,0xfffa},
	{0xffff,0xffff} // Sentinel value
	};

struct DicomSequence
	{
	/* Embedded classes: */
	public:
	typedef IO::SeekableFile::Offset Offset; // Type for file positions
	
	/* Elements: */
	public:
	DicomSequence* succ; // Pointer to next sequence in sequence stack
	Misc::UInt16 tag[2]; // The tag of the data element the sequence belongs to
	bool fixedSize; // Flag if the sequence has a fixed size (hence no end sequence data element)
	Offset endOffset; // Offset of the first tag after the sequence if the sequence has fixed size
	int currentItemIndex; // Index of current sequence item
	bool currentItemFixedSize; // Flag if the current sequence item has a fixed size
	Offset currentItemEndOffset; // Offset of the first tag after the current item if the current item has fixed size
	
	/* Constructors and destructors: */
	DicomSequence(DicomSequence* sSucc,Misc::UInt16 sTag[2])
		:succ(sSucc),fixedSize(false),
		 currentItemIndex(-1),currentItemFixedSize(false)
		{
		for(int i=0;i<2;++i)
			tag[i]=sTag[i];
		};
	};

struct DicomSliceImage
	{
	public:
	int sliceIndex; // Slice index of image
	const DicomFile::Image* image; // Pointer to image structure
	
	/* Constructors and destructors: */
	DicomSliceImage(int sSliceIndex,const DicomFile::Image* sImage)
		:sliceIndex(sSliceIndex),image(sImage)
		{
		};
	
	/* Methods: */
	bool operator()(const DicomSliceImage& si1,const DicomSliceImage& si2) const
		{
		return si1.sliceIndex<si2.sliceIndex;
		};
	};

template <class SourcePixelTypeParam,class DestPixelTypeParam>
class PixelTypeConverter // Default pixel type converter
	{
	/* Embedded classes: */
	public:
	typedef SourcePixelTypeParam SourcePixelType;
	typedef DestPixelTypeParam DestPixelType;
	
	/* Constructors and destructors: */
	PixelTypeConverter(int pixelBitsMSB)
		{
		};
	
	/* Methods: */
	static DestPixelType convertPixelValue(SourcePixelType source)
		{
		return DestPixelType(source);
		};
	};

template <>
class PixelTypeConverter<signed char,unsigned char> // Converter from signed to unsigned char
	{
	/* Embedded classes: */
	public:
	typedef signed char SourcePixelType;
	typedef unsigned char DestPixelType;
	
	/* Constructors and destructors: */
	PixelTypeConverter(int pixelBitsMSB)
		{
		};
	
	/* Methods: */
	static DestPixelType convertPixelValue(SourcePixelType source)
		{
		return DestPixelType(int(source)+128);
		};
	};

template <>
class PixelTypeConverter<unsigned char,signed char> // Converter from unsigned to signed char
	{
	/* Embedded classes: */
	public:
	typedef unsigned char SourcePixelType;
	typedef signed char DestPixelType;
	
	/* Constructors and destructors: */
	PixelTypeConverter(int pixelBitsMSB)
		{
		};
	
	/* Methods: */
	static DestPixelType convertPixelValue(SourcePixelType source)
		{
		return DestPixelType(int(source)-128);
		};
	};

template <>
class PixelTypeConverter<signed short,unsigned char> // Converter from signed short to unsigned char
	{
	/* Embedded classes: */
	public:
	typedef signed short SourcePixelType;
	typedef unsigned char DestPixelType;
	
	/* Elements: */
	private:
	int valueShift; // Number to add to source values to make them unsigned
	int bitShift; // Number of bits to right-shift before conversion
	
	/* Constructors and destructors: */
	public:
	PixelTypeConverter(int pixelBitsMSB)
		:valueShift(1<<pixelBitsMSB),
		 bitShift(pixelBitsMSB-7)
		{
		};
	
	/* Methods: */
	DestPixelType convertPixelValue(SourcePixelType source) const
		{
		return DestPixelType((int(source)+valueShift)>>bitShift);
		};
	};

template <>
class PixelTypeConverter<unsigned short,unsigned char> // Converter from unsigned short to unsigned char
	{
	/* Embedded classes: */
	public:
	typedef unsigned short SourcePixelType;
	typedef unsigned char DestPixelType;
	
	/* Elements: */
	private:
	int bitShift; // Number of bits to right-shift before conversion
	
	/* Constructors and destructors: */
	public:
	PixelTypeConverter(int pixelBitsMSB)
		:bitShift(pixelBitsMSB-7)
		{
		};
	
	/* Methods: */
	DestPixelType convertPixelValue(SourcePixelType source) const
		{
		return DestPixelType(source>>bitShift);
		};
	};

template <>
class PixelTypeConverter<signed short,unsigned short> // Converter from signed to unsigned short
	{
	/* Embedded classes: */
	public:
	typedef signed short SourcePixelType;
	typedef unsigned short DestPixelType;
	
	/* Constructors and destructors: */
	PixelTypeConverter(int pixelBitsMSB)
		{
		};
	
	/* Methods: */
	static DestPixelType convertPixelValue(SourcePixelType source)
		{
		return DestPixelType(int(source)+32768);
		};
	};

template <>
class PixelTypeConverter<unsigned short,signed short> // Converter from unsigned to signed short
	{
	/* Embedded classes: */
	public:
	typedef unsigned short SourcePixelType;
	typedef signed short DestPixelType;
	
	/* Constructors and destructors: */
	PixelTypeConverter(int pixelBitsMSB)
		{
		};
	
	/* Methods: */
	static DestPixelType convertPixelValue(SourcePixelType source)
		{
		return DestPixelType(int(source)-32768);
		};
	};

template <class DestPixelTypeParam>
class MemoryImageWriter:public JPEGImageWriter // Class to write image data into a memory buffer
	{
	/* Embedded classes: */
	public:
	typedef DestPixelTypeParam DestPixelType; // The image's pixel component type
	typedef PixelTypeConverter<short,DestPixelType> PTC; // Class to convert pixel types
	
	/* Elements: */
	private:
	DestPixelType* imageBuffer; // Pointer to the start of the image buffer
	int imageSize[2]; // Size of the image (width, height)
	ptrdiff_t imageBufferStrides[2]; // Stride values for accessing the image buffer
	PTC ptc; // Pixel type converter for this image
	
	/* Constructors and destructors: */
	public:
	MemoryImageWriter(DestPixelType* sImageBuffer,const int sImageSize[2],const ptrdiff_t sImageBufferStrides[2],int pixelBitsMSB)
		:imageBuffer(sImageBuffer),
		 ptc(pixelBitsMSB)
		{
		for(int i=0;i<2;++i)
			{
			imageSize[i]=sImageSize[i];
			imageBufferStrides[i]=sImageBufferStrides[i];
			}
		};
	
	/* Methods: */
	virtual void setImageParameters(const int newImageSize[2],int numScanComponents,int numBits)
		{
		/* Check if the image size matches the allocated size: */
		if(newImageSize[0]!=imageSize[0]||newImageSize[1]!=imageSize[1]||numScanComponents!=1)
			Misc::throwStdErr("MemoryImageWriter::setImageParameters: Mismatching image parameters");
		};
	virtual void writeImageRow(int rowIndex,const short* imageRow)
		{
		const short* irPtr=imageRow;
		DestPixelType* rowPtr=imageBuffer+rowIndex*imageBufferStrides[1];
		for(int col=0;col<imageSize[0];++col,++irPtr,rowPtr+=imageBufferStrides[0])
			*rowPtr=ptc.convertPixelValue(*irPtr);
		};
	};

/*********************************
Methods of class DicomDataElement:
*********************************/

bool DicomDataElement::read(IO::File& dcmFile,DicomFile::VrMode vrMode)
	{
	/* Bail out on end-of-file: */
	if(dcmFile.eof())
		return false;
	
	/* Read the tag: */
	dcmFile.read(tag,2);
	
	/* Determine the value representation: */
	if(tag[0]==0xfffeU&&(tag[1]==0xe000U||tag[1]==0xe00dU||tag[1]==0xe0ddU)) // VR is defined by special tag
		{
		/* Assign dummy VR: */
		vr=UN;
		
		/* Read the value size: */
		valueSize=dcmFile.read<Misc::UInt32>();
		}
	else if(vrMode==DicomFile::VR_IMPLICIT&&tag[0]!=0x0002U) // VR is defined by dictionary
		{
		/* Assign dummy VR: */
		vr=UN;
		
		/* Read the value size: */
		valueSize=dcmFile.read<Misc::UInt32>();
		}
	else // VR is explicitly stored in file
		{
		/* Read the value representation: */
		Misc::UInt8 vrName[2];
		dcmFile.read(vrName,2);
		vr=(unsigned int)(vrName[0])*256U+(unsigned int)(vrName[1]);
		
		/* Read the value size: */
		if(vr==OB||vr==OF||vr==OW||vr==SQ||vr==UN||vr==UT)
			{
			/* Value size is four bytes with padding: */
			dcmFile.skip<Misc::UInt8>(2);
			valueSize=dcmFile.read<Misc::UInt32>();
			}
		else
			{
			/* Value size is two bytes: */
			valueSize=dcmFile.read<Misc::UInt16>();
			}
		}
	
	return true;
	}

void DicomDataElement::skipValue(IO::File& dcmFile) const
	{
	if(valueSize>0U&&valueSize!=0xffffffffU)
		{
		/* Round the value size up to the next even value: */
		dcmFile.skip<Misc::UInt8>((valueSize+1U)&~0x1U);
		}
	}

char* DicomDataElement::readValue(IO::File& dcmFile) const
	{
	char* result;
	if(valueSize>0U&&valueSize!=0xffffffffU)
		{
		/* Create the result array: */
		result=new char[valueSize+1];
		
		/* Round the value size up to the next even value: */
		dcmFile.read(result,(valueSize+1U)&~0x1U);
		
		/* NUL-terminate the result: */
		result[valueSize]='\0';
		}
	else
		{
		/* Create a dummy result: */
		result=new char[1];
		result[0]='\0';
		}
	return result;
	}

int DicomDataElement::readIntValues(IO::File& dcmFile,int numValues,int values[]) const
	{
	/* Read the data element value: */
	char* value=readValue(dcmFile);
	
	/* Parse the value string: */
	char* start=value;
	bool moreValues=true;
	int numValuesRead;
	for(numValuesRead=0;moreValues;++numValuesRead)
		{
		/* Find the end of the current value: */
		char* end;
		for(end=start;*end!='\0'&&*end!='\\';++end)
			;
		if(*end=='\0')
			moreValues=false;
		*end='\0';
		
		if(numValuesRead<numValues)
			{
			/* Convert the current value: */
			values[numValuesRead]=atoi(start);
			}
		
		/* Go to the next value: */
		start=end+1;
		}
	
	/* Clean up: */
	delete[] value;
	
	return numValuesRead;
	}

int DicomDataElement::readFloatValues(IO::File& dcmFile,int numValues,float values[]) const
	{
	/* Read the data element value: */
	char* value=readValue(dcmFile);
	
	/* Parse the value string: */
	char* start=value;
	bool moreValues=true;
	int numValuesRead;
	for(numValuesRead=0;moreValues;++numValuesRead)
		{
		/* Find the end of the current value: */
		char* end;
		for(end=start;*end!='\0'&&*end!='\\';++end)
			;
		if(*end=='\0')
			moreValues=false;
		*end='\0';
		
		if(numValuesRead<numValues)
			{
			/* Convert the current value: */
			values[numValuesRead]=float(atof(start));
			}
		
		/* Go to the next value: */
		start=end+1;
		}
	
	/* Clean up: */
	delete[] value;
	
	return numValuesRead;
	}

/****************
Helper functions:
****************/

template <class SourcePixelType,class DestPixelType>
void readRawImage(IO::File& file,const int imageSize[2],DestPixelType* imageBuffer,const ptrdiff_t imageBufferStrides[2],const PixelTypeConverter<SourcePixelType,DestPixelType>& ptc)
	{
	/* Read the image one row at a time: */
	SourcePixelType* rowBuffer=new SourcePixelType[imageSize[0]];
	for(int y=0;y<imageSize[1];++y)
		{
		/* Read the next image row: */
		file.read(rowBuffer,imageSize[0]);
		
		/* Process the image row into the image buffer: */
		DestPixelType* imageRowPtr=imageBuffer+y*imageBufferStrides[1];
		for(int x=0;x<imageSize[0];++x,imageRowPtr+=imageBufferStrides[0])
			*imageRowPtr=ptc.convertPixelValue(rowBuffer[x]);
		}
	delete[] rowBuffer;
	}

}

/*******************************************
Methods of class DicomFile::ImageDescriptor:
*******************************************/

DicomFile::ImageDescriptor::ImageDescriptor(void)
	:stackIndex(-1),
	 sliceThickness(0.0f),
	 pixelSamples(0),pixelSigned(false),pixelBits(0),pixelBitsUsed(0),pixelBitsMSB(-1),
	 imageOffset(0),imageDataSize(0)
	{
	}

/************************************************
Methods of class DicomFile::ImageStackDescriptor:
************************************************/

DicomFile::ImageStackDescriptor::ImageStackDescriptor(int sNumImages)
	:numImages(sNumImages),sliceThickness(0.0f),imageFileNames(0)
	{
	if(numImages>0)
		{
		imageFileNames=new char*[numImages];
		for(int i=0;i<numImages;++i)
			imageFileNames[i]=0;
		}
	}

DicomFile::ImageStackDescriptor::~ImageStackDescriptor(void)
	{
	for(int i=0;i<numImages;++i)
		delete[] imageFileNames[i];
	delete[] imageFileNames;
	}

/*************************************
Methods of class DicomFile::Directory:
*************************************/

void DicomFile::Directory::printDirectory(int indent) const
	{
	/* Print all child directory entries: */
	for(const Directory* dPtr=firstChild;dPtr!=0;dPtr=dPtr->nextSibling)
		dPtr->printDirectory(indent+1);
	}

const DicomFile::Series* DicomFile::Directory::findSeries(int findSeriesNumber) const
	{
	/* Recurse into the children: */
	const Series* result=0;
	for(const Directory* dPtr=firstChild;dPtr!=0&&result==0;dPtr=dPtr->nextSibling)
		result=dPtr->findSeries(findSeriesNumber);
	
	return result;
	}

DicomFile::Directory::Directory(void)
	:firstChild(0),nextSibling(0)
	{
	}

DicomFile::Directory::~Directory(void)
	{
	/* Destroy all child directory entries: */
	while(firstChild!=0)
		{
		Directory* nextChild=firstChild->nextSibling;
		delete firstChild;
		firstChild=nextChild;
		}
	}

void DicomFile::Directory::printDirectory(void) const
	{
	/* Print the entire directory recursively: */
	printDirectory(0);
	}

void DicomFile::Directory::printSeries(void) const
	{
	/* Recurse into the children: */
	for(const Directory* dPtr=firstChild;dPtr!=0;dPtr=dPtr->nextSibling)
		dPtr->printSeries();
	}

DicomFile::ImageStackDescriptor* DicomFile::Directory::getImageStackDescriptor(int seriesNumber) const
	{
	/* Find the series of the given number: */
	const Series* series=findSeries(seriesNumber);
	if(series==0)
		return 0;
	
	/* Check if the series has any images: */
	if(series->firstChild==0)
		return 0;
	
	/* Sort the series images by slice index: */
	typedef std::vector<DicomSliceImage> SliceImageList;
	SliceImageList sliceImages;
	for(const Directory* dPtr=series->firstChild;dPtr!=0;dPtr=dPtr->nextSibling)
		{
		const Image* iPtr=dynamic_cast<const Image*>(dPtr);
		if(iPtr!=0)
			sliceImages.push_back(DicomSliceImage(iPtr->sliceIndex,iPtr));
		}
	std::sort(sliceImages.begin(),sliceImages.end(),sliceImages.front());
	
	/* Check if the series images are consistent: */
	int firstSliceIndex=sliceImages.front().image->sliceIndex;
	int stackImageSize[2];
	for(int i=0;i<2;++i)
		stackImageSize[i]=sliceImages.front().image->imageSize[i];
	float stackSliceThickness=sliceImages.front().image->sliceThickness;
	float stackPixelSize[2];
	for(int i=0;i<2;++i)
		stackPixelSize[i]=sliceImages.front().image->pixelSize[i];
	SliceImageList::const_iterator predSlIt=sliceImages.begin();
	bool stackConsistent=true;
	SliceImageList::const_iterator slIt=predSlIt;
	for(++slIt;slIt!=sliceImages.end();predSlIt=slIt,++slIt)
		{
		if(slIt->image->sliceIndex!=predSlIt->image->sliceIndex+1
		   ||slIt->image->imageSize[0]!=stackImageSize[0]
		   ||slIt->image->imageSize[1]!=stackImageSize[1]
		   ||slIt->image->sliceThickness!=stackSliceThickness
		   ||slIt->image->pixelSize[0]!=stackPixelSize[0]
		   ||slIt->image->pixelSize[1]!=stackPixelSize[1])
			stackConsistent=false;
		}
	if(!stackConsistent)
		{
		std::cerr<<"DicomFile::Directory::getImageStackDescriptor: Image stack is inconsistent"<<std::endl;
		// return 0;
		}
	
	/* Build the image stack descriptor: */
	ImageStackDescriptor* result=new ImageStackDescriptor(sliceImages.size());
	for(int i=0;i<2;++i)
		result->imageSize[i]=stackImageSize[i];
	for(int i=0;i<3;++i)
		result->stackPosition[i]=sliceImages.front().image->imagePosition[i];
	result->sliceThickness=stackSliceThickness;
	for(int i=0;i<2;++i)
		result->pixelSize[i]=stackPixelSize[i];
	for(SliceImageList::const_iterator slIt=sliceImages.begin();slIt!=sliceImages.end();++slIt)
		{
		result->imageFileNames[slIt->sliceIndex-firstSliceIndex]=new char[strlen(slIt->image->imageFileName)+1];
		strcpy(result->imageFileNames[slIt->sliceIndex-firstSliceIndex],slIt->image->imageFileName);
		}
	
	return result;
	}

/**********************************
Methods of class DicomFile::Series:
**********************************/

void DicomFile::Series::printDirectory(int indent) const
	{
	/* Print the series info: */
	for(int i=0;i<indent;++i)
		std::cout<<'\t';
	std::cout<<"Series "<<std::setw(3)<<seriesNumber<<std::endl;
	
	/* Print all child directory entries: */
	Directory::printDirectory(indent);
	}

const DicomFile::Series* DicomFile::Series::findSeries(int findSeriesNumber) const
	{
	if(findSeriesNumber<0||seriesNumber==findSeriesNumber)
		return this;
	else
		return Directory::findSeries(findSeriesNumber);
	}

DicomFile::Series::Series(void)
	:seriesNumber(-1)
	{
	}

void DicomFile::Series::printSeries(void) const
	{
	/* Collect all images in the series: */
	typedef std::vector<DicomSliceImage> SliceImageList;
	SliceImageList sliceImages;
	for(const Directory* dPtr=getFirstChild();dPtr!=0;dPtr=dPtr->getNextSibling())
		{
		const Image* iPtr=dynamic_cast<const Image*>(dPtr);
		if(iPtr!=0)
			sliceImages.push_back(DicomSliceImage(iPtr->getSliceIndex(),iPtr));
		}
	if(sliceImages.empty())
		return;
	
	/* Sort the series images by slice index: */
	std::sort(sliceImages.begin(),sliceImages.end(),sliceImages.front());
	
	/* Check if the series images are consistent: */
	int firstSliceIndex=sliceImages.front().image->getSliceIndex();
	int stackImageSize[2];
	for(int i=0;i<2;++i)
		stackImageSize[i]=sliceImages.front().image->getImageSize(i);
	float stackSliceThickness=sliceImages.front().image->getSliceThickness();
	float stackPixelSize[2];
	for(int i=0;i<2;++i)
		stackPixelSize[i]=sliceImages.front().image->getPixelSize(i);
	SliceImageList::const_iterator predSlIt=sliceImages.begin();
	bool stackConsistent=true;
	SliceImageList::const_iterator slIt=predSlIt;
	for(++slIt;slIt!=sliceImages.end();predSlIt=slIt,++slIt)
		{
		if(slIt->image->getSliceIndex()!=predSlIt->image->getSliceIndex()+1
		   ||slIt->image->getImageSize(0)!=stackImageSize[0]
		   ||slIt->image->getImageSize(1)!=stackImageSize[1]
		   ||slIt->image->getSliceThickness()!=stackSliceThickness
		   ||slIt->image->getPixelSize(0)!=stackPixelSize[0]
		   ||slIt->image->getPixelSize(1)!=stackPixelSize[1])
			stackConsistent=false;
		}
	if(!stackConsistent)
		{
		std::cerr<<"DicomFile::Series::printSeries: Image stack is inconsistent"<<std::endl;
		// return 0;
		}
	
	/* Print information about the series' image stack: */
	std::cout<<"Series "<<seriesNumber<<": ";
	std::cout<<sliceImages.size()<<" x "<<stackImageSize[0]<<" x "<<stackImageSize[1]<<" starting at index "<<firstSliceIndex<<std::endl;
	}

/*********************************
Methods of class DicomFile::Image:
*********************************/

void DicomFile::Image::printDirectory(int indent) const
	{
	/* Print the image info: */
	for(int i=0;i<indent;++i)
		std::cout<<'\t';
	std::cout<<"Slice "<<std::setw(3)<<sliceIndex<<": "<<std::setw(4)<<imageSize[0]<<" x "<<std::setw(4)<<imageSize[1]<<" pixel image in file "<<imageFileName<<std::endl;
	
	/* Print all child directory entries: */
	Directory::printDirectory(indent);
	}

DicomFile::Image::Image(void)
	:imageFileName(0),sliceIndex(-1),sliceThickness(0.0f)
	{
	for(int i=0;i<2;++i)
		{
		imageSize[i]=0;
		pixelSize[i]=0.0f;
		}
	for(int i=0;i<3;++i)
		imagePosition[i]=0.0f;
	}

DicomFile::Image::~Image(void)
	{
	delete[] imageFileName;
	}

/**************************
Methods of class DicomFile:
**************************/

DicomFile::DicomFile(const char* dcmFileName,IO::SeekableFilePtr sDcmFile)
	:dcmFile(sDcmFile),
	 vrMode(VR_IMPLICIT),fileType(FILE_UNKNOWN),imageType(IMAGETYPE_UNKNOWN),imageMode(IMAGE_RAW)
	{
	dcmFile->setEndianness(Misc::LittleEndian);
	
	/* Skip the preamble: */
	dcmFile->setReadPosAbs(128);
	
	/* Read the prefix: */
	char prefix[4];
	dcmFile->read(prefix,sizeof(prefix));
	if(strncasecmp(prefix,"DICM",4)!=0)
		Misc::throwStdErr("DicomFile::DicomFile: file \"%s\" is not a DICOM file",dcmFileName);
	
	/* Process the header tag group: */
	DicomDataElement de;
	Offset lastTagOffset;
	while(true)
		{
		/* Read the next data element: */
		lastTagOffset=dcmFile->getReadPos();
		de.read(*dcmFile,vrMode);
		
		/* Finish processing if a non-header group tag is encountered: */
		if(de.tag[0]!=0x0002U)
			break;
		
		/* Process the tag: */
		if(de.tag[1]==0x0002U) // Media Storage SOP Class UID
			{
			/* Read the UID value: */
			char* uid=de.readValue(*dcmFile);
			
			/* Determine the file type: */
			if(strcmp(uid,"1.2.840.10008.1.3.10")==0) // Media Storage Directory Storage
				fileType=FILE_DIRECTORY;
			else if(strcmp(uid,"1.2.840.10008.5.1.4.1.1.1")==0) // Computed Radiography Image Storage
				{
				fileType=FILE_IMAGE;
				imageType=IMAGETYPE_CRI;
				}
			else if(strcmp(uid,"1.2.840.10008.5.1.4.1.1.2")==0) // CT Image Storage
				{
				fileType=FILE_IMAGE;
				imageType=IMAGETYPE_CTI;
				}
			else if(strcmp(uid,"1.2.840.10008.5.1.4.1.1.4")==0) // MR Image Storage
				{
				fileType=FILE_IMAGE;
				imageType=IMAGETYPE_MRI;
				}
			
			delete[] uid;
			}
		else if(de.tag[1]==0x0010U) // Transfer Syntax UID
			{
			/* Read the UID value: */
			char* uid=de.readValue(*dcmFile);
			
			/* Determine the transfer syntax: */
			if(strcmp(uid,"1.2.840.10008.1.2")==0) // Implicit VR Little Endian
				{
				vrMode=VR_IMPLICIT;
				dcmFile->setEndianness(Misc::LittleEndian);
				}
			else if(strcmp(uid,"1.2.840.10008.1.2.1")==0) // Explicit VR Little Endian
				{
				vrMode=VR_EXPLICIT;
				dcmFile->setEndianness(Misc::LittleEndian);
				}
			else if(strcmp(uid,"1.2.840.10008.1.2.2")==0) // Explicit VR Big Endian
				{
				vrMode=VR_EXPLICIT;
				dcmFile->setEndianness(Misc::BigEndian);
				}
			else if(strcmp(uid,"1.2.840.10008.1.2.4.50")==0) // Lossy JPEG
				{
				vrMode=VR_EXPLICIT;
				dcmFile->setEndianness(Misc::LittleEndian);
				imageMode=IMAGE_JPEG_LOSSY;
				}
			else if(strcmp(uid,"1.2.840.10008.1.2.4.70")==0) // Lossless JPEG
				{
				vrMode=VR_EXPLICIT;
				dcmFile->setEndianness(Misc::LittleEndian);
				imageMode=IMAGE_JPEG_LOSSLESS;
				}
			else if(strcmp(uid,"1.2.840.10008.1.2.5")==0) // RLE
				{
				vrMode=VR_EXPLICIT;
				dcmFile->setEndianness(Misc::LittleEndian);
				imageMode=IMAGE_RLE;
				}
			
			delete[] uid;
			}
		else
			de.skipValue(*dcmFile);
		}
	
	/* Rewind to the last read tag: */
	dcmFile->setReadPosAbs(lastTagOffset);
	}

DicomFile::~DicomFile(void)
	{
	}

DicomFile::ImageDescriptor* DicomFile::readImageDescriptor(void)
	{
	if(fileType!=FILE_IMAGE)
		Misc::throwStdErr("DicomFile::readImageDescriptor: file is not an image file");
	
	/* Create the result image descriptor: */
	Misc::SelfDestructPointer<ImageDescriptor> result(new ImageDescriptor);
	
	/* Read all data elements: */
	DicomSequence* sequenceTop=0;
	DicomDataElement de;
	while(true)
		{
		/* Check if the next data element ends the current sequence: */
		Offset deOffset=dcmFile->getReadPos();
		if(sequenceTop!=0&&sequenceTop->fixedSize&&sequenceTop->endOffset==deOffset)
			{
			/* End the current sequence: */
			DicomSequence* newSequenceTop=sequenceTop->succ;
			delete sequenceTop;
			sequenceTop=newSequenceTop;
			}
		
		/* Read the next data element: */
		if(!de.read(*dcmFile,vrMode))
			break;
		
		/* Handle sequences: */
		if(de.isSequence())
			{
			/* Start a new sequence: */
			sequenceTop=new DicomSequence(sequenceTop,de.tag);
			
			/* Check if it is a fixed-size sequence: */
			if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
				{
				sequenceTop->fixedSize=true;
				sequenceTop->endOffset=dcmFile->getReadPos()+Offset(de.valueSize);
				}
			}
		
		/* Parse relevant information: */
		if(de.is(0x0018U,0x0050U)) // Slice thickness
			{
			char* value=de.readValue(*dcmFile);
			result->sliceThickness=float(atof(value));
			delete[] value;
			}
		else if(de.is(0x0020U,0x0013U)) // Instance number (index in image stack)
			{
			char* value=de.readValue(*dcmFile);
			result->stackIndex=atoi(value);
			delete[] value;
			}
		else if(de.is(0x0020U,0x0032U)) // Image position
			{
			de.readFloatValues(*dcmFile,3,result->imagePos);
			}
		else if(de.is(0x0028U,0x0002U)) // Samples per pixel
			{
			result->pixelSamples=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0010U)) // Number of image rows
			{
			result->imageSize[1]=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0011U)) // Number of image columns
			{
			result->imageSize[0]=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0030U)) // Pixel size
			{
			de.readFloatValues(*dcmFile,2,result->pixelSize);
			}
		else if(de.is(0x0028U,0x0100U)) // Bits per pixel
			{
			result->pixelBits=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0101U)) // Bits per pixel used
			{
			result->pixelBitsUsed=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0102U)) // Pixel high bit index
			{
			result->pixelBitsMSB=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0103U)) // Pixel representation
			{
			result->pixelSigned=dcmFile->read<Misc::UInt16>()!=0;
			}
		else if(de.is(0x7fe0U,0x0010U)) // Pixel data
			{
			if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
				{
				/* It's a fixed-size pixel data element: */
				result->imageOffset=dcmFile->getReadPos();
				result->imageDataSize=size_t(de.valueSize);
				de.skipValue(*dcmFile);
				}
			else
				{
				/* Start a variable-size pixel data sequence: */
				sequenceTop=new DicomSequence(sequenceTop,de.tag);
				}
			}
		else if(de.is(0xfffeU,0xe000U)) // Start of sequence item
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context sequence item");
			
			++sequenceTop->currentItemIndex;
			
			/* Check if the current item is a fixed-size item: */
			if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
				{
				sequenceTop->currentItemFixedSize=true;
				sequenceTop->currentItemEndOffset=dcmFile->getReadPos()+Offset(de.valueSize);
				}
			
			/* Process sequence items based on context: */
			if(sequenceTop->tag[0]==0x7fe0U&&sequenceTop->tag[1]==0x0010U) // Pixel data sequence
				{
				if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
					{
					result->imageOffset=dcmFile->getReadPos();
					result->imageDataSize=size_t(de.valueSize);
					de.skipValue(*dcmFile);
					}
				}
			}
		else if(de.is(0xfffeU,0xe00dU)) // End of sequence item
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context sequence item");
			}
		else if(de.is(0xfffeU,0xe0ddU)) // End of sequence
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context end-of-sequence element");
			
			/* End the current sequence: */
			DicomSequence* newSequenceTop=sequenceTop->succ;
			delete sequenceTop;
			sequenceTop=newSequenceTop;
			}
		else
			de.skipValue(*dcmFile);
		}
	
	/* Close all pending sequences: */
	while(sequenceTop!=0)
		{
		DicomSequence* nextSequenceTop=sequenceTop->succ;
		delete sequenceTop;
		sequenceTop=nextSequenceTop;
		}
	
	return result.releaseTarget();
	}

namespace {

typedef std::pair<std::string,DicomFile::ImageDescriptor*> DicomImageFile;

class DicomImageFileSorter
	{
	/* Methods: */
	public:
	bool operator()(const DicomImageFile& dif1,const DicomImageFile& dif2)
		{
		return dif1.second->stackIndex<dif2.second->stackIndex;
		}
	};

}

DicomFile::ImageStackDescriptor* DicomFile::readImageStackDescriptor(IO::DirectoryPtr directory)
	{
	/* Read all DICOM image files in the directory: */
	std::vector<DicomImageFile> images;
	while(directory->readNextEntry())
		{
		try
			{
			if(directory->getEntryType()==Misc::PATHTYPE_FILE)
				{
				/* Open the directory entry as a DICOM file: */
				DicomFile dcm(directory->getEntryName(),directory->openFile(directory->getEntryName()));

				/* Read the DICOM file's image descriptor: */
				ImageDescriptor* id=dcm.readImageDescriptor();
				if(id!=0)
					{
					/* Store the image name / image descriptor pair: */
					images.push_back(DicomImageFile(directory->getEntryName(),id));
					}
				}
			}
		catch(std::runtime_error err)
			{
			/* Ignore the offending file */
			std::cout<<"Ignoring file "<<directory->getEntryName()<<" due to exception "<<err.what()<<std::endl;
			}
		}
	
	/* Sort the image files by slice index: */
	DicomImageFileSorter difs;
	std::sort(images.begin(),images.end(),difs);
	
	/* Check if the image files are consistent: */
	std::vector<DicomImageFile>::iterator predIt=images.begin();
	int firstStackIndex=predIt->second->stackIndex;
	int stackImageSize[2];
	for(int i=0;i<2;++i)
		stackImageSize[i]=predIt->second->imageSize[i];
	float stackImagePos[3];
	for(int i=0;i<3;++i)
		stackImagePos[i]=predIt->second->imagePos[i];
	float stackSliceThickness=predIt->second->sliceThickness;
	float stackPixelSize[2];
	for(int i=0;i<2;++i)
		stackPixelSize[i]=predIt->second->pixelSize[i];
	bool stackConsistent=true;
	std::vector<DicomImageFile>::iterator it=predIt;
	for(++it;it!=images.end();predIt=it,++it)
		{
		if(it->second->stackIndex!=predIt->second->stackIndex+1
		   ||it->second->imageSize[0]!=stackImageSize[0]
		   ||it->second->imageSize[1]!=stackImageSize[1]
		   ||it->second->imagePos[0]!=stackImagePos[0]
		   ||it->second->imagePos[1]!=stackImagePos[1]
		   ||it->second->sliceThickness!=stackSliceThickness
		   ||it->second->pixelSize[0]!=stackPixelSize[0]
		   ||it->second->pixelSize[1]!=stackPixelSize[1])
			stackConsistent=false;
		}
	if(!stackConsistent)
		{
		std::cerr<<"DicomFile::readImageStackDescriptor: Image stack is inconsistent"<<std::endl;
		// return 0;
		}
	
	/* Build the image stack descriptor: */
	ImageStackDescriptor* result=new ImageStackDescriptor(images.size());
	for(int i=0;i<2;++i)
		result->imageSize[i]=stackImageSize[i];
	for(int i=0;i<3;++i)
		result->stackPosition[i]=stackImagePos[i];
	result->sliceThickness=stackSliceThickness;
	for(int i=0;i<2;++i)
		result->pixelSize[i]=stackPixelSize[i];
	for(std::vector<DicomImageFile>::iterator it=images.begin();it!=images.end();++it)
		{
		/* Construct the full slice file name: */
		std::string name=directory->getPath();
		name.push_back('/');
		name.append(it->first);
		
		/* Store the slice file name in the image stack descriptor: */
		int si=it->second->stackIndex-firstStackIndex;
		result->imageFileNames[si]=new char[name.length()+1];
		memcpy(result->imageFileNames[si],name.data(),name.length());
		result->imageFileNames[si][name.length()]='\0';
		
		/* Delete the image descriptor: */
		delete it->second;
		}
	
	return result;
	}

template <class DestPixelTypeParam>
void DicomFile::readImage(const DicomFile::ImageDescriptor& id,DestPixelTypeParam* imageBuffer,const ptrdiff_t imageBufferStrides[2])
	{
	/* Process depending on the image type: */
	if(imageMode==IMAGE_RAW)
		{
		/* Position the file to the image data: */
		dcmFile->setReadPosAbs(id.imageOffset);
		
		if(id.pixelBits<=8)
			{
			if(id.pixelSigned)
				{
				/* Raw pixel format is signed char: */
				PixelTypeConverter<signed char,DestPixelTypeParam> ptc(id.pixelBitsMSB);
				readRawImage(*dcmFile,id.imageSize,imageBuffer,imageBufferStrides,ptc);
				}
			else
				{
				/* Raw pixel format is unsigned char: */
				PixelTypeConverter<unsigned char,DestPixelTypeParam> ptc(id.pixelBitsMSB);
				readRawImage(*dcmFile,id.imageSize,imageBuffer,imageBufferStrides,ptc);
				}
			}
		else if(id.pixelBits<=16)
			{
			if(id.pixelSigned)
				{
				/* Raw pixel format is signed short: */
				PixelTypeConverter<signed short,DestPixelTypeParam> ptc(id.pixelBitsMSB);
				readRawImage(*dcmFile,id.imageSize,imageBuffer,imageBufferStrides,ptc);
				}
			else
				{
				/* Raw pixel format is unsigned short: */
				PixelTypeConverter<unsigned short,DestPixelTypeParam> ptc(id.pixelBitsMSB);
				readRawImage(*dcmFile,id.imageSize,imageBuffer,imageBufferStrides,ptc);
				}
			}
		}
	else if(imageMode==IMAGE_RLE)
		{
		Misc::throwStdErr("DicomFile::readImage: Runlength-encoded images currently not supported");
		}
	else if(imageMode==IMAGE_JPEG_LOSSY)
		{
		Misc::throwStdErr("DicomFile::readImage: lossy JPEG images currently not supported");
		}
	else if(imageMode==IMAGE_JPEG_LOSSLESS)
		{
		/* Read the compressed image into a buffer: */
		IO::FixedMemoryFile jpegBuffer(id.imageDataSize);
		jpegBuffer.ref();
		dcmFile->setReadPosAbs(id.imageOffset);
		dcmFile->readRaw(jpegBuffer.getMemory(),id.imageDataSize);
		
		/* Create a JPEG decompressor: */
		JPEGDecompressor dc(jpegBuffer);
		
		/* Read the first scan: */
		if(dc.readScanHeader())
			{
			/* Check the JPEG scan for consistency with the image descriptor: */
			if(id.imageSize[0]!=dc.getImageSize(0)||id.imageSize[1]!=dc.getImageSize(1)||dc.getNumComponents()!=1||id.pixelBitsMSB!=dc.getNumBits()-1)
				Misc::throwStdErr("DicomFile::readImage: JPEG image stream incompatible with image descriptor");
			
			/* Set up a writer for the result image: */
			MemoryImageWriter<DestPixelTypeParam> imgWriter(imageBuffer,id.imageSize,imageBufferStrides,id.pixelBitsMSB);
			
			/* Decompress the JPEG scan: */
			dc.readImage(imgWriter);
			}
		}
	}

DicomFile::Directory* DicomFile::readDirectory(void)
	{
	if(fileType!=FILE_DIRECTORY)
		Misc::throwStdErr("DicomFile::readDirectory: file is not a directory file");
	
	Directory* result=new Directory;
	
	/* Read all data elements: */
	DicomSequence* sequenceTop=0;
	Offset currentDirectoryItemOffset=0;
	Offset nextSiblingRecordOffset=0;
	Offset firstChildRecordOffset=0;
	Directory* currentDirectory=0;
	Series* currentSeries=0;
	Image* currentImage=0;
	typedef Misc::HashTable<Offset,std::pair<Directory*,int> > DirectoryLinkHasher;
	DirectoryLinkHasher directoryLinkHasher(101);
	DicomDataElement de;
	while(true)
		{
		/* Check if the next data element ends the current sequence: */
		Offset deOffset=dcmFile->getReadPos();
		if(sequenceTop!=0&&sequenceTop->fixedSize&&sequenceTop->endOffset==deOffset)
			{
			/* End the current sequence: */
			DicomSequence* newSequenceTop=sequenceTop->succ;
			delete sequenceTop;
			sequenceTop=newSequenceTop;
			}
		
		/* Read the next data element: */
		if(!de.read(*dcmFile,vrMode))
			break;
		
		/* Handle sequences: */
		if(de.vr==DicomDataElement::SQ)
			{
			/* Start a new sequence: */
			sequenceTop=new DicomSequence(sequenceTop,de.tag);
			
			/* Check if it is a fixed-size sequence: */
			if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
				{
				sequenceTop->fixedSize=true;
				sequenceTop->endOffset=dcmFile->getReadPos()+Offset(de.valueSize);
				}
			}
		
		/* Parse relevant information: */
		if(de.is(0x0004U,0x1200U)) // First directory entry offset
			{
			directoryLinkHasher[dcmFile->read<Misc::UInt32>()]=std::pair<Directory*,int>(result,0);
			}
		else if(de.is(0x0004U,0x1400U)) // Offset of next directory record
			{
			nextSiblingRecordOffset=dcmFile->read<Misc::UInt32>();
			}
		else if(de.is(0x0004U,0x1420U)) // Offset of lower-level directory record
			{
			firstChildRecordOffset=dcmFile->read<Misc::UInt32>();
			}
		else if(de.is(0x0004U,0x1430U)) // Directory record type
			{
			/* Create a new directory item based on the record type: */
			char* value=de.readValue(*dcmFile); // Note that values are space-padded to even length!
			if(strcmp(value,"PATIENT ")==0)
				{
				currentDirectory=new Directory;
				}
			else if(strcmp(value,"STUDY ")==0)
				{
				currentDirectory=new Directory;
				}
			else if(strcmp(value,"SERIES")==0)
				{
				currentSeries=new Series;
				currentDirectory=currentSeries;
				}
			else if(strcmp(value,"IMAGE ")==0)
				{
				currentImage=new Image;
				currentDirectory=currentImage;
				}
			else
				Misc::throwStdErr("DicomFile::readDictionary: unknown directory record type \"%s\"",value);
			delete[] value;
			
			/* Link the new entry to the directory tree: */
			DirectoryLinkHasher::Iterator dlIt=directoryLinkHasher.findEntry(currentDirectoryItemOffset);
			if(dlIt.isFinished())
				Misc::throwStdErr("DicomFile::readDirectory: Unlinked directory entry");
			const std::pair<Directory*,int>& dl=dlIt->getDest();
			if(dl.second==0)
				{
				if(dl.first->firstChild!=0)
					Misc::throwStdErr("DicomFile::readDirectory: Doubly-linked directory entry");
				dl.first->firstChild=currentDirectory;
				}
			else
				{
				if(dl.first->nextSibling!=0)
					Misc::throwStdErr("DicomFile::readDirectory: Doubly-linked directory entry");
				dl.first->nextSibling=currentDirectory;
				}
			
			/* Store the new entry's offsets in the hash table: */
			directoryLinkHasher[firstChildRecordOffset]=std::pair<Directory*,int>(currentDirectory,0);
			directoryLinkHasher[nextSiblingRecordOffset]=std::pair<Directory*,int>(currentDirectory,1);
			}
		else if(de.is(0x0004U,0x1500U)) // Referenced file ID
			{
			/* Read file name: */
			char* value=de.readValue(*dcmFile);
			
			/* Convert to UNIX-style file name: */
			for(char* cPtr=value;*cPtr!='\0';++cPtr)
				if(*cPtr=='\\')
					*cPtr='/';
			
			/* Store the file name: */
			currentImage->imageFileName=value;
			}
		else if(de.is(0x0018U,0x0050U)) // Slice thickness
			{
			char* value=de.readValue(*dcmFile);
			currentImage->sliceThickness=float(atof(value));
			delete[] value;
			}
		else if(de.is(0x0020U,0x0011U)) // Series number
			{
			char* value=de.readValue(*dcmFile);
			currentSeries->seriesNumber=atoi(value);
			delete[] value;
			}
		else if(de.is(0x0020U,0x0013U)) // Instance number (index in image stack)
			{
			char* value=de.readValue(*dcmFile);
			currentImage->sliceIndex=atoi(value);
			delete[] value;
			}
		else if(de.is(0x0020U,0x0032U)) // Image position
			{
			de.readFloatValues(*dcmFile,3,currentImage->imagePosition);
			}
		else if(de.is(0x0028U,0x0010U)) // Number of image rows
			{
			currentImage->imageSize[1]=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0011U)) // Number of image columns
			{
			currentImage->imageSize[0]=dcmFile->read<Misc::UInt16>();
			}
		else if(de.is(0x0028U,0x0030U)) // Pixel size
			{
			de.readFloatValues(*dcmFile,2,currentImage->pixelSize);
			}
		else if(de.is(0xfffeU,0xe000U)) // Start of sequence item
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context sequence item");
			
			++sequenceTop->currentItemIndex;
			
			/* Check if the current item is a fixed-size item: */
			if(de.valueSize>0U&&de.valueSize!=0xffffffffU)
				{
				sequenceTop->currentItemFixedSize=true;
				sequenceTop->currentItemEndOffset=dcmFile->getReadPos()+Offset(de.valueSize);
				}
			
			/* Process sequence items based on context: */
			if(sequenceTop->tag[0]==0x0004U&&sequenceTop->tag[1]==0x1220U) // Directory record sequence
				{
				/* Save the directory item's offset to link the new item into the tree later: */
				currentDirectoryItemOffset=deOffset;
				
				/* Reset the current record values: */
				nextSiblingRecordOffset=Offset(0);
				firstChildRecordOffset=Offset(0);
				currentDirectory=0;
				currentSeries=0;
				currentImage=0;
				}
			}
		else if(de.is(0xfffeU,0xe00dU)) // End of sequence item
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context sequence item");
			}
		else if(de.is(0xfffeU,0xe0ddU)) // End of sequence
			{
			if(sequenceTop==0)
				Misc::throwStdErr("DicomFile::readImageDescriptor: Out-of-context end-of-sequence element");
			
			/* End the current sequence: */
			DicomSequence* newSequenceTop=sequenceTop->succ;
			delete sequenceTop;
			sequenceTop=newSequenceTop;
			}
		else
			de.skipValue(*dcmFile);
		}
	
	/* Close all pending sequences: */
	while(sequenceTop!=0)
		{
		DicomSequence* nextSequenceTop=sequenceTop->succ;
		delete sequenceTop;
		sequenceTop=nextSequenceTop;
		}
	
	return result;
	}

/**************************************************
Force instantiations of standard readImage methods:
**************************************************/

template void DicomFile::readImage<signed char>(const ImageDescriptor& id,signed char* imageBuffer,const ptrdiff_t imageBufferStrides[2]);
template void DicomFile::readImage<unsigned char>(const ImageDescriptor& id,unsigned char* imageBuffer,const ptrdiff_t imageBufferStrides[2]);
template void DicomFile::readImage<signed short>(const ImageDescriptor& id,signed short* imageBuffer,const ptrdiff_t imageBufferStrides[2]);
template void DicomFile::readImage<unsigned short>(const ImageDescriptor& id,unsigned short* imageBuffer,const ptrdiff_t imageBufferStrides[2]);

}

}
