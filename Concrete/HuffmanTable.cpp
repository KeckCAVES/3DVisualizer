/***********************************************************************
HuffmanTable - Class representing a table for Huffman compression/
decompression.
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

#include <Concrete/HuffmanTable.h>

namespace Visualization {

namespace Concrete {

/*****************************
Methods of class HuffmanTable:
*****************************/

HuffmanTable::HuffmanTable(int sBits[17],unsigned char sValues[256])
	:tableSent(false)
	{
	/* Copy the given arrays: */
	for(int i=0;i<17;++i)
		bits[i]=sBits[i];
	for(int i=0;i<256;++i)
		values[i]=sValues[i];
	
	/***************************************
	Initialize the derived table structures:
	***************************************/
	
	/* Create table of Huffman code lengths: */
	int huffmanSizes[257];
	int p=0;
	for(int l=1;l<=16;++l)
		{
		for(int i=1;i<=bits[l];++i,++p)
			huffmanSizes[p]=l;
		}
	huffmanSizes[p]=0;
	int lastP=p;
	
	/* Generate the Huffman codes in code-length order: */
	unsigned short huffmanCodes[257];
	unsigned short code=0;
	int si=huffmanSizes[0];
	for(p=0;huffmanSizes[p]>0;code<<=1,++si)
		for(;huffmanSizes[p]==si;++p,++code)
			huffmanCodes[p]=code;
	
	/* Generate encoding tables: */
	for(p=0;p<256;++p)
		ehufsi[p]=0;
	for(p=0;p<lastP;++p)
		{
		ehufco[values[p]]=huffmanCodes[p];
		ehufsi[values[p]]=huffmanSizes[p];
		}
	
	/* Generate decoding tables: */
	p=0;
	for(int l=1;l<=16;++l)
		{
		if(bits[l]!=0)
			{
			valPtr[l]=p;
			mincode[l]=int(huffmanCodes[p]);
			p+=bits[l];
			maxcode[l]=int(huffmanCodes[p-1]);
			}
		else
			maxcode[l]=-1;
		}
	maxcode[17]=0xfffff;
	
	/* Build look-up table from numBits to value to quickly identify short Huffman codes: */
	for(p=0;p<256;++p)
		numBits[p]=0;
	for(p=0;p<lastP;++p)
		{
		int size=huffmanSizes[p];
		if(size<=8)
			{
			/* Calculate the range of bytes that start with the short Huffman code: */
			int ll=int(huffmanCodes[p])<<(8-size);
			int ul=ll|((0x1<<(8-size))-1);
			for(int i=ll;i<=ul;++i)
				{
				numBits[i]=size;
				value[i]=values[p];
				}
			}
		}
	}

HuffmanTable::~HuffmanTable(void)
	{
	/* Nothing to do... */
	}

}

}
