/***********************************************************************
GocadVoxetFile - Class to encapsulate operations on multi-scalar-valued
data sets stored in GoCAD Voxet format.
Copyright (c) 2009 Oliver Kreylos

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

#include <Concrete/GocadVoxetFile.h>

#include <string.h>
#include <string>
#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Misc/HashTable.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
****************/

void skipValues(IO::ValueSource& source,int numValues)
	{
	for(int i=0;i<numValues;++i)
		source.skipString();
	}

template <class ScalarParam>
inline
void
readVector(
	IO::ValueSource& source,
	ScalarParam vector[])
	{
	for(int i=0;i<3;++i)
		vector[i]=ScalarParam(source.readNumber());
	}

double readLength(IO::ValueSource& source)
	{
	double length=0.0;
	for(int i=0;i<3;++i)
		length+=Math::sqr(source.readNumber());
	return Math::sqrt(length);
	}

}

/*******************************
Methods of class GocadVoxetFile:
*******************************/

GocadVoxetFile::GocadVoxetFile(void)
	:BaseModule("GocadVoxetFile")
	{
	}

Visualization::Abstract::DataSet* GocadVoxetFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Parse the command line: */
	bool saveCoords=false;
	DS::Size scale(1,1,1);
	std::string fileName;
	for(std::vector<std::string>::const_iterator aIt=args.begin();aIt!=args.end();++aIt)
		{
		if((*aIt)[0]=='-')
			{
			if(strcasecmp(aIt->c_str(),"-saveCoords")==0)
				saveCoords=true;
			else if(strcasecmp(aIt->c_str(),"-scale")==0)
				{
				for(int i=0;i<3;++i)
					{
					++aIt;
					scale[i]=DS::Scalar(atof(aIt->c_str()));
					}
				}
			}
		else if(fileName.empty())
			fileName=*aIt;
		}
	if(fileName.empty())
		Misc::throwStdErr("GocadVoxelFile::load: No input file name provided");
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Open the voxet file: */
	IO::ValueSource voxet(openFile(fileName.c_str(),pipe));
	voxet.setPunctuation("{}");
	voxet.setQuotes("\"");
	voxet.skipWs();
	
	/* Parse the voxet file header: */
	if(voxet.readString()!="GOCAD"||voxet.readString()!="Voxet"||voxet.readString()!="1")
		Misc::throwStdErr("GocadVoxetFile::load: File %s is not a valid GoCAD Voxet file",fileName.c_str());
	if(voxet.readString()!="HEADER"||voxet.readString()!="{")
		Misc::throwStdErr("GocadVoxetFile::load: File %s is not a valid GoCAD Voxet file",fileName.c_str());
	while(!voxet.eof()&&voxet.peekc()!='}')
		voxet.skipString();
	
	/* Read keywords from the voxet file: */
	DS::Point domainOrigin=DS::Point::origin;
	DS::Size domainSize(0,0,0);
	DS::Index numVertices(0,0,0);
	bool haveDataSet=false;
	Misc::HashTable<int,int> propertyIndexMap(17);
	Misc::HashTable<int,Value> propertyNanMap(17);
	while(!voxet.eof())
		{
		std::string keyword=voxet.readString();
		if(keyword=="AXIS_O")
			readVector(voxet,domainOrigin.getComponents());
		else if(keyword=="AXIS_U")
			domainSize[0]=DS::Scalar(readLength(voxet))*scale[0];
		else if(keyword=="AXIS_V")
			domainSize[1]=DS::Scalar(readLength(voxet))*scale[1];
		else if(keyword=="AXIS_W")
			domainSize[2]=DS::Scalar(readLength(voxet))*scale[2];
		else if(keyword=="AXIS_MIN")
			skipValues(voxet,3);
		else if(keyword=="AXIS_MAX")
			skipValues(voxet,3);
		else if(keyword=="AXIS_N")
			{
			for(int i=0;i<3;++i)
				numVertices[i]=int(voxet.readUnsignedInteger());
			}
		else if(keyword=="AXIS_NAME")
			skipValues(voxet,3);
		else if(keyword=="AXIS_UNIT")
			skipValues(voxet,3);
		else if(keyword=="AXIS_TYPE")
			skipValues(voxet,3);
		else if(keyword=="PROPERTY")
			{
			if(!haveDataSet)
				{
				/* Check if the data set grid structure is defined: */
				bool defined=true;
				for(int i=0;i<3;++i)
					defined=defined&&numVertices[i]>0&&domainSize[i]>DS::Scalar(0);
				if(!defined)
					Misc::throwStdErr("GocadVoxetFile::load: File %s defines properties before the grid structure",fileName.c_str());
				
				/* Initialize the data set's grid structure: */
				DS::Size cellSize;
				for(int i=0;i<3;++i)
					cellSize[i]=domainSize[i]/DS::Scalar(numVertices[i]-1);
				dataSet.setData(numVertices,cellSize,0);
				
				haveDataSet=true;
				}
			
			/* Read the property index and name: */
			int index=voxet.readInteger();
			std::string name=voxet.readString();
			
			/* Add a new slice to the data set: */
			int newSliceIndex=dataSet.addSlice();
			propertyIndexMap.setEntry(Misc::HashTable<int,int>::Entry(index,newSliceIndex));
			
			/* Add a new scalar variable to the data value: */
			dataValue.addScalarVariable(name.c_str());
			}
		else if(keyword=="PROPERTY_CLASS")
			skipValues(voxet,2);
		else if(keyword=="PROP_ORIGINAL_UNIT")
			skipValues(voxet,2);
		else if(keyword=="PROP_UNIT")
			skipValues(voxet,2);
		else if(keyword=="PROP_NO_DATA_VALUE")
			{
			/* Read the property index and nan value: */
			int index=voxet.readInteger();
			Value nanValue=Value(voxet.readNumber());
			propertyNanMap.setEntry(Misc::HashTable<int,Value>::Entry(index,nanValue));
			}
		else if(keyword=="PROP_SAMPLE_STATS")
			skipValues(voxet,6);
		else if(keyword=="PROP_ESIZE")
			{
			voxet.skipString();
			if(voxet.readInteger()!=sizeof(Value))
				Misc::throwStdErr("GocadVoxetFile::load: File %s contains a property with non-floating-point values",fileName.c_str());
			}
		else if(keyword=="PROP_ETYPE")
			{
			voxet.skipString();
			if(voxet.readString()!="IEEE")
				Misc::throwStdErr("GocadVoxetFile::load: File %s contains a property with non-floating-point values",fileName.c_str());
			}
		else if(keyword=="PROP_PAINTED_FLAG_BIT_POS")
			skipValues(voxet,2);
		else if(keyword=="PROP_FORMAT")
			{
			voxet.skipString();
			if(voxet.readString()!="RAW")
				Misc::throwStdErr("GocadVoxetFile::load: File %s contains a property with non-raw values",fileName.c_str());
			}
		else if(keyword=="PROP_OFFSET")
			{
			voxet.skipString();
			if(voxet.readNumber()!=0.0)
				Misc::throwStdErr("GocadVoxetFile::load: File %s contains a property with non-zero offset",fileName.c_str());
			}
		else if(keyword=="PROP_FILE")
			{
			/* Get the property index and its slice index in the data set: */
			int propertyIndex=voxet.readInteger();
			int sliceIndex=propertyIndexMap.getEntry(propertyIndex).getDest();
			Value nanValue=propertyNanMap.getEntry(propertyIndex).getDest();
			std::string propertyFileName=voxet.readString();
			
			/* Extract the base directory name from the voxet file name: */
			std::string::iterator slashIt=fileName.end();
			for(std::string::iterator fnIt=fileName.begin();fnIt!=fileName.end();++fnIt)
				if(*fnIt=='/')
					slashIt=fnIt;
			if(slashIt!=fileName.end())
				{
				/* Prefix the property file name with the base directory name: */
				std::string dirName=std::string(fileName.begin(),slashIt+1);
				dirName+=propertyFileName;
				propertyFileName=dirName;
				}
			
			/* Read the property values from the property file: */
			std::cout<<"Reading property from file "<<propertyFileName<<"..."<<std::flush;
			IO::FilePtr propertyFile(openFile(propertyFileName.c_str(),pipe));
			propertyFile->setEndianness(Misc::BigEndian);
			Value* slicePtr=dataSet.getSliceArray(sliceIndex);
			Value* tempArray=new Value[numVertices[0]];
			DS::Index index;
			ptrdiff_t spanStride=numVertices[1]*numVertices[2];
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				{
				for(index[1]=0;index[1]<numVertices[1];++index[1])
					{
					/* Read a span of data from the file: */
					propertyFile->read<Value>(tempArray,numVertices[0]);
					
					/* Copy the data span into the data set: */
					Value* sPtr=tempArray;
					Value* dPtr=slicePtr+(index[1]*numVertices[2]+index[2]);
					for(index[0]=0;index[0]<numVertices[0];++index[0],++sPtr,dPtr+=spanStride)
						*dPtr=*sPtr==nanValue?Value(0):*sPtr;
					}
				}
			delete[] tempArray;
			std::cout<<" done"<<std::endl;
			}
		else if(keyword=="END")
			break;
		}
	
	if(saveCoords)
		{
		/* Save vertex coordinates as additional scalar variables: */
		static const char* coordNames[3]={"X","Y","Z"};
		Value* slicePtr[3];
		for(int i=0;i<3;++i)
			{
			int newSliceIndex=dataSet.addSlice();
			dataValue.addScalarVariable(coordNames[i]);
			slicePtr[i]=dataSet.getSliceArray(newSliceIndex);
			}
		
		DS::Size cellSize;
		for(int i=0;i<3;++i)
			cellSize[i]=domainSize[i]/DS::Scalar(numVertices[i]-1);
		for(DS::Index index(0);index[0]<numVertices[0];index.preInc(numVertices))
			for(int i=0;i<3;++i)
				slicePtr[i][(index[0]*numVertices[1]+index[1])*numVertices[2]+index[2]]=domainOrigin[i]+cellSize[i]*DS::Scalar(index[i]);
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
	Visualization::Concrete::GocadVoxetFile* module=new Visualization::Concrete::GocadVoxetFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
