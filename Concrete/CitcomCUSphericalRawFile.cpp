/***********************************************************************
CitcomCUSphericalRawFile - Class reading raw files produced by parallel
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

#include <Concrete/CitcomCUSphericalRawFile.h>

#include <string.h>
#include <stdio.h>
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

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>

namespace Visualization {

namespace Concrete {

/*****************************************
Methods of class CitcomCUSphericalRawFile:
*****************************************/

CitcomCUSphericalRawFile::CitcomCUSphericalRawFile(void)
	:BaseModule("CitcomCUSphericalRawFile")
	{
	}

Visualization::Abstract::DataSet* CitcomCUSphericalRawFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<EarthDataSet<DataSet> > result(new EarthDataSet<DataSet>(args));
	result->setFlatteningFactor(0.0);
	result->getSphericalCoordinateTransformer()->setColatitude(true);
	
	/* Parse command line parameters related to the grid definition file: */
	std::vector<std::string>::const_iterator argIt=args.begin();
	bool storeSphericals=false;
	while((*argIt)[0]=='-')
		{
		/* Parse the command line parameter: */
		if(strcasecmp(argIt->c_str(),"-storeCoords")==0)
			storeSphericals=true;
		
		++argIt;
		}
	
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
			Misc::throwStdErr("CitcomCUSphericalRawFile::load: Invalid multigrid definition in header file %s",((*argIt)+".hdr").c_str());
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
			Misc::throwStdErr("CitcomCUSphericalRawFile::load: Invalid conjugate gradient definition in header file %s",((*argIt)+".hdr").c_str());
			}
		}
	else
		Misc::throwStdErr("CitcomCUSphericalRawFile::load: Unrecognized mesh type %s in header file %s",gridType.c_str(),((*argIt)+".hdr").c_str());
	
	try
		{
		/* Read the number of CPUs: */
		for(int i=0;i<3;++i)
			numCpus[i]=headerSource.readInteger();
		}
	catch(IO::ValueSource::NumberError err)
		{
		Misc::throwStdErr("CitcomCUSphericalRawFile::load: Invalid number of CPUs in header file %s",((*argIt)+".hdr").c_str());
		}
	}
	
	/* Initialize the data set: */
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	if(storeSphericals)
		{
		/* Add three slices to the data set: */
		static const char* coordSliceNames[3]={"Colatitude","Longitude","Radius"};
		for(int i=0;i<3;++i)
			{
			dataSet.addSlice();
			dataValue.addScalarVariable(coordSliceNames[i]);
			}
		}
	
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
		
		/* Prepare the spherical-to-Cartesian formula: */
		const double a=6378.14e3; // Equatorial radius in m
		// const double f=1.0/298.247; // Geoid flattening factor (not used, since there could be vector values)
		const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
		
		/* Assemble and write the CPU's grid vertices: */
		DS::Index index;
		int linearIndex=0;
		for(index[1]=0;index[1]<cpuNumVertices[1];++index[1])
			for(index[0]=0;index[0]<cpuNumVertices[0];++index[0])
				for(index[2]=0;index[2]<cpuNumVertices[2];++index[2],++linearIndex)
					{
					/* Get a reference to the vertex in the merged grid: */
					DS::Index gIndex=cpuBase+index;
					DS::Point& vertex=dataSet.getVertexPosition(gIndex);
					
					/* Convert the input grid point from spherical to Cartesian coordinates: */
					double latitude=Math::rad(90.0)-double(gridVertices[0][linearIndex]);
					double s0=Math::sin(latitude);
					double c0=Math::cos(latitude);
					double longitude=double(gridVertices[1][linearIndex]);
					double s1=Math::sin(longitude);
					double c1=Math::cos(longitude);
					double r=double(gridVertices[2][linearIndex])*a*scaleFactor;
					double xy=r*c0;
					vertex[0]=Scalar(xy*c1);
					vertex[1]=Scalar(xy*s1);
					vertex[2]=Scalar(r*s0);
					
					if(storeSphericals)
						{
						dataSet.getVertexValue(0,gIndex)=Scalar(Math::deg(double(gridVertices[0][linearIndex])));
						dataSet.getVertexValue(1,gIndex)=Scalar(Math::deg(longitude));
						dataSet.getVertexValue(2,gIndex)=Scalar(r);
						}
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
		Misc::throwStdErr("CitcomCUSphericalRawFile::load: no time step index provided");
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
				
				/* Add seven new slices to the data set (3 components spherical and Cartesian each plus Cartesian magnitude): */
				static const char* componentNames[7]={" Colatitude"," Longitude"," Radius"," X"," Y"," Z"," Magnitude"};
				for(int i=0;i<7;++i)
					{
					dataSet.addSlice();
					dataValue.addScalarVariable(((*argIt)+componentNames[i]).c_str());
					}
				
				/* Set the vector variables' component indices: */
				for(int i=0;i<3;++i)
					dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,sliceIndex+3+i);
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
								/* Convert the vector from spherical to Cartesian coordinates: */
								const DS::Point& p=dataSet.getVertexPosition(gridIndex);
								double xy=Math::sqr(double(p[0]))+Math::sqr(double(p[1]));
								double r=xy+Math::sqr(double(p[2]));
								xy=Math::sqrt(xy);
								r=Math::sqrt(r);
								double s0=double(p[2])/r;
								double c0=xy/r;
								double s1=double(p[1])/xy;
								double c1=double(p[0])/xy;
								DataValue::VVector vector;
								vector[0]=VScalar(c1*(c0*double(dvPtr[2])+s0*double(dvPtr[0]))-s1*double(dvPtr[1]));
								vector[1]=VScalar(s1*(c0*double(dvPtr[2])+s0*double(dvPtr[0]))+c1*double(dvPtr[1]));
								vector[2]=VScalar(s0*dvPtr[2]-c0*dvPtr[0]);
								dataSet.getVertexValue(sliceIndex+0,gridIndex)=VScalar(dvPtr[0]);
								dataSet.getVertexValue(sliceIndex+1,gridIndex)=VScalar(dvPtr[1]);
								dataSet.getVertexValue(sliceIndex+2,gridIndex)=VScalar(dvPtr[2]);
								for(int i=0;i<3;++i)
									dataSet.getVertexValue(sliceIndex+3+i,gridIndex)=vector[i];
								dataSet.getVertexValue(sliceIndex+6,gridIndex)=VScalar(Geometry::mag(vector));
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

Visualization::Abstract::DataSetRenderer* CitcomCUSphericalRawFile::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new EarthDataSetRenderer<DataSet,DataSetRenderer>(dataSet);
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::CitcomCUSphericalRawFile* module=new Visualization::Concrete::CitcomCUSphericalRawFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
