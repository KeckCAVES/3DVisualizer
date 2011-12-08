/***********************************************************************
BitBuffer - Data structure to extract arbitrary-length integers from a
stream of bytes.
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

#ifndef VISUALIZATION_CONCRETE_BITBUFFER_INCLUDED
#define VISUALIZATION_CONCRETE_BITBUFFER_INCLUDED

#include <Misc/ThrowStdErr.h>
#include <IO/File.h>

namespace Visualization {

namespace Concrete {

class BitBuffer
	{
	/* Elements: */
	private:
	typedef unsigned long BufferType; // This should be the longest integer that can be processed quickly by the CPU
	static const int bufferSize=sizeof(BufferType)*8; // Number of bits in the buffer
	static const int fillSize=bufferSize-7; // Number of bits to grab from the input stream when a buffer underrun occurs
	
	/* Elements: */
	IO::File& source; // Source for compressed JPEG data
	BufferType bits; // Buffer of bits available for extraction
	int numBits; // Number of bits still left in buffer
	
	/* Private methods: */
	void fillBuffer(void) // Reads more bits from the input stream
		{
		/* Read bytes from the input stream until the fill size is reached: */
		while(numBits<fillSize)
			{
			/* Read a byte from the input stream: */
			int c=source.getChar();
			
			/* Check for byte stuffing: */
			if(c==0xff)
				{
				/* Check if this is a marker rather than a stuffed 0xff byte: */
				int d;
				if((d=source.getChar())!=0)
					{
					/* Put the marker back into the input stream for later processing: */
					source.ungetChar(d);
					source.ungetChar(c);
					
					/* Break out of the loop (caller will check if there are enough bits to proceed): */
					break;
					}
				}
			
			/* Add the read byte to the buffer: */
			bits=(bits<<8)|c;
			numBits+=8;
			}
		};
	
	/* Constructors and destructors: */
	public:
	BitBuffer(IO::File& sSource) // Creates a bit buffer for the given JPEG stream
		:source(sSource),
		 bits(0),numBits(0)
		{
		};
	
	/* Methods: */
	void clear(void) // Clear bit buffer
		{
		numBits=0;
		};
	int peekBits(int numGetBits) // Returns the requested number of bits without removing them from the buffer
		{
		/* Check if there are enough bits in the buffer: */
		if(numBits<numGetBits)
			{
			/* Fill up the buffer: */
			fillBuffer();
			
			/* Check for error condition: */
			if(numBits<numGetBits)
				{
				/* Return the smaller number of bits and hope it still makes sense (this happens at the end of a scan): */
				return int((bits<<(numGetBits-numBits))&((0x1U<<numGetBits)-1));
				}
			}
		
		/* Extract the requested number of bits: */
		return int((bits>>(numBits-numGetBits))&((0x1U<<numGetBits)-1));
		};
	int getBits(int numGetBits) // Returns the requested number of bits and removes them from the buffer
		{
		/* Check if there are enough bits in the buffer: */
		if(numBits<numGetBits)
			{
			/* Fill up the buffer: */
			fillBuffer();
			
			/* Check for error condition: */
			if(numBits<numGetBits)
				Misc::throwStdErr("BitBuffer::getBits: Corrupted JPEG data stream");
			}
		
		/* Extract the requested number of bits: */
		numBits-=numGetBits;
		return int((bits>>numBits)&((0x1U<<numGetBits)-1));
		};
	int getSignedBits(int numGetBits) // Returns the requested number of bits as a signed integer and removes them from the buffer
		{
		/* Check if there are enough bits in the buffer: */
		if(numBits<numGetBits)
			{
			/* Fill up the buffer: */
			fillBuffer();
			
			/* Check for error condition: */
			if(numBits<numGetBits)
				Misc::throwStdErr("BitBuffer::getSignedBits: Corrupted JPEG data stream");
			}
		
		/* Extract the requested number of bits: */
		numBits-=numGetBits;
		int result=int((bits>>numBits)&((0x1U<<numGetBits)-1));
		if(result<(0x1<<(numGetBits-1)))
			result+=((-1)<<numGetBits)+1;
		return result;
		};
	int getBit(void) // Returns a single bit and removes it from the buffer
		{
		/* Check if there are enough bits in the buffer: */
		if(numBits==0)
			{
			/* Fill up the buffer: */
			fillBuffer();
			
			/* Check for error condition: */
			if(numBits==0)
				Misc::throwStdErr("BitBuffer::getBit: Corrupted JPEG data stream");
			}
		
		/* Extract the requested number of bits: */
		--numBits;
		return int((bits>>numBits)&0x1U);
		};
	void flushBits(int numFlushBits) // Removes the given number of bits from the buffer
		{
		numBits-=numFlushBits;
		};
	};

}

}

#endif
