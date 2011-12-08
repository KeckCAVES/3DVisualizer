/***********************************************************************
JPEGDecompressor - Class to decompress lossless JPEG images.
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

#include <Concrete/JPEGDecompressor.h>

#include <Misc/ThrowStdErr.h>

#include <Concrete/BitBuffer.h>
#include <Concrete/HuffmanTable.h>
#include <Concrete/JPEGImageWriter.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
****************/

inline int readNextMarker(IO::File& source)
	{
	enum State
		{
		START,FF_READ,MARKER_READ
		} state=START;
	
	int result;
	while(state!=MARKER_READ)
		{
		int c=source.getChar();
		
		switch(state)
			{
			case START:
				if(c==0xff) // FF (potentially) starting marker found
					state=FF_READ;
				break;
			
			case FF_READ:
				if(c==0x00) // Stuffed zeros don't count
					state=START;
				else if(c!=0xff) // Multiple FFs are legal
					{
					result=c;
					state=MARKER_READ;
					}
				break;
			
			default:
				/* This can't happen; just to make compiler happy: */
				;
			}
		}
	
	return result;
	}

inline int readShort(IO::File& source)
	{
	/* Read the integer in MSB-first order: */
	return (int(source.getChar())<<8)+int(source.getChar());
	}

inline void skipVariable(IO::File& source)
	{
	/* Skip the variable's value: */
	int length=readShort(source)-2;
	source.skip<char>(length);
	}

}

/*********************************
Methods of class JPEGDecompressor:
*********************************/

void JPEGDecompressor::processSoi(void)
	{
	/* Reset restart interval: */
	restartInterval=0;
	}

void JPEGDecompressor::processDht(void)
	{
	/* Read the Huffman table's length: */
	int length=readShort(source)-2;
	
	while(length>0)
		{
		/* Read the Huffman table index: */
		int index=source.getChar();
		if(index<0||index>=4)
			Misc::throwStdErr("JPEGDecompressor::processDht: Huffman table index out of range");
		
		/* Read the bits array: */
		int huffmanBits[17];
		huffmanBits[0]=0;
		int numHuffmanValues=0;
		for(int i=1;i<=16;++i)
			{
			huffmanBits[i]=source.getChar();
			numHuffmanValues+=huffmanBits[i];
			}
		if(numHuffmanValues>256)
			Misc::throwStdErr("JPEGDecompressor::processDht: too many values in Huffman table");
		
		/* Read the Huffman code value array: */
		unsigned char huffmanValues[256];
		source.read(huffmanValues,numHuffmanValues);
		
		/* Decrement the leftover variable length: */
		length-=1+16+numHuffmanValues;
		
		/* Store the Huffman table: */
		if(huffmanTables[index]!=0)
			delete huffmanTables[index];
		huffmanTables[index]=new HuffmanTable(huffmanBits,huffmanValues);
		}
	}

void JPEGDecompressor::processDri(void)
	{
	/* Check length of variable: */
	if(readShort(source)!=4)
		Misc::throwStdErr("JPEGDecompressor::processDri: DRI marker has wrong length");
	
	/* Read restart interval: */
	restartInterval=readShort(source);
	}

void JPEGDecompressor::processSof(int sofMarker)
	{
	int length=readShort(source)-2;
	
	/* Read SOF elements: */
	numBits=source.getChar();
	imageSize[1]=readShort(source);
	imageSize[0]=readShort(source);
	numComponents=source.getChar();
	
	/* Check JPEG stream for sanity: */
	if(imageSize[0]<=0||imageSize[1]<=0||numComponents<=0)
		Misc::throwStdErr("JPEGDecompressor::processSof: emtpy JPEG stream");
	if(numBits<2||numBits>16)
		Misc::throwStdErr("JPEGDecompressor::processSof: unsupported number of bits per pixel component");
	if(length!=numComponents*3+6)
		Misc::throwStdErr("JPEGDecompressor::processSof: wrong SOF marker length");
	
	/* Read component information: */
	components=new JPEGComponent[numComponents];
	for(int i=0;i<numComponents;++i)
		{
		components[i].index=i;
		
		/* Read the component's identity: */
		components[i].id=source.getChar();
		
		/* Read the downsampling factors: */
		int ds=source.getChar();
		components[i].samplingFactors[0]=(ds>>4)&0xf;
		components[i].samplingFactors[1]=ds&0xf;
		
		/* Initialize the Huffman table index: */
		components[i].huffmanTableIndex=-1;
		
		/* Skip the Tq value stored in the JPEG stream: */
		source.getChar();
		}
	}

void JPEGDecompressor::processSos(void)
	{
	int length=readShort(source)-2;
	
	/* Get the number of components in scan: */
	numScanComponents=source.getChar();
	if(numScanComponents<1||numScanComponents>4)
		Misc::throwStdErr("JPEGDecompressor::processSos: wrong number of components in scan");
	if(length!=numScanComponents*2+4)
		Misc::throwStdErr("JPEGDecompressor::processSof: wrong SOF marker length");
	
	/* Read component information: */
	for(int i=0;i<numScanComponents;++i)
		{
		/* Read the ID of the component and find its description in the table: */
		int componentId=source.getChar();
		int componentIndex;
		for(componentIndex=0;componentIndex<numComponents&&componentId!=components[componentIndex].id;++componentIndex)
			;
		if(componentIndex>=numComponents)
			Misc::throwStdErr("JPEGDecompressor::processSos: invalid component ID in scan");
		
		/* Store a pointer to the component: */
		scanComponents[i]=&components[componentIndex];
		
		/* Read the Huffman table index for this component: */
		scanComponents[i]->huffmanTableIndex=(int(source.getChar())>>4)&0xf;
		}
	
	/* Read the point transformation parameters: */
	ss=source.getChar();
	if(ss>7)
		Misc::throwStdErr("JPEGDecompressor::processSos: invalid predictor type");
	source.getChar();
	pt=int(source.getChar())&0xf;
	}

int JPEGDecompressor::processTables(void)
	{
	while(true)
		{
		/* Read the next marker from the input stream: */
		int marker=readNextMarker(source);
		
		/* Process it: */
		switch(marker)
			{
			case SOF0:
			case SOF1:
			case SOF2:
			case SOF3:
			case SOF5:
			case SOF6:
			case SOF7:
			case JPG:
			case SOF9:
			case SOF10:
			case SOF11:
			case SOF13:
			case SOF14:
			case SOF15:
			case SOI:
			case EOI:
			case SOS:
				return marker;
			
			case DHT:
				processDht();
				break;
			
			case DQT:
				Misc::throwStdErr("JPEGDecompressor::processTables: input stream is not a lossless JPEG stream");
				break;
			
			case DRI:
				processDri();
				break;
			
			case RST0:
			case RST1:
			case RST2:
			case RST3:
			case RST4:
			case RST5:
			case RST6:
			case RST7:
			case TEM:
				/* These are parameterless, so ignore them */
				break;
			
			default:
				skipVariable(source);
			}
		}
	
	/* This is never reached: */
	return 0;
	}

void JPEGDecompressor::processRestart(short nextMarkerNumber)
	{
	/* Scan for the next JPEG marker: */
	int c;
	do
		{
		/* Scan for the next 0xff: */
		while((c=source.getChar())!=0xff)
			;
		
		/* Skip duplicate 0xffs: */
		while((c=source.getChar())==0xff)
			;
		}
	while(c==0);
	
	/* Check for the proper marker number: */
	if(c!=RST0+nextMarkerNumber)
		Misc::throwStdErr("JPEGDecompressor::processRestart: wrong number in restart marker");
	}

JPEGDecompressor::JPEGDecompressor(IO::File& sSource)
	:source(sSource),
	 numComponents(0),
	 numBits(0),
	 components(0),
	 numScanComponents(0),
	 restartInterval(0),
	 ss(0),pt(0)
	{
	/* Clear the current components: */
	for(int i=0;i<4;++i)
		scanComponents[i]=0;
	
	/* Clear the Huffman tables: */
	for(int i=0;i<4;++i)
		huffmanTables[i]=0;
	
	/* Check for an SOI marker at the beginning of the stream: */
	int c1=source.getChar();
	int c2=source.getChar();
	if(c1!=0xff||c2!=SOI)
		Misc::throwStdErr("JPEGDecompressor::JPEGDecompressor: input stream is not a JPEG stream");
	
	/* Process the SOI marker just read: */
	processSoi();
	
	/* Process all markers in the file header: */
	int nextMarker=processTables();
	
	/* Process the next marker: */
	switch(nextMarker)
		{
		case SOF0:
		case SOF1:
		case SOF3:
			processSof(nextMarker);
			break;
		
		default:
			/* Just ignore the wrong SOF marker type for now... */
			;
		}
	}

JPEGDecompressor::~JPEGDecompressor(void)
	{
	/* Destroy the component table: */
	delete[] components;
	
	/* Destroy the Huffman tables: */
	for(int i=0;i<4;++i)
		delete huffmanTables[i];
	}

bool JPEGDecompressor::readScanHeader(void)
	{
	/* Process all markers in the scan header: */
	int nextMarker=processTables();
	
	/* Process the next marker: */
	if(nextMarker==SOS)
		processSos();
	
	return nextMarker==SOS;
	}

void JPEGDecompressor::readImage(JPEGImageWriter& imageWriter)
	{
	/* Check validity of downsampling factors for current scan: */
	for(int i=0;i<numScanComponents;++i)
		if(scanComponents[i]->samplingFactors[0]!=1||scanComponents[i]->samplingFactors[1]!=1)
			Misc::throwStdErr("JPEGDecompressor::initDecoder: downsampling not supported");
	
	/* Check that all required Huffman tables are present: */
	for(int i=0;i<numScanComponents;++i)
		if(huffmanTables[scanComponents[i]->huffmanTableIndex]==0)
			Misc::throwStdErr("JPEGDecompressor::initDecoder: undefined Huffman table used in scan");
	
	/* Initialize the image writer: */
	imageWriter.setImageParameters(imageSize,numScanComponents,numBits);
	
	/* Create buffers for two rows of pixels for predictor calculation: */
	short* imageRows[2];
	for(int i=0;i<2;++i)
		imageRows[i]=new short[numScanComponents*imageSize[0]];
	
	/* Initialize the restart interval counters: */
	int restartInRows=restartInterval/imageSize[0];
	int restartRowsToGo=restartInRows;
	short nextRestartNumber=0;
	
	/* Create a bit buffer: */
	BitBuffer bb(source);
	
	/* Create a shortcut to Huffman tables for this scan: */
	HuffmanTable* scanTables[4];
	for(int comp=0;comp<numScanComponents;++comp)
		scanTables[comp]=huffmanTables[scanComponents[comp]->huffmanTableIndex];
	
	/* Decode all image rows: */
	for(int row=0;row<imageSize[1];++row)
		{
		short* cirPtr=imageRows[0];
		short* pirPtr=imageRows[1];
		
		/* Check if the current restart interval is over: */
		if(restartRowsToGo==0)
			{
			if(restartInRows!=0)
				{
				/* Process the restart marker: */
				processRestart(nextRestartNumber);
				
				/* Reset the restart interval: */
				restartRowsToGo=restartInRows-1;
				nextRestartNumber=(nextRestartNumber+1)%7;
				
				/* Clear the bit buffer: */
				bb.clear();
				}
			else
				restartRowsToGo=imageSize[1];
			
			/* Decode the first column: */
			for(int comp=0;comp<numScanComponents;++comp,++pirPtr,++cirPtr)
				{
				/* Decode the difference: */
				int s=scanTables[comp]->decode(bb);
				int diff=s!=0?bb.getSignedBits(s):0;
				
				/* Set the pixel value: */
				*cirPtr=short((1<<(numBits-pt-1))+diff);
				}
			
			/* Predict the rest of the columns: */
			for(int col=1;col<imageSize[0];++col)
				{
				for(int comp=0;comp<numScanComponents;++comp,++pirPtr,++cirPtr)
					{
					/* Decode the difference: */
					int s=scanTables[comp]->decode(bb);
					int diff=s!=0?bb.getSignedBits(s):0;
					
					/* Set the pixel value: */
					*cirPtr=short(int(cirPtr[-numScanComponents])+diff);
					}
				}
			}
		else
			{
			/* Predict the first column from upper neighbors: */
			for(int comp=0;comp<numScanComponents;++comp,++pirPtr,++cirPtr)
				{
				/* Decode the difference: */
				int s=scanTables[comp]->decode(bb);
				int diff=s!=0?bb.getSignedBits(s):0;
				
				/* Set the pixel value: */
				*cirPtr=short(int(*pirPtr)+diff);
				}
			
			/* Predict the rest of the columns based on PSV: */
			for(int col=1;col<imageSize[0];++col)
				{
				for(int comp=0;comp<numScanComponents;++comp,++pirPtr,++cirPtr)
					{
					/* Decode the difference: */
					int s=scanTables[comp]->decode(bb);
					int diff=s!=0?bb.getSignedBits(s):0;
						
					/* Calculate the predictor value: */
					int predicted=0;
					switch(ss)
						{
						case 0:
							predicted=0;
							break;
						
						case 1:
							predicted=int(cirPtr[-numScanComponents]);
							break;

						case 2:
							predicted=int(pirPtr[0]);
							break;
						
						case 3:
							predicted=int(pirPtr[-numScanComponents]);
							break;
						
						case 4:
							predicted=int(cirPtr[-numScanComponents])+(int(pirPtr[0])-int(pirPtr[-numScanComponents]));
							break;
						
						case 5:
							predicted=int(cirPtr[-numScanComponents])+((int(pirPtr[0])-int(pirPtr[-numScanComponents]))>>1);
							break;
						
						case 6:
							predicted=int(pirPtr[0])+((int(cirPtr[-numScanComponents])-int(pirPtr[-numScanComponents]))>>1);
							break;

						case 7:
							predicted=(int(cirPtr[-numScanComponents])+int(pirPtr[0]))>>1;
							break;
						}
					*cirPtr=short(predicted+diff);
					}
				}
			
			--restartRowsToGo;
			}
		
		/* Save the current image row: */
		imageWriter.writeImageRow(row,imageRows[0]);
		short* tempRow=imageRows[0];
		imageRows[0]=imageRows[1];
		imageRows[1]=tempRow;
		}
	
	/* Clean up: */
	for(int i=0;i<2;++i)
		delete[] imageRows[i];
	}

}

}
