/***********************************************************************
DicomImageStack - Class to encapsulate operations on scalar-valued
cartesian data sets stored in stacks of DICOM medical interchange
images.
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

#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdexcept>
#include <vector>
#include <Plugins/FactoryManager.h>

#include <Concrete/DicomImageFile.h>

#include <Concrete/DicomImageStack.h>

namespace Visualization {

namespace Concrete {

/********************************
Methods of class DicomImageStack:
********************************/

DicomImageStack::DicomImageStack(void)
	:BaseModule("DicomImageStack")
	{
	}

Visualization::Abstract::DataSet* DicomImageStack::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Extract the image metadata from all DICOM images inside the given directory: */
	DIR* directory=opendir(args[0].c_str());
	if(directory==0)
		Misc::throwStdErr("DicomImageStack::load: Could not read directory \"%s\"",args[0].c_str());
	
	char dicomFileName[1024];
	strcpy(dicomFileName,args[0].c_str());
	char* dicomFileNameStart=dicomFileName+strlen(args[0].c_str());
	std::vector<DicomImageInformation> images;
	while(true)
		{
		/* Read the next directory entry: */
		struct dirent* dirEntry=readdir(directory);
		if(dirEntry==0)
			break;
		
		/* Try extracting DICOM image metadata from the directory entry: */
		strcpy(dicomFileNameStart,dirEntry->d_name);
		try
			{
			images.push_back(readDicomImageInformation(dicomFileName));
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
		Misc::throwStdErr("DicomImageStack::load: directory \"%s\" does not contain slices",args[0].c_str());
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
		Misc::throwStdErr("DicomImageStack::load: slice images in \"%s\" do not form a consistent image stack",args[0].c_str());
	
	/* Create the data set: */
	DS::Index numVertices(numSlices,stackImageSize[1],stackImageSize[0]);
	DS::Size cellSize(stackSliceThickness,stackPixelSize[1],stackPixelSize[0]);
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Read each slice: */
	ptrdiff_t increments[2];
	increments[0]=result->getDs().getVertices().getIncrement(2);
	increments[1]=result->getDs().getVertices().getIncrement(1);
	for(iIt=images.begin();iIt!=images.end();++iIt)
		{
		unsigned short* sliceBase=result->getDs().getVertices().getAddress(iIt->stackIndex-stackIndexMin,0,0);
		readDicomImage(*iIt,sliceBase,increments);
		}
	
	return result;
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
