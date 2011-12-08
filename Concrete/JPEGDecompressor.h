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

#ifndef VISUALIZATION_CONCRETE_JPEGDECOMPRESSOR_INCLUDED
#define VISUALIZATION_CONCRETE_JPEGDECOMPRESSOR_INCLUDED

#include <IO/File.h>

/* Forward declarations: */
namespace Visualization {
namespace Concrete {
class HuffmanTable;
class JPEGImageWriter;
}
}

namespace Visualization {

namespace Concrete {

class JPEGDecompressor
	{
	/* Embedded classes: */
	private:
	enum JPEGMarker // Enumerated type for JPEG stream markers
		{
		TEM=0x01,
		SOF0=0xc0,SOF1=0xc1,SOF2=0xc2,SOF3=0xc3,
		DHT=0xc4,
		SOF5=0xc5,SOF6=0xc6,SOF7=0xc7,
		JPG=0xc8,
		SOF9=0xc9,SOF10=0xca,SOF11=0xcb,
		DAC=0xcc,
		SOF13=0xcd,SOF14=0xce,SOF15=0xcf,
		RST0=0xd0,RST1=0xd1,RST2=0xd2,RST3=0xd3,RST4=0xd4,RST5=0xd5,RST6=0xd6,RST7=0xd7,
		SOI=0xd8,EOI=0xd9,SOS=0xda,DQT=0xdb,DNL=0xdc,DRI=0xdd,DHP=0xde,EXP=0xdf,
		APP0=0xe0,APP15=0xef,
		JPG0=0xf0,JPG13=0xfd,
		COM=0xfe,
		ERROR=0x100
		};
	
	struct JPEGComponent // Description of a component stored in a JPEG stream
		{
		/* Elements: */
		public:
		int index; // Index of the component in stream tables
		int id; // Identifier for the component
		int samplingFactors[2]; // Downsampling factors (not normally used in lossless JPEG)
		int huffmanTableIndex; // Index of the Huffman table used to encode this component
		};
	
	/* Elements: */
	IO::File& source; // Source for compressed JPEG data
	int imageSize[2]; // Width and height of uncompressed image
	int numComponents; // Number of components per pixel
	int numBits; // Number of bits per pixel component
	JPEGComponent* components; // Description of the pixel components stored in the JPEG stream
	int numScanComponents; // Number of components in current scan
	JPEGComponent* scanComponents[4]; // Up to four components present in current scan
	HuffmanTable* huffmanTables[4]; // Pointer to up to four Huffman tables
	int restartInterval; // Number of MCUs per restart interval, 0 = no restart
	int ss,pt; // Point transformation parameters
	
	/* Private methods: */
	void processSoi(void); // Processes an SOI marker
	void processDht(void); // Processes an DHT marker
	void processDri(void); // Processes an DRI marker
	void processSof(int sofMarker); // Processes an SOF marker
	void processSos(void); // Processes an SOS marker
	int processTables(void); // Processes the tables section of a JPEG stream; returns marker that ended tables
	void processRestart(short nextMarkerNumber); // Processes a restart marker inside a stream
	
	/* Constructors and destructors: */
	public:
	JPEGDecompressor(IO::File& sSource); // Creates a decompressor for the given compressed JPEG stream by reading the stream's header
	~JPEGDecompressor(void); // Destroys the decompressor after use
	
	/* Methods: */
	bool readScanHeader(void); // Prepares for decompressing an image scan by reading the scan header; returns true if an image scan follows
	const int* getImageSize(void) const // Returns the image size as an array
		{
		return imageSize;
		};
	int getImageSize(int dimension) const // Returns one component of the image size
		{
		return imageSize[dimension];
		};
	int getNumComponents(void) const // Returns the number of components in the image
		{
		return numScanComponents;
		};
	int getNumBits(void) const // Returns the number of bits per component in the image
		{
		return numBits;
		};
	void readImage(JPEGImageWriter& imageWriter); // Decompresses the image contained in the current scan and writes it to the given receiver object
	};

}

}

#endif
