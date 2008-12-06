/***********************************************************************
CitcomSRegionalASCIIFile - Class reading ASCII files produced by
parallel regional CitcomS simulations. These are the uncombined files
produced by each CPU in a parallel run.
Copyright (c) 2008 Oliver Kreylos

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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>
#include <Concrete/CitcomSCfgFileParser.h>

#include <Concrete/CitcomSRegionalASCIIFile.h>

namespace Visualization {

namespace Concrete {

/*****************************************
Methods of class CitcomSRegionalASCIIFile:
*****************************************/

CitcomSRegionalASCIIFile::CitcomSRegionalASCIIFile(void)
	:BaseModule("CitcomSRegionalASCIIFile")
	{
	}

Visualization::Abstract::DataSet* CitcomSRegionalASCIIFile::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
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
	
	/* Parse the run's configuration file: */
	std::string dataDir;
	std::string dataFileName;
	int numSurfaces=0;
	DS::Index numCpus(0,0,0);
	DS::Index numVertices(0,0,0);
	parseCitcomSCfgFile(argIt->c_str(),dataDir,dataFileName,numSurfaces,numCpus,numVertices);
	if(numSurfaces==0||numCpus.calcIncrement(-1)==0||numVertices.calcIncrement(-1)==0)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: %s is not a valid CitcomS configuration file",argIt->c_str());
	
	/* Check if it's really a regional model: */
	if(numSurfaces!=1)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: configuration file %s does not describe a regional model; use CitcomSGlobalASCIIFile instead",argIt->c_str());
	
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
	std::cout<<"Reading grid vertex positions...   0%"<<std::flush;
	int cpuCounter=0;
	DS::GridArray& grid=dataSet.getGrid();
	for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
		{
		/* Open the CPU's coordinate file: */
		char coordFileName[1024];
		int cpuLinearIndex=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
		snprintf(coordFileName,sizeof(coordFileName),"%s/%s.coord.%d",dataDir.c_str(),dataFileName.c_str(),cpuLinearIndex);
		Misc::File coordFile(coordFileName,"rt");
		
		/* Read and check the header line: */
		char line[256];
		coordFile.gets(line,sizeof(line));
		int dummy,coordFileNumVertices;
		if(sscanf(line,"%d %d",&dummy,&coordFileNumVertices)!=2)
			Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in coordinate file %s",coordFileName);
		if(coordFileNumVertices!=totalCpuNumVertices)
			Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in coordinate file %s",coordFileName);
		
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
					/* Read the next line: */
					coordFile.gets(line,sizeof(line));
					
					/* Parse the grid vertex: */
					double colatitude,longitude,radius;
					if(sscanf(line,"%lf %lf %lf",&colatitude,&longitude,&radius)!=3)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex definition in coordinate file %s",coordFileName);
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
						dataSet.getVertexValue(0,gIndex)=Scalar(Math::deg(colatitude));
						dataSet.getVertexValue(1,gIndex)=Scalar(Math::deg(longitude));
						dataSet.getVertexValue(2,gIndex)=Scalar(r);
						}
					}
		
		std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	std::cout<<" done"<<std::endl;
	
	/* Read the time step index given on the command line: */
	++argIt;
	bool isNumber=true;
	for(std::string::const_iterator tiIt=argIt->begin();isNumber&&tiIt!=argIt->end();++tiIt)
		isNumber=isdigit(*tiIt);
	if(!isNumber)
		Misc::throwStdErr("CitcomSRegionalASCIIFile::load: no time step index provided");
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
			
			/* Check if it's the special-case velo two-variable file (bleargh): */
			bool isVeloFile=strcasecmp(argIt->c_str(),"velo")==0;
			if(isVeloFile||nextVector)
				{
				/* Add another vector variable to the data value: */
				int vectorVariableIndex=dataValue.getNumVectorVariables();
				dataValue.addVectorVariable(argIt->c_str());
				std::cout<<"Reading vector variable "<<*argIt<<"...   0%"<<std::flush;
				
				/* Add seven new slices to the data set (3 components spherical and Cartesian each plus Cartesian magnitude): */
				static const char* componentNames[6]={"Colatitude","Longitude","Radius","X","Y","Z"};
				char variableName[256];
				for(int i=0;i<6;++i)
					{
					/* Add a component scalar variable to the data value: */
					snprintf(variableName,sizeof(variableName),"%s %s",argIt->c_str(),componentNames[i]);
					dataValue.addScalarVariable(variableName);
					dataSet.addSlice();
					}
				
				/* Set the vector variables' component indices: */
				for(int i=0;i<3;++i)
					dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,sliceIndex+3+i);
				
				/* Add a magnitude scalar variable to the data value: */
				snprintf(variableName,sizeof(variableName),"%s Magnitude",argIt->c_str());
				dataValue.addScalarVariable(variableName);
				dataSet.addSlice();
				
				/* Test for the special case of the velo file: */
				if(isVeloFile)
					{
					/* Add another scalar variable to the data value: */
					if(logNextScalar)
						{
						char variableName[256];
						snprintf(variableName,sizeof(variableName),"log(%s)","temp");
						dataValue.addScalarVariable(variableName);
						}
					else
						dataValue.addScalarVariable("temp");
					dataSet.addSlice();
					}
				}
			else
				{
				/* Add another scalar variable to the data value: */
				if(logNextScalar)
					{
					char variableName[256];
					snprintf(variableName,sizeof(variableName),"log(%s)",argIt->c_str());
					dataValue.addScalarVariable(variableName);
					std::cout<<"Reading scalar variable "<<variableName<<"...   0%"<<std::flush;
					}
				else
					{
					dataValue.addScalarVariable(argIt->c_str());
					std::cout<<"Reading scalar variable "<<*argIt<<"...   0%"<<std::flush;
					}
				dataSet.addSlice();
				}
			
			/* Read data files for all CPUs: */
			cpuCounter=0;
			DS::GridArray& grid=dataSet.getGrid();
			for(DS::Index cpuIndex(0);cpuIndex[0]<numCpus[0];cpuIndex.preInc(numCpus),++cpuCounter)
				{
				/* Open the CPU's data value file: */
				char dataValueFileName[1024];
				int cpuLinearIndex=(cpuIndex[1]*numCpus[0]+cpuIndex[0])*numCpus[2]+cpuIndex[2];
				snprintf(dataValueFileName,sizeof(dataValueFileName),"%s/%s.%s.%d.%d",dataDir.c_str(),dataFileName.c_str(),argIt->c_str(),cpuLinearIndex,timeStepIndex);
				Misc::File dataValueFile(dataValueFileName,"rt");
				
				char line[256];
				if(isVeloFile)
					{
					/* Read and check the two header lines in the velo file: */
					dataValueFile.gets(line,sizeof(line));
					int dataValueFileTimeStepIndex,dataValueFileNumVertices;
					double dummy1;
					if(sscanf(line,"%d %d %lf",&dataValueFileTimeStepIndex,&dataValueFileNumVertices,&dummy1)!=3)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in data value file %s",dataValueFileName);
					if(dataValueFileNumVertices!=totalCpuNumVertices)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in data value file %s",dataValueFileName);
					dataValueFile.gets(line,sizeof(line));
					int dummy2;
					if(sscanf(line,"%d %d",&dummy2,&dataValueFileNumVertices)!=2)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in data value file %s",dataValueFileName);
					if(dataValueFileNumVertices!=totalCpuNumVertices)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in data value file %s",dataValueFileName);
					}
				else
					{
					/* Read and check the header line: */
					dataValueFile.gets(line,sizeof(line));
					int dummy,dataValueFileNumVertices;
					if(sscanf(line,"%d %d",&dummy,&dataValueFileNumVertices)!=2)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid header line in data value file %s",dataValueFileName);
					if(dataValueFileNumVertices!=totalCpuNumVertices)
						Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Mismatching grid size in data value file %s",dataValueFileName);
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
							/* Read the next line: */
							dataValueFile.gets(line,sizeof(line));
							
							DS::Index index=cpuBaseIndex+gridIndex;
							if(isVeloFile||nextVector)
								{
								/* Read the vector components: */
								double colatitude,longitude,radius,temp;
								if(isVeloFile)
									{
									if(sscanf(line,"%lf %lf %lf %lf",&colatitude,&longitude,&radius,&temp)!=4)
										Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex definition in data value file %s",dataValueFileName);
									}
								else
									{
									if(sscanf(line,"%lf %lf %lf",&colatitude,&longitude,&radius)!=3)
										Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex definition in data value file %s",dataValueFileName);
									}
								
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
									dataSet.getVertexValue(sliceIndex+7,index)=logNextScalar?VScalar(Math::log10(temp)):VScalar(temp);
								}
							else
								{
								/* Read the scalar value: */
								double value;
								if(sscanf(line,"%lf",&value)!=1)
									Misc::throwStdErr("CitcomSRegionalASCIIFile::load: Invalid vertex definition in data value file %s",dataValueFileName);
								
								/* Store the data value: */
								dataSet.getVertexValue(sliceIndex,index)=logNextScalar?VScalar(Math::log10(value)):VScalar(value);
								}
							}
				
				std::cout<<"\b\b\b\b"<<std::setw(3)<<((cpuCounter+1)*100)/numCpus.calcIncrement(-1)<<"%"<<std::flush;
				}
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
