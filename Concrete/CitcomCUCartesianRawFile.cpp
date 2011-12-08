/***********************************************************************
CitcomCUCartesianRawFile - Class reading raw files produced by parallel
regional CITCOMCU simulations. Raw files are binary files stored on each
processing node describing the grid and result values.
Copyright (c) 2008-2011 Oliver Kreylos

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

#include <Concrete/CitcomCUCartesianRawFile.h>

#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Plugins/FactoryManager.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Math/Math.h>

namespace Visualization {

namespace Concrete {

/*****************************************
Methods of class CitcomCUCartesianRawFile:
*****************************************/

CitcomCUCartesianRawFile::CitcomCUCartesianRawFile(void)
	:BaseModule("CitcomCUCartesianRawFile")
	{
	}

Visualization::Abstract::DataSet* CitcomCUCartesianRawFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet());
	
	std::vector<std::string>::const_iterator argIt=args.begin();
	
	DS::Index numVertices;
	DS::Index numCpus;
	
	/* Open the header file: */
	{
	IO::ValueSource headerSource(openFile((*argIt)+".hdr",pipe));
	
	/* Check the grid type: */
	std::string gridType=headerSource.readString();
	if(gridType=="multigrid")
		{
		try
			{
			/* Read description of multigrid mesh: */
			DS::Index numBlocks;
			for(int i=0;i<3;++i)
				numBlocks[i]=headerSource.readInteger();
			int numLevels=headerSource.readInteger();
			
			/* Compute the number of nodes: */
			for(int i=0;i<3;++i)
				numVertices[i]=(numBlocks[i]<<(numLevels-1))+1;
			}
		catch(IO::ValueSource::NumberError err)
			{
			Misc::throwStdErr("CitcomCUCartesianRawFile::load: Invalid multigrid definition in header file %s",((*argIt)+".hdr").c_str());
			}
		}
	else if(gridType=="conj-grad")
		{
		try
			{
			/* Read description of conjugate-gradient mesh: */
			for(int i=0;i<3;++i)
				numVertices[i]=headerSource.readInteger();
			}
		catch(IO::ValueSource::NumberError err)
			{
			Misc::throwStdErr("CitcomCUCartesianRawFile::load: Invalid conjugate gradient definition in header file %s",((*argIt)+".hdr").c_str());
			}
		}
	else
		Misc::throwStdErr("CitcomCUCartesianRawFile::load: Unrecognized mesh type %s in header file %s",gridType.c_str(),((*argIt)+".hdr").c_str());
	
	try
		{
		/* Read the number of CPUs: */
		for(int i=0;i<3;++i)
			numCpus[i]=headerSource.readInteger();
		}
	catch(IO::ValueSource::NumberError err)
		{
		Misc::throwStdErr("CitcomCUCartesianRawFile::load: Invalid number of CPUs in header file %s",((*argIt)+".hdr").c_str());
		}
	}
	
	/* Initialize the data set: */
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	
	/* Compute the number of nodes per CPU: */
	DS::Index cpuNumVertices;
	for(int i=0;i<3;++i)
		cpuNumVertices[i]=(numVertices[i]-1)/numCpus[i]+1;
	int totalCpuNumVertices=cpuNumVertices.calcIncrement(-1);
	
	/* Create temporary arrays to hold the grid vertex components: */
	float* gridVertices[3];
	for(int i=0;i<3;++i)
		gridVertices[i]=new float[totalCpuNumVertices];
	
	/* Read grid and value files from each CPU and merge them into the data set: */
	if(master)
		std::cout<<"Reading grid vertex positions...   0%"<<std::flush;
	int cpuCounter=0;
	for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
		{
		/* Calculate the base grid index and linear number for this CPU: */
		DS::Index cpuBase;
		for(int i=0;i<3;++i)
			cpuBase[i]=(cpuNumVertices[i]-1)*cpuIndex[i];
		int cpuNumber=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
		
		/* Read the CPU's grid definition files: */
		for(int i=0;i<3;++i)
			{
			/* Read the grid file into the temporary array, skipping the first (bogus) element: */
			std::string gridFileName=*argIt;
			gridFileName.push_back('.');
			gridFileName.push_back('x'+i);
			gridFileName.push_back('.');
			gridFileName.append(Misc::ValueCoder<int>::encode(cpuNumber));
			IO::FilePtr gridFile(openFile(gridFileName,pipe));
			gridFile->setEndianness(Misc::LittleEndian);
			gridFile->skip<float>(1);
			gridFile->read(gridVertices[i],totalCpuNumVertices);
			}
		
		/* Assemble and write the CPU's grid vertices: */
		DS::Index index;
		int linearIndex=0;
		for(index[1]=0;index[1]<cpuNumVertices[1];++index[1])
			for(index[0]=0;index[0]<cpuNumVertices[0];++index[0])
				for(index[2]=0;index[2]<cpuNumVertices[2];++index[2],++linearIndex)
					{
					/* Get a reference to the vertex in the merged grid: */
					DS::Point& vertex=dataSet.getVertexPosition(DS::Index(cpuBase[0]+index[0],cpuBase[1]+index[1],cpuBase[2]+index[2]));
					
					/* Copy the grid point: */
					for(int i=0;i<3;++i)
						vertex[i]=Scalar(gridVertices[i][linearIndex]);
					}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Delete the grid vertex arrays: */
	for(int i=0;i<3;++i)
		delete[] gridVertices[i];
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Read the time step index given on the command line: */
	++argIt;
	bool isNumber=false;
	std::string::const_iterator tiIt=argIt->begin();
	if(tiIt!=argIt->end())
		{
		isNumber=*tiIt=='-'||isdigit(*tiIt);
		for(++tiIt;isNumber&&tiIt!=argIt->end();++tiIt)
			isNumber=isdigit(*tiIt);
		}
	if(!isNumber)
		Misc::throwStdErr("CitcomCUCartesianRawFile::load: no time step index provided");
	int timeStepIndex=atoi(argIt->c_str());
	
	/* Read all data components given on the command line: */
	bool logNextScalar=false;
	bool nextVector=false;
	for(++argIt;argIt!=args.end();++argIt)
		{
		if(strcasecmp(argIt->c_str(),"-log")==0)
			logNextScalar=true;
		else if(strcasecmp(argIt->c_str(),"-vector")==0)
			nextVector=true;
		else
			{
			/* Remember the (base) slice index for this variable: */
			int sliceIndex=dataSet.getNumSlices();
			
			if(nextVector)
				{
				/* Add another vector variable to the data value: */
				int vectorVariableIndex=dataValue.addVectorVariable(argIt->c_str());
				if(master)
					std::cout<<"Reading vector variable "<<*argIt<<"...   0%"<<std::flush;
				
				/* Add four new slices to the data set (three components plus magnitude): */
				for(int i=0;i<4;++i)
					{
					dataSet.addSlice();
					int variableIndex=dataValue.addScalarVariable(makeVectorSliceName(*argIt,i).c_str());
					if(i<3)
						dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,variableIndex);
					}
				}
			else
				{
				/* Add another scalar variable to the data value: */
				dataSet.addSlice();
				if(logNextScalar)
					{
					std::string scalarName="log(";
					scalarName.append(*argIt);
					scalarName.push_back(')');
					dataValue.addScalarVariable(scalarName.c_str());
					if(master)
						std::cout<<"Reading scalar variable "<<scalarName<<"...   0%"<<std::flush;
					}
				else
					{
					dataValue.addScalarVariable(argIt->c_str());
					if(master)
						std::cout<<"Reading scalar variable "<<*argIt<<"...   0%"<<std::flush;
					}
				}
			
			/* Create a temporary array for the data values: */
			float* dataValues=new float[nextVector?totalCpuNumVertices*3:totalCpuNumVertices];
			
			/* Read data files for all CPUs: */
			cpuCounter=0;
			for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
				{
				/* Calculate the base grid index and linear number for this CPU: */
				DS::Index cpuBase;
				for(int i=0;i<3;++i)
					cpuBase[i]=(cpuNumVertices[i]-1)*cpuIndex[i];
				int cpuNumber=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
				
				/* Read the data file into the temporary array, skipping the first (bogus) element: */
				std::string dataFileName=args[0];
				dataFileName.push_back('.');
				dataFileName.append(*argIt);
				dataFileName.push_back('.');
				dataFileName.append(Misc::ValueCoder<int>::encode(cpuNumber));
				dataFileName.push_back('.');
				dataFileName.append(Misc::ValueCoder<int>::encode(timeStepIndex));
				IO::FilePtr dataFile(openFile(dataFileName,pipe));
				dataFile->setEndianness(Misc::LittleEndian);
				dataFile->skip<float>(1);
				dataFile->read(dataValues,nextVector?totalCpuNumVertices*3:totalCpuNumVertices);
				
				/* Write the CPU's data values: */
				DS::Index index;
				float* dvPtr=dataValues;
				for(index[1]=0;index[1]<cpuNumVertices[1];++index[1])
					for(index[0]=0;index[0]<cpuNumVertices[0];++index[0])
						for(index[2]=0;index[2]<cpuNumVertices[2];++index[2])
							{
							DS::Index gridIndex(cpuBase[0]+index[0],cpuBase[1]+index[1],cpuBase[2]+index[2]);
							if(nextVector)
								{
								/* Store the vector value: */
								DataValue::VVector vector;
								for(int i=0;i<3;++i)
									{
									vector[i]=VScalar(dvPtr[i]);
									dataSet.getVertexValue(sliceIndex+i,gridIndex)=vector[i];
									}
								dataSet.getVertexValue(sliceIndex+3,gridIndex)=VScalar(Geometry::mag(vector));
								dvPtr+=3;
								}
							else
								{
								dataSet.getVertexValue(sliceIndex,gridIndex)=logNextScalar?VScalar(Math::log10(double(*dvPtr))):VScalar(*dvPtr);
								++dvPtr;
								}
							}
				if(master)
					std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
				}
			if(master)
				std::cout<<"\b\b\b\bdone"<<std::endl;
			
			/* Delete the temporary array: */
			delete[] dataValues;
			
			if(nextVector)
				nextVector=false;
			else
				logNextScalar=false;
			}
		}
	
	/* Return the result data set: */
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
	Visualization::Concrete::CitcomCUCartesianRawFile* module=new Visualization::Concrete::CitcomCUCartesianRawFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
