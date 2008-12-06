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

#ifndef VISUALIZATION_CONCRETE_DICOMIMAGEFILEREADER_INCLUDED
#define VISUALIZATION_CONCRETE_DICOMIMAGEFILEREADER_INCLUDED

#include <Misc/Array.h>

namespace Visualization {

namespace Concrete {

struct DicomImageInformation // Structure to represent image metadata from a DICOM file
	{
	/* Elements: */
	public:
	char* fileName; // Name of the DICOM file this image information belongs to
	int stackIndex; // Index of the slice in the stack of images
	float imagePos[3]; // Origin of the slice in the patient coordinate system
	int imageSize[2]; // Image size in pixels (width, height)
	float pixelSize[2]; // Pixel size in patient coordinate system units
	float sliceThickness; // Thickness of the slice in patient coordinate system units
	int pixelSamples; // Number of samples per pixel (1 for grayscale)
	bool pixelSigned; // Flag whether pixels are signed
	int pixelBits; // Number of bits allocated for each pixel
	int pixelBitsUsed; // Number of bits used per pixel
	int pixelBitsMSB; // Index of pixel high bit in pixel cell
	long imageOffset; // Offset of start of raw image data in DICOM file
	
	/* Constructors and destructors: */
	DicomImageInformation(void); // Creates an invalid ("empty") metadata structure
	DicomImageInformation(const char* sFileName); // Creates metadata structure for given DICOM file
	DicomImageInformation(const DicomImageInformation& source); // Copy constructor
	DicomImageInformation& operator=(const DicomImageInformation& source); // Assignment operator
	~DicomImageInformation(void);
	
	/* Methods: */
	bool isValid(void) const // Returns true if the image information describes a valid image
		{
		return imageSize[0]>0&&imageSize[1]>0&&imageOffset>0;
		}
	};

struct DicomImageStackInformation // Structure to represent a DICOM image stack
	{
	/* Embedded classes: */
	public:
	typedef Misc::Array<unsigned short,3> Array;
	typedef Array::Index Index;
	
	/* Elements: */
	public:
	Array array; // 3D array of voxel values
	float cellSize[3]; // Size of a cell in patient coordinate system units
	
	/* Constructors and destructors: */
	DicomImageStackInformation(const Index& arraySize);
	};

DicomImageInformation readDicomImageInformation(const char* dicomFileName); // Extracts image metadata from a DICOM file
void readDicomImage(const DicomImageInformation& imageInformation,unsigned short* sliceBase,const ptrdiff_t increments[2]); // Reads image data from a DICOM file into a 3D array of unsigned short values
DicomImageStackInformation* readDicomImageStack(const char* dicomSliceDirectory); // Reads a stack of DICOM image slices into a 3D array

}

}

#endif
