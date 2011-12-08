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

#ifndef VISUALIZATION_CONCRETE_DICOMFILE_INCLUDED
#define VISUALIZATION_CONCRETE_DICOMFILE_INCLUDED

#include <IO/SeekableFile.h>
#include <IO/Directory.h>

namespace Visualization {

namespace Concrete {

class DicomFile
	{
	/* Embedded classes: */
	public:
	enum VrMode // Storage modes for value representations
		{
		VR_IMPLICIT,VR_EXPLICIT
		};
	
	enum FileType // File type
		{
		FILE_UNKNOWN,FILE_DIRECTORY,FILE_IMAGE
		};
	
	enum ImageType // Image source type
		{
		IMAGETYPE_UNKNOWN,
		IMAGETYPE_CRI, // Computed radiography image
		IMAGETYPE_CTI, // Computed tomography image
		IMAGETYPE_MRI // Magnetic resonance image
		};
	
	enum ImageMode // Storage modes for image data
		{
		IMAGE_RAW,IMAGE_RLE,IMAGE_JPEG_LOSSY,IMAGE_JPEG_LOSSLESS
		};
	
	typedef IO::SeekableFile::Offset Offset; // Type for file positions
	
	struct ImageDescriptor // Structure describing the contents of a DICOM image file
		{
		/* Elements: */
		public:
		int stackIndex; // Index of the slice in an image stack
		int imageSize[2]; // Image size (width, height)
		float imagePos[3]; // Origin of the image in patient coordinate system
		float sliceThickness; // Thickness of slice in patient coordinate system
		float pixelSize[2]; // Pixel size in patient coordinate system
		int pixelSamples; // Number of samples per pixel (1 for grayscale)
		bool pixelSigned; // Flag whether pixels are signed
		int pixelBits; // Number of bits allocated for each pixel
		int pixelBitsUsed; // Number of bits used per pixel
		int pixelBitsMSB; // Index of pixel high bit in pixel cell
		Offset imageOffset; // Offset of start of raw image data in DICOM file
		size_t imageDataSize; // Size of raw image data
		
		/* Constructors and destructors: */
		public:
		ImageDescriptor(void);
		};
	
	struct ImageStackDescriptor // Structure describing a stack of DICOM images
		{
		/* Elements: */
		public:
		int numImages; // Number of images in the stack
		int imageSize[2]; // Image size (width, height)
		float stackPosition[3]; // Coordinates of origin of image stack in patient coordinate system
		float sliceThickness; // Thickness of slice in patient coordinate system
		float pixelSize[2]; // Pixel size in patient coordinate system
		char** imageFileNames; // Array of image file names in stack order
		
		/* Constructors and destructors: */
		ImageStackDescriptor(int sNumImages);
		~ImageStackDescriptor(void);
		};
	
	class Series;
	
	class Directory // Class representing a DICOM directory entry
		{
		friend class DicomFile;
		
		/* Elements: */
		private:
		Directory* firstChild; // Pointer to first child directory entry
		Directory* nextSibling; // Pointer to next sibling directory entry
		
		/* Protected methods: */
		protected:
		virtual void printDirectory(int indent) const; // Recursively prints directory structure
		virtual const Series* findSeries(int findSeriesNumber) const; // Returns pointer to a series entry of the given number
		
		/* Constructors and destructors: */
		public:
		Directory(void); // Creates unconnected directory entry
		virtual ~Directory(void); // Destroys directory
		
		/* Methods: */
		Directory* getFirstChild(void) const
			{
			return firstChild;
			};
		Directory* getNextSibling(void) const
			{
			return nextSibling;
			};
		void printDirectory(void) const; // Prints the directory structure
		virtual void printSeries(void) const; // Prints all image series contained in the directory
		ImageStackDescriptor* getImageStackDescriptor(int seriesNumber) const; // Returns an image stack descriptor for the given series (or 0 if no series/series inconsistent)
		};
	
	class Series:public Directory // Class representing series directory entries
		{
		friend class DicomFile;
		
		/* Elements: */
		private:
		int seriesNumber; // Number of the series in the DICOM directory
		
		/* Protected methods: */
		protected:
		virtual void printDirectory(int indent) const;
		virtual const Series* findSeries(int findSeriesNumber) const;
		
		/* Constructors and destructors: */
		public:
		Series(void); // Creates uninitialized series
		
		/* Methods: */
		virtual void printSeries(void) const;
		};
	
	class Image:public Directory // Class representing image directory entries
		{
		friend class DicomFile;
		friend class Directory;
		
		/* Elements: */
		private:
		const char* imageFileName; // Name of the image's DICOM file
		int sliceIndex; // Index of the image in the containing series' image stack
		int imageSize[2]; // Size of the image in pixels
		float imagePosition[3]; // Position of the slice in patient coordinates
		float sliceThickness; // Thickness of the slice in patient coordinates
		float pixelSize[2]; // Size of the slice's pixels in patient coordinates
		
		/* Protected methods: */
		protected:
		virtual void printDirectory(int indent) const;
		
		/* Constructors and destructors: */
		public:
		Image(void); // Creates uninitialized image
		virtual ~Image(void);
		
		/* Methods: */
		int getSliceIndex(void) const // Returns the image's slice index
			{
			return sliceIndex;
			};
		int getImageSize(int dimension) const // Returns the image's size in one dimension
			{
			return imageSize[dimension];
			};
		float getSliceThickness(void) const // Returns the image's slice thickness
			{
			return sliceThickness;
			};
		float getPixelSize(int dimension) const // Returns the image's pixel size in one dimension
			{
			return pixelSize[dimension];
			};
		};
	
	/* Elements: */
	private:
	IO::SeekableFilePtr dcmFile; // The underlying file object
	VrMode vrMode; // The file's value representation mode
	FileType fileType; // The file's media storage SOP class
	ImageType imageType; // Type of image stored in the file
	ImageMode imageMode; // The file's image storage mode
	
	/* Constructors and destructors: */
	public:
	DicomFile(const char* dcmFileName,IO::SeekableFilePtr sDcmFile); // Reads the DICOM file of the given name through the provided seekable file abstraction
	~DicomFile(void); // Closes the DICOM file
	
	/* Methods: */
	bool isDirectoryFile(void) const // Returns true if the DICOM file is a directory file
		{
		return fileType==FILE_DIRECTORY;
		};
	bool isImageFile(void) const // Returns true if the DICOM file is an image file
		{
		return fileType==FILE_IMAGE;
		};
	ImageType getImageType(void) const // Returns the file's image type
		{
		return imageType;
		}
	ImageMode getImageMode(void) const // Returns the file's image storage mode
		{
		return imageMode;
		}
	ImageDescriptor* readImageDescriptor(void); // Returns image descriptor for a DICOM image file
	static ImageStackDescriptor* readImageStackDescriptor(IO::DirectoryPtr directory); // Assembles image stack descriptor for all DICOM image files in the given directory, or 0 if the images are inconsistent
	template <class DestPixelTypeParam>
	void readImage(const ImageDescriptor& id,DestPixelTypeParam* imageBuffer,const ptrdiff_t imageBufferStrides[2]); // Reads the image described in an image descriptor into a 2D array
	Directory* readDirectory(void); // Returns the directory structure of a DICOM directory file
	};

}

}

#endif
