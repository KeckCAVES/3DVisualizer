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

#ifndef VISUALIZATION_CONCRETE_HUFFMANTABLE_INCLUDED
#define VISUALIZATION_CONCRETE_HUFFMANTABLE_INCLUDED

#include <Misc/ThrowStdErr.h>

#include <Concrete/BitBuffer.h>

namespace Visualization {

namespace Concrete {

class HuffmanTable
	{
	/* Elements: */
	private:
	int bits[17];
	unsigned char values[256];
	bool tableSent; // Used during compression; set to true when table has been emitted to file
	unsigned short ehufco[256];
	char ehufsi[256];
	int mincode[17];
	int maxcode[18];
	int valPtr[17];
	int numBits[256];
	int value[256];
	
	/* Constructors and destructors: */
	public:
	HuffmanTable(int sBits[17],unsigned char sValues[256]); // Creates a Huffman table from the given arrays
	~HuffmanTable(void);
	
	int decode(BitBuffer& bb) const // Decodes a bit sequence
		{
		/* Peek at the next eight bits in the buffer to determine whether the next code is a short code: */
		int code=bb.peekBits(8);
		if(numBits[code]!=0)
			{
			/* Remove the short code from the bit buffer: */
			bb.flushBits(numBits[code]);
			
			/* Return the decoded value: */
			return value[code];
			}
		else
			{
			/* Keep adding more bits to the code until it is valid: */
			int codeBits=8;
			bb.flushBits(8);
			while(code>maxcode[codeBits])
				{
				code=(code<<1)|bb.getBit();
				++codeBits;
				}
			
			/* Check for error condition: */
			if(codeBits>16)
				Misc::throwStdErr("HuffmanTable::decode: Corrupted JPEG stream");
			
			/* Return the decoded value: */
			return values[valPtr[codeBits]+(code-mincode[codeBits])];
			}
		}
	};

}

}

#endif
