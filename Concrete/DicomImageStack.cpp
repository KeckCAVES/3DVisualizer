/***********************************************************************
DicomImageStack - Class to encapsulate operations on scalar-valued
cartesian data sets stored in stacks of DICOM medical interchange
images.
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

#include <Concrete/DicomImageStack.h>

#include <string.h>
#include <dirent.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <Misc/SelfDestructPointer.h>
#include <Misc/FileTests.h>
#include <Plugins/FactoryManager.h>
#include <Cluster/OpenFile.h>

#include <Concrete/DicomFile.h>

namespace Visualization {

namespace Concrete {

/********************************
Methods of class DicomImageStack:
********************************/

DicomImageStack::DicomImageStack(void)
	:BaseModule("DicomImageStack")
	{
	}

Visualization::Abstract::DataSet* DicomImageStack::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Parse the command line: */
	std::string fileName;
	int seriesNumber=-1;
	bool flip=false;
	for(std::vector<std::string>::const_iterator aIt=args.begin();aIt!=args.end();++aIt)
		{
		if((*aIt)[0]=='-')
			{
			if(strcasecmp(aIt->c_str()+1,"series")==0)
				{
				++aIt;
				seriesNumber=atoi(aIt->c_str());
				}
			else if(strcasecmp(aIt->c_str()+1,"flip")==0)
				flip=true;
			}
		else if(fileName.empty())
			fileName=getFullPath(*aIt);
		}
	if(fileName.empty())
		Misc::throwStdErr("DicomImageStack::load: No DICOM file name provided");
	
	/* Create a stack descriptor for the given stack of DICOM images: */
	Misc::SelfDestructPointer<DicomFile::ImageStackDescriptor> isd;
	
	/* Check if the given DICOM file name is a directory containing DICOM image files, or a DICOM directory file: */
	if(Misc::isPathDirectory(fileName.c_str()))
		{
		/* Open the directory: */
		IO::DirectoryPtr directory=Cluster::openDirectory(pipe!=0?pipe->getMultiplexer():0,fileName.c_str());
		
		/* Create a stack descriptor from all DICOM images in the given directory: */
		isd.setTarget(DicomFile::readImageStackDescriptor(directory));
		if(!isd.isValid())
			Misc::throwStdErr("DicomImageStack::load: Directory %s does not contain a valid image series",fileName.c_str());
		}
	else
		{
		/* Open the DICOM directory file: */
		DicomFile dcmDirectory(fileName.c_str(),Cluster::openFile(pipe!=0?pipe->getMultiplexer():0,fileName.c_str()));
		
		/* Read the image stack descriptor for the selected series: */
		Misc::SelfDestructPointer<DicomFile::Directory> directory(dcmDirectory.readDirectory());
		isd.setTarget(directory->getImageStackDescriptor(seriesNumber));
		if(!isd.isValid())
			{
			if(seriesNumber==-1)
				Misc::throwStdErr("DicomImageStack::load: Directory file %s does not contain a valid image series",fileName.c_str());
			else
				Misc::throwStdErr("DicomImageStack::load: Directory file %s does not contain a valid image series %d",fileName.c_str(),seriesNumber);
			}
		}
	
	/* Create the data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	
	DS::Index numVertices(isd->numImages,isd->imageSize[1],isd->imageSize[0]);
	DS::Size cellSize(isd->sliceThickness,isd->pixelSize[1],isd->pixelSize[0]);
	result->getDs().setData(numVertices,cellSize);
	
	/* Read each slice: */
	ptrdiff_t increments[2];
	increments[0]=result->getDs().getVertices().getIncrement(2);
	increments[1]=result->getDs().getVertices().getIncrement(1);
	for(int i=0;i<isd->numImages;++i)
		{
		/* Open the slice DICOM file: */
		DicomFile dcm(isd->imageFileNames[i],Cluster::openFile(pipe!=0?pipe->getMultiplexer():0,isd->imageFileNames[i]));
		
		/* Read the slice image descriptor: */
		Misc::SelfDestructPointer<DicomFile::ImageDescriptor> id(dcm.readImageDescriptor());
		
		/* Read the slice image: */
		Value* sliceBase=result->getDs().getVertices().getAddress(flip?isd->numImages-i-1:i,0,0);
		dcm.readImage(*id,sliceBase,increments);
		}
	
	return result.releaseTarget();
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::DicomImageStack* module=new Visualization::Concrete::DicomImageStack();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
