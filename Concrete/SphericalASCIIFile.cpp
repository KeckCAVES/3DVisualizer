/***********************************************************************
SphericalASCIIFile - Class to read multivariate scalar data in spherical
coordinates from simple ASCII files.
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

#include <Concrete/SphericalASCIIFile.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Plugins/FactoryManager.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>

namespace Visualization {

namespace Concrete {

namespace {

/**************
Helper classes:
**************/

struct ScalarVariable
	{
	/* Elements: */
	public:
	std::string name;
	bool log;
	int columnIndex;
	int sliceIndex;
	};

struct VectorVariable
	{
	/* Elements: */
	public:
	std::string name;
	int columnIndices[3];
	int baseSliceIndex;
	};

}

/***********************************
Methods of class SphericalASCIIFile:
***********************************/

SphericalASCIIFile::SphericalASCIIFile(void)
	:BaseModule("SphericalASCIIFile")
	{
	}

Visualization::Abstract::DataSet* SphericalASCIIFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Parse the command line: */
	const char* dataFileName=0;
	int numHeaderLines=0;
	DS::Index numVertices(0,0,0); // Order is radius, (co)latitude, longitude
	int nodeCountOrder[3]={0,1,2};
	bool flip=false;
	int coordColumnIndices[3]={-1,-1,-1};
	bool coordColatitude=false;
	bool coordDegrees=false;
	double radiusScale=6378.14e3; // Equatorial radius in m
	std::vector<ScalarVariable> scalars;
	std::vector<VectorVariable> vectors;
	bool storeSphericals=false;
	int sphericalBaseIndex=-1;
	int maxColumnIndex=-1;
	int numDataSlices=0;
	for(std::vector<std::string>::const_iterator argIt=args.begin();argIt!=args.end();++argIt)
		{
		if((*argIt)[0]=='-')
			{
			if(strcasecmp(argIt->c_str()+1,"headers")==0)
				{
				/* Read the number of header lines at the beginning of the file: */
				++argIt;
				numHeaderLines=atoi(argIt->c_str());
				}
			else if(strcasecmp(argIt->c_str()+1,"nodes")==0)
				{
				/* Read the number of nodes in the data set: */
				for(int i=0;i<3;++i)
					{
					++argIt;
					numVertices[i]=atoi(argIt->c_str());
					}
				}
			else if(strcasecmp(argIt->c_str()+1,"nodecount")==0)
				{
				/* Read the node counting order: */
				for(int i=0;i<3;++i)
					{
					++argIt;
					int speed=atoi(argIt->c_str());
					if(speed<0||speed>=3)
						Misc::throwStdErr("SphericalASCIIFile::load: Invalid node counting speed %d specified, must be 0, 1, or 2",speed);
					nodeCountOrder[speed]=i;
					}
				}
			else if(strcasecmp(argIt->c_str()+1,"flip")==0)
				flip=true;
			else if(strcasecmp(argIt->c_str()+1,"coords")==0)
				{
				/* Read the coordinate column indices: */
				for(int i=0;i<3;++i)
					{
					++argIt;
					coordColumnIndices[2-i]=atoi(argIt->c_str());
					if(maxColumnIndex<coordColumnIndices[2-i])
						maxColumnIndex=coordColumnIndices[2-i];
					}
				}
			else if(strcasecmp(argIt->c_str()+1,"colat")==0)
				{
				/* Coordinates use colatitude: */
				coordColatitude=true;
				}
			else if(strcasecmp(argIt->c_str()+1,"degree")==0)
				{
				/* Coordinates use degrees: */
				coordDegrees=true;
				}
			else if(strcasecmp(argIt->c_str()+1,"radius")==0)
				{
				/* Read the radius scaling factor: */
				++argIt;
				radiusScale=atof(argIt->c_str());
				}
			else if(strcasecmp(argIt->c_str()+1,"storeCoords")==0)
				storeSphericals=true;
			else if(strcasecmp(argIt->c_str()+1,"scalar")==0)
				{
				/* Parse a scalar variable: */
				ScalarVariable scalar;
				++argIt;
				scalar.name=*argIt;
				scalar.log=false;
				++argIt;
				scalar.columnIndex=atoi(argIt->c_str());
				if(maxColumnIndex<scalar.columnIndex)
					maxColumnIndex=scalar.columnIndex;
				scalar.sliceIndex=numDataSlices;
				++numDataSlices;
				scalars.push_back(scalar);
				}
			else if(strcasecmp(argIt->c_str()+1,"vector")==0)
				{
				/* Parse a vector variable: */
				VectorVariable vector;
				++argIt;
				vector.name=*argIt;
				for(int i=0;i<3;++i)
					{
					++argIt;
					vector.columnIndices[i]=atoi(argIt->c_str());
					if(maxColumnIndex<vector.columnIndices[i])
						maxColumnIndex=vector.columnIndices[i];
					}
				vector.baseSliceIndex=numDataSlices;
				numDataSlices+=7;
				vectors.push_back(vector);
				}
			}
		else if(dataFileName==0)
			dataFileName=argIt->c_str();
		}
	if(dataFileName==0)
		Misc::throwStdErr("SphericalASCIIFile::load: No data file name provided");
	if(numVertices.calcIncrement(-1)==0)
		Misc::throwStdErr("SphericalASCIIFile::load: No number of nodes provided");
	if(numDataSlices==0)
		Misc::throwStdErr("SphericalASCIIFile::load: No scalar or vector data values specified");
	
	/* Open the data file: */
	IO::ValueSource reader(openFile(dataFileName,pipe));
	reader.setPunctuation('\n',true);
	
	/* Skip the data file header: */
	for(int i=0;i<numHeaderLines;++i)
		reader.skipLine();
	reader.skipWs();
	
	/* Create and initialize the result data set: */
	Misc::SelfDestructPointer<EarthDataSet<DataSet> > result(new EarthDataSet<DataSet>(args));
	result->setFlatteningFactor(0.0);
	result->getSphericalCoordinateTransformer()->setColatitude(coordColatitude);
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	for(int i=0;i<numDataSlices;++i)
		dataSet.addSlice();
	if(storeSphericals)
		{
		sphericalBaseIndex=dataSet.getNumSlices();
		for(int i=0;i<3;++i)
			dataSet.addSlice();
		}
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,vectors.size());
	for(std::vector<ScalarVariable>::const_iterator sIt=scalars.begin();sIt!=scalars.end();++sIt)
		dataValue.setScalarVariableName(sIt->sliceIndex,sIt->name.c_str());
	if(storeSphericals)
		{
		dataValue.setScalarVariableName(sphericalBaseIndex+0,"Longitude");
		dataValue.setScalarVariableName(sphericalBaseIndex+1,coordColatitude?"Colatitude":"Latitude");
		dataValue.setScalarVariableName(sphericalBaseIndex+2,"Radius");
		}
	int vectorVariableIndex=0;
	for(std::vector<VectorVariable>::const_iterator vIt=vectors.begin();vIt!=vectors.end();++vIt,++vectorVariableIndex)
		{
		static const char* vectorComponentNames[7]={" Longitude"," Latitude"," Radius"," X"," Y"," Z"," Magnitude"};
		for(int i=0;i<7;++i)
			{
			if(i==1)
				dataValue.setScalarVariableName(vIt->baseSliceIndex+i,(vIt->name+(coordColatitude?" Colatitude":" Latitude")).c_str());
			else
				dataValue.setScalarVariableName(vIt->baseSliceIndex+i,(vIt->name+vectorComponentNames[i]).c_str());
			}
		dataValue.setVectorVariableName(vectorVariableIndex,vIt->name.c_str());
		for(int i=0;i<3;++i)
			dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,vIt->baseSliceIndex+3+i);
		}
	
	/* Prepare the spherical-to-Cartesian formula: */
	// const double a=6378.14e3; // Equatorial radius in m
	// const double f=1.0/298.247; // Geoid flattening factor (not used, since there could be vector values)
	const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
	
	/* Allocate an array to parse the file's data columns: */
	double* columns=new double[maxColumnIndex+1];
	
	/* Read all node positions and values: */
	if(master)
		std::cout<<"Reading grid vertex positions and values...   0%"<<std::flush;
	DS::GridArray& grid=dataSet.getGrid();
	DS::Index index;
	int index0Min,index0Max,index0Increment;
	if(flip)
		{
		index0Min=numVertices[nodeCountOrder[0]]-1;
		index0Max=-1;
		index0Increment=-1;
		}
	else
		{
		index0Min=0;
		index0Max=numVertices[nodeCountOrder[0]];
		index0Increment=1;
		}
	unsigned int lineNumber=numHeaderLines+1;
	for(index[nodeCountOrder[0]]=index0Min;index[nodeCountOrder[0]]!=index0Max;index[nodeCountOrder[0]]+=index0Increment)
		{
		for(index[nodeCountOrder[1]]=0;index[nodeCountOrder[1]]<numVertices[nodeCountOrder[1]];++index[nodeCountOrder[1]])
			for(index[nodeCountOrder[2]]=0;index[nodeCountOrder[2]]<numVertices[nodeCountOrder[2]];++index[nodeCountOrder[2]])
				{
				try
					{
					/* Read all relevant columns from the next line: */
					for(int i=0;i<=maxColumnIndex;++i)
						columns[i]=reader.readNumber();
					reader.skipLine();
					reader.skipWs();
					++lineNumber;
					}
				catch(IO::ValueSource::NumberError err)
					{
					Misc::throwStdErr("SphericalASCIIFile::load: Number format error in line %u",lineNumber);
					}
				
				/* Get the vertex' linear index: */
				int linearIndex=grid.calcLinearIndex(index);
				
				/* Calculate the vertex' position: */
				double longitude=columns[coordColumnIndices[0]];
				double latitude=columns[coordColumnIndices[1]];
				double radius=columns[coordColumnIndices[2]];
				if(coordDegrees)
					{
					longitude=Math::rad(longitude);
					latitude=Math::rad(latitude);
					}
				if(coordColatitude)
					latitude=Math::rad(90.0)-latitude;
				double s0=Math::sin(latitude);
				double c0=Math::cos(latitude);
				double s1=Math::sin(longitude);
				double c1=Math::cos(longitude);
				double r=radius*radiusScale*scaleFactor;
				double xy=r*c0;
				DS::Point& vertex=grid.getArray()[linearIndex];
				vertex[0]=Scalar(xy*c1);
				vertex[1]=Scalar(xy*s1);
				vertex[2]=Scalar(r*s0);
				
				/* Store the vertex' scalar values: */
				for(std::vector<ScalarVariable>::const_iterator sIt=scalars.begin();sIt!=scalars.end();++sIt)
					dataSet.getSliceArray(sIt->sliceIndex)[linearIndex]=sIt->log?VScalar(Math::log10(columns[sIt->columnIndex])):VScalar(columns[sIt->columnIndex]);
				
				if(storeSphericals)
					{
					/* Store the spherical coordinates: */
					dataSet.getSliceArray(sphericalBaseIndex+0)[linearIndex]=VScalar(columns[coordColumnIndices[0]]);
					dataSet.getSliceArray(sphericalBaseIndex+1)[linearIndex]=VScalar(columns[coordColumnIndices[1]]);
					dataSet.getSliceArray(sphericalBaseIndex+2)[linearIndex]=VScalar(columns[coordColumnIndices[2]]);
					}
				
				/* Store the vertex' vector values: */
				for(std::vector<VectorVariable>::const_iterator vIt=vectors.begin();vIt!=vectors.end();++vIt)
					{
					/* Store the spherical vector components: */
					double vlongitude=columns[vIt->columnIndices[0]];
					double vlatitude=columns[vIt->columnIndices[1]];
					double vradius=columns[vIt->columnIndices[2]];
					dataSet.getSliceArray(vIt->baseSliceIndex+0)[linearIndex]=VScalar(vlongitude);
					dataSet.getSliceArray(vIt->baseSliceIndex+1)[linearIndex]=VScalar(vlatitude);
					dataSet.getSliceArray(vIt->baseSliceIndex+2)[linearIndex]=VScalar(vradius);
					
					/* Convert the vector to Cartesian coordinates: */
					if(coordDegrees)
						{
						vlongitude=Math::rad(vlongitude);
						vlatitude=Math::rad(vlatitude);
						}
					if(coordColatitude)
						vlatitude=-vlatitude;
					DataValue::VVector vector;
					vector[0]=VScalar(c1*(c0*vradius-s0*vlatitude)-s1*vlongitude);
					vector[1]=VScalar(s1*(c0*vradius-s0*vlatitude)+c1*vlongitude);
					vector[2]=VScalar(s0*vradius+c0*vlatitude);
					for(int i=0;i<3;++i)
						dataSet.getSliceArray(vIt->baseSliceIndex+3+i)[linearIndex]=vector[i];
					dataSet.getSliceArray(vIt->baseSliceIndex+6)[linearIndex]=VScalar(Geometry::mag(vector));
					}
				}
		
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[nodeCountOrder[0]]+1)*100)/numVertices[nodeCountOrder[0]]<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	delete[] columns;
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Return the result data set: */
	return result.releaseTarget();
	}

Visualization::Abstract::DataSetRenderer* SphericalASCIIFile::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
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
	Visualization::Concrete::SphericalASCIIFile* module=new Visualization::Concrete::SphericalASCIIFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
