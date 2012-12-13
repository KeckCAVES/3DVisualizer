/***********************************************************************
CitcomSRegionalASCIIFile - Class reading ASCII files produced by
parallel regional CitcomS simulations. These are the uncombined files
produced by each CPU in a parallel run.
Copyright (c) 2008-2012 Oliver Kreylos

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

#include <Concrete/CitcomSRegionalASCIIFile.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>
#include <Concrete/CitcomSCfgFileParser.h>

namespace Visualization {

namespace Concrete {

/*****************************************
Methods of class CitcomSRegionalASCIIFile:
*****************************************/

CitcomSRegionalASCIIFile::CitcomSRegionalASCIIFile(void)
	:BaseModule("CitcomSRegionalASCIIFile")
	{
	}

Visualization::Abstract::DataSet* CitcomSRegionalASCIIFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
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
		if(*argIt=="-storeCoords")
			storeSphericals=true;
		
		++argIt;
		}
	
	/* Parse the run's configuration file: */
	std::string fullCfgName=getFullPath(*argIt);
	IO::FilePtr cfgFile(openFile(fullCfgName,pipe));
	std::string dataDir;
	std::string dataFileName;
	int numSurfaces=0;
	DS::Index numCpus(0,0,0);
	DS::Index numVertices(0,0,0);
	parseCitcomSCfgFile(fullCfgName,cfgFile,dataDir,dataFileName,numSurfaces,numCpus,numVertices);
	if(numSurfaces==0||numCpus.calcIncrement(-1)==0||numVertices.calcIncrement(-1)==0)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: %s is not a valid CitcomS configuration file",fullCfgName.c_str());
	
	/* Check if it's really a regional model: */
	if(numSurfaces!=1)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: configuration file %s does not describe a regional model; use CitcomSGlobalASCIIFile instead",fullCfgName.c_str());
	
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
	
	/* Prepare the spherical-to-Cartesian formula: */
	const double a=6378.14e3; // Equatorial radius in m
	// const double f=1.0/298.247; // Geoid flattening factor (not used, since there could be vector values)
	const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
	
	/* Read the grid coordinate files for all CPUs: */
	if(master)
		std::cout<<"Reading grid vertex positions...   0%"<<std::flush;
	int cpuCounter=0;
	DS::GridArray& grid=dataSet.getGrid();
	for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
		{
		/* Open the CPU's coordinate file: */
		int cpuLinearIndex=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
		std::string coordFileName=dataDir;
		coordFileName.append(dataFileName);
		coordFileName.append(".coord.");
		coordFileName.append(Misc::ValueCoder<int>::encode(cpuLinearIndex));
		IO::ValueSource coordReader(openFile(coordFileName,pipe));
		coordReader.skipWs();
		
		/* Read and check the header line: */
		try
			{
			/* Skip the unknown value: */
			coordReader.readInteger();
			
			/* Read the number of vertices: */
			if(coordReader.readInteger()!=totalCpuNumVertices)
				Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in coordinate file %s",coordFileName.c_str());
			}
		catch(IO::ValueSource::NumberError err)
			{
			Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in coordinate file %s",coordFileName.c_str());
			}
		
		/* Compute the CPU's base index in the surface's grid: */
		DS::Index cpuBaseIndex;
		for(int i=0;i<3;++i)
			cpuBaseIndex[i]=(cpuNumVertices[i]-1)*cpuIndex[i];
		
		/* Read the grid vertices: */
		DS::Index gridIndex;
		for(gridIndex[1]=0;gridIndex[1]<cpuNumVertices[1];++gridIndex[1])
			for(gridIndex[0]=0;gridIndex[0]<cpuNumVertices[0];++gridIndex[0])
				for(gridIndex[2]=0;gridIndex[2]<cpuNumVertices[2];++gridIndex[2])
					{
					/* Read the next grid vertex: */
					try
						{
						double colatitude=coordReader.readNumber();
						double longitude=coordReader.readNumber();
						double radius=coordReader.readNumber();
						
						/* Convert the vertex to Cartesian coordinates: */
						double latitude=Math::rad(90.0)-colatitude;
						double s0=Math::sin(latitude);
						double c0=Math::cos(latitude);
						double s1=Math::sin(longitude);
						double c1=Math::cos(longitude);
						double r=radius*a*scaleFactor;
						double xy=r*c0;
						DS::Index gIndex=cpuBaseIndex+gridIndex;
						DS::Point& vertex=grid(gIndex);
						vertex[0]=Scalar(xy*c1);
						vertex[1]=Scalar(xy*s1);
						vertex[2]=Scalar(r*s0);
						
						if(storeSphericals)
							{
							/* Store the original spherical coordinates as a scalar field: */
							dataSet.getVertexValue(0,gIndex)=Scalar(Math::deg(colatitude));
							dataSet.getVertexValue(1,gIndex)=Scalar(Math::deg(longitude));
							dataSet.getVertexValue(2,gIndex)=Scalar(r);
							}
						}
					catch(IO::ValueSource::NumberError err)
						{
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex definition in coordinate file %s",coordFileName.c_str());
						}
					}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Read the time step index given on the command line: */
	++argIt;
	bool isNumber=true;
	int timeStepIndex=0;
	for(std::string::const_iterator tiIt=argIt->begin();isNumber&&tiIt!=argIt->end();++tiIt)
		{
		if(*tiIt>='0'&&*tiIt<='9')
			timeStepIndex=timeStepIndex*10+int(*tiIt-'0');
		else
			isNumber=false;
		}
	if(!isNumber)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: no time step index provided");
	
	/* Read all data components given on the command line: */
	bool logNextScalar=false;
	bool nextVector=false;
	for(++argIt;argIt!=args.end();++argIt)
		{
		if(*argIt=="-log")
			logNextScalar=true;
		else if(*argIt=="-vector")
			nextVector=true;
		else
			{
			/* Remember the (base) slice index for this variable: */
			int sliceIndex=dataSet.getNumSlices();
			
			/* Check if it's the special-case velo two-variable file (bleargh): */
			bool isVeloFile=*argIt=="velo";
			if(isVeloFile||nextVector)
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
				
				/* Test for the special case of the velo file: */
				if(isVeloFile)
					{
					/* Add another scalar variable to the data value: */
					dataSet.addSlice();
					if(logNextScalar)
						dataValue.addScalarVariable("log(temp)");
					else
						dataValue.addScalarVariable("temp");
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
			
			/* Read data files for all CPUs: */
			cpuCounter=0;
			DS::GridArray& grid=dataSet.getGrid();
			for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
				{
				/* Open the CPU's data value file: */
				int cpuLinearIndex=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
				std::string dataValueFileName=dataDir;
				dataValueFileName.append(dataFileName);
				dataValueFileName.push_back('.');
				dataValueFileName.append(*argIt);
				dataValueFileName.push_back('.');
				dataValueFileName.append(Misc::ValueCoder<int>::encode(cpuLinearIndex));
				dataValueFileName.push_back('.');
				dataValueFileName.append(Misc::ValueCoder<int>::encode(timeStepIndex));
				IO::ValueSource dataValueReader(openFile(dataValueFileName,pipe));
				dataValueReader.skipWs();
				
				/* Read and check the header line(s) in the data value file: */
				try
					{
					int dataValueFileNumVertices1=totalCpuNumVertices;
					if(isVeloFile)
						{
						/* Read the first header line only found in velo files: */
						dataValueReader.readInteger();
						dataValueFileNumVertices1=dataValueReader.readInteger();
						dataValueReader.readNumber();
						}
					
					/* Read the common header line: */
					dataValueReader.readInteger();
					int dataValueFileNumVertices2=dataValueReader.readInteger();
					
					/* Check for consistency: */
					if(dataValueFileNumVertices1!=totalCpuNumVertices||dataValueFileNumVertices2!=totalCpuNumVertices)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in data value file %s",dataValueFileName.c_str());
					}
				catch(IO::ValueSource::NumberError err)
					{
					Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in data value file %s",dataValueFileName.c_str());
					}
				
				/* Compute the CPU's base index in the surface's grid: */
				DS::Index cpuBaseIndex;
				for(int i=0;i<3;++i)
					cpuBaseIndex[i]=(cpuNumVertices[i]-1)*cpuIndex[i];
				
				/* Read the grid vertices: */
				DS::Index gridIndex;
				for(gridIndex[1]=0;gridIndex[1]<cpuNumVertices[1];++gridIndex[1])
					for(gridIndex[0]=0;gridIndex[0]<cpuNumVertices[0];++gridIndex[0])
						for(gridIndex[2]=0;gridIndex[2]<cpuNumVertices[2];++gridIndex[2])
							{
							/* Read the next vertex' value: */
							DS::Index index=cpuBaseIndex+gridIndex;
							
							try
								{
								if(isVeloFile||nextVector)
									{
									/* Read the vector components: */
									double colatitude=dataValueReader.readNumber();
									double longitude=dataValueReader.readNumber();
									double radius=dataValueReader.readNumber();
									
									/* Convert the vector from spherical to Cartesian coordinates: */
									const DS::Point& p=grid(index);
									double xy=Math::sqr(double(p[0]))+Math::sqr(double(p[1]));
									double r=xy+Math::sqr(double(p[2]));
									xy=Math::sqrt(xy);
									r=Math::sqrt(r);
									double s0=double(p[2])/r;
									double c0=xy/r;
									double s1=double(p[1])/xy;
									double c1=double(p[0])/xy;
									DataValue::VVector vector;
									vector[0]=VScalar(c1*(c0*radius+s0*colatitude)-s1*longitude);
									vector[1]=VScalar(s1*(c0*radius+s0*colatitude)+c1*longitude);
									vector[2]=VScalar(s0*radius-c0*colatitude);
									dataSet.getVertexValue(sliceIndex+0,index)=VScalar(colatitude);
									dataSet.getVertexValue(sliceIndex+1,index)=VScalar(longitude);
									dataSet.getVertexValue(sliceIndex+2,index)=VScalar(radius);
									for(int i=0;i<3;++i)
										dataSet.getVertexValue(sliceIndex+3+i,index)=vector[i];
									dataSet.getVertexValue(sliceIndex+6,index)=VScalar(Geometry::mag(vector));
									
									if(isVeloFile)
										{
										/* Read the temperature value: */
										double temp=dataValueReader.readNumber();
										dataSet.getVertexValue(sliceIndex+7,index)=logNextScalar?VScalar(Math::log10(temp)):VScalar(temp);
										}
									}
								else
									{
									/* Read the scalar value: */
									double value=dataValueReader.readNumber();
									dataSet.getVertexValue(sliceIndex,index)=logNextScalar?VScalar(Math::log10(value)):VScalar(value);
									}
								}
							catch(IO::ValueSource::NumberError err)
								{
								Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex value definition in data value file %s",dataValueFileName.c_str());
								}
							}
				if(master)
					std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
				}
			if(master)
				std::cout<<"\b\b\b\bdone"<<std::endl;
			
			if(nextVector)
				nextVector=false;
			else
				logNextScalar=false;
			}
		}
	
	/* Return the result data set: */
	return result.releaseTarget();
	}

Visualization::Abstract::DataSetRenderer* CitcomSRegionalASCIIFile::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
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
	Visualization::Concrete::CitcomSRegionalASCIIFile* module=new Visualization::Concrete::CitcomSRegionalASCIIFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
