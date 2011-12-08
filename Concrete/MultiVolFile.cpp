/***********************************************************************
MultiVolFile - Class to represent multivariate scalar-valued Cartesian
data sets stored as multiple matching volume files.
Copyright (c) 2010 Oliver Kreylos

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

#include <Concrete/MultiVolFile.h>

#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Geometry/Point.h>

namespace Visualization {

namespace Concrete {

namespace {

/**************
Helper methods:
**************/

template <class ValueParam>
inline
void readVolFile(Misc::File& volFile,DS& dataSet,int sliceIndex)
	{
	/* Get a pointer to the slice: */
	Value* slicePtr=dataSet.getSliceArray(sliceIndex);
	
	/* Allocate temporary array to read the vol file by spans: */
	ValueParam* span=new ValueParam[dataSet.getNumVertices()[2]];
	
	/* Read and convert all spans: */
	for(int x=0;x<dataSet.getNumVertices()[0];++x)
		for(int y=0;y<dataSet.getNumVertices()[1];++y)
			{
			volFile.read<ValueParam>(span,dataSet.getNumVertices()[2]);
			ValueParam* spanPtr=span;
			for(int z=0;z<dataSet.getNumVertices()[2];++z,++slicePtr,++spanPtr)
				*slicePtr=Value(*spanPtr);
			}
	
	/* Clean up: */
	delete[] span;
	}

}

/*****************************
Methods of class MultiVolFile:
*****************************/

MultiVolFile::MultiVolFile(void)
	:BaseModule("MultiVolFile")
	{
	}

Visualization::Abstract::DataSet* MultiVolFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Parse the module arguments: */
	DS::Index gridSize(0);
	DS::Point gridOrigin=DS::Point::origin;
	DS::Size gridCellSize=DS::Size(0);
	bool gridInitialized=false;
	for(size_t argc=0;argc<args.size();argc+=2)
		{
		/* Open the vol file: */
		Misc::File volFile(args[argc+1].c_str(),"rb",Misc::File::LittleEndian);
		
		/* Read the vol file header: */
		DS::Index volGridSize;
		for(int i=0;i<3;++i)
			volGridSize[i]=volFile.read<int>();
		DS::Point volGridOrigin;
		for(int i=0;i<3;++i)
			volGridOrigin[i]=Scalar(volFile.read<float>());
		DS::Size volGridCellSize;
		for(int i=0;i<3;++i)
			volGridCellSize[i]=Scalar(volFile.read<float>());
		
		bool volOk=true;
		if(gridInitialized)
			{
			/* Check the vol file for consistency: */
			if(volGridSize!=gridSize||volGridOrigin!=gridOrigin||volGridCellSize!=gridCellSize)
				{
				std::cout<<"Vol file "<<args[argc+1]<<" does not match data set layout; skipping"<<std::endl;
				volOk=false;
				}
			}
		else
			{
			/* Initialize the result data set: */
			gridSize=volGridSize;
			gridOrigin=volGridOrigin;
			gridCellSize=volGridCellSize;
			dataSet.setData(gridSize,gridCellSize,0);
			gridInitialized=true;
			}
		
		if(volOk)
			{
			/* Determine the vol file's value type: */
			unsigned int volTypeSize=volFile.read<unsigned int>();
			if(volTypeSize==1||volTypeSize==2||volTypeSize==4||volTypeSize==8)
				{
				/* Add a new slice to the data set: */
				int newSliceIndex=dataSet.addSlice();
				
				/* Add a new scalar variable to the data value: */
				dataValue.addScalarVariable(args[argc].c_str());
				
				/* Read the vol file: */
				if(volTypeSize==1)
					readVolFile<unsigned char>(volFile,dataSet,newSliceIndex);
				else if(volTypeSize==2)
					readVolFile<signed short int>(volFile,dataSet,newSliceIndex);
				else if(volTypeSize==4)
					readVolFile<float>(volFile,dataSet,newSliceIndex);
				else
					readVolFile<double>(volFile,dataSet,newSliceIndex);
				}
			else
				std::cout<<"Vol file "<<args[argc+1]<<" has unknown data type; skipping"<<std::endl;
			}
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
	Visualization::Concrete::MultiVolFile* module=new Visualization::Concrete::MultiVolFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
