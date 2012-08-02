/***********************************************************************
StructuredGridASCII - Class defining lowest-common-denominator ASCII
file format for curvilinear grids in Cartesian or spherical coordinates.
Vertex positions and attributes are stored in separate files.
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

#include <Concrete/StructuredGridASCII.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>

namespace Visualization {

namespace Concrete {

/************************************
Methods of class StructuredGridASCII:
************************************/

StructuredGridASCII::StructuredGridASCII(void)
	:BaseModule("StructuredGridASCII")
	{
	}

Visualization::Abstract::DataSet* StructuredGridASCII::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	
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
	
	/* Open the grid definition file: */
	if(master)
		std::cout<<"Reading grid file "<<*argIt<<"..."<<std::flush;
	IO::ValueSource gridReader(openFile(*argIt,pipe));
	gridReader.setPunctuation("#\n");
	gridReader.skipWs();
	
	/* Parse the grid file header: */
	DS::Index numVertices(-1,-1,-1);
	bool sphericalCoordinates=false;
	unsigned int lineIndex=1;
	int parsedHeaderLines=0;
	while(parsedHeaderLines<2)
		{
		/* Check if the file is already over: */
		if(gridReader.eof())
			Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in grid file %s",argIt->c_str());
		
		/* Read the next token: */
		std::string token=gridReader.readString();
		if(token=="gridSize")
			{
			/* Read the grid size: */
			try
				{
				for(int i=0;i<3;++i)
					numVertices[i]=gridReader.readInteger();
				}
			catch(IO::ValueSource::NumberError err)
				{
				Misc::throwStdErr("StructuredGridASCII::load: Invalid grid size in line %u in grid file %s",lineIndex,argIt->c_str());
				}
			
			++parsedHeaderLines;
			}
		else if(token=="coordinate")
			{
			/* Read the coordinate mode: */
			std::string coordinateMode=gridReader.readString();
			if(coordinateMode=="Cartesian")
				sphericalCoordinates=false;
			else if(coordinateMode=="Spherical")
				sphericalCoordinates=true;
			else
				Misc::throwStdErr("StructuredGridASCII::load: invalid coordinate mode %s in line %u in grid file %s",coordinateMode.c_str(),lineIndex,argIt->c_str());
			
			++parsedHeaderLines;
			}
		else if(token!="#")
			Misc::throwStdErr("StructuredGridASCII::load: Unknown header token %s in line %u in grid file %s",token.c_str(),lineIndex,argIt->c_str());
		
		/* Go to the next line: */
		gridReader.skipLine();
		gridReader.skipWs();
		++lineIndex;
		}
	
	/* Initialize the data set: */
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	if(sphericalCoordinates&&storeSphericals)
		{
		/* Add three slices to the data set: */
		static const char* coordSliceNames[3]={"Latitude","Longitude","Radius"};
		for(int i=0;i<3;++i)
			{
			dataSet.addSlice();
			dataValue.addScalarVariable(coordSliceNames[i]);
			}
		}
	
	/* Prepare the spherical-to-Cartesian formula: */
	// const double a=6378.14e3; // Equatorial radius in m
	// const double f=1.0/298.247; // Geoid flattening factor (not used, since there could be vector values)
	const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
	
	/* Read all vertex positions: */
	if(master)
		std::cout<<"   0%"<<std::flush;
	DS::Index index(0);
	while(index[2]<numVertices[2])
		{
		/* Check if the file is already over: */
		if(gridReader.eof())
			Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in grid file %s",argIt->c_str());
		
		/* Check for empty or comment lines: */
		if(gridReader.peekc()!='\n'&&gridReader.peekc()!='#')
			{
			/* Parse the line: */
			DS::Point& vertex=dataSet.getVertexPosition(index);
			if(sphericalCoordinates)
				{
				try
					{
					/* Read the vertex' position in spherical coordinates: */
					double longitude=gridReader.readNumber();
					double latitude=gridReader.readNumber();
					double radius=gridReader.readNumber();
					
					/* Convert the vertex position to Cartesian coordinates: */
					double s0=Math::sin(latitude);
					double c0=Math::cos(latitude);
					double r=radius*scaleFactor;
					double xy=r*c0;
					double s1=Math::sin(longitude);
					double c1=Math::cos(longitude);
					vertex[0]=Scalar(xy*c1);
					vertex[1]=Scalar(xy*s1);
					vertex[2]=Scalar(r*s0);
					
					if(storeSphericals)
						{
						/* Store the spherical coordinate components in the first three value slices: */
						dataSet.getVertexValue(0,index)=Scalar(Math::deg(latitude));
						dataSet.getVertexValue(1,index)=Scalar(Math::deg(longitude));
						dataSet.getVertexValue(2,index)=Scalar(r);
						}
					}
				catch(IO::ValueSource::NumberError err)
					{
					Misc::throwStdErr("StructuredGridASCII::load: Invalid spherical vertex coordinate in line %u in grid file %s",lineIndex,argIt->c_str());
					}
				}
			else
				{
				try
					{
					for(int i=0;i<3;++i)
						vertex[i]=DS::Scalar(gridReader.readNumber());
					}
				catch(IO::ValueSource::NumberError err)
					{
					Misc::throwStdErr("StructuredGridASCII::load: Invalid Cartesian vertex coordinate in line %u in grid file %s",lineIndex,argIt->c_str());
					}
				}
			
			/* Go to the next vertex: */
			int incDim;
			for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
				index[incDim]=0;
			++index[incDim];
			if(incDim==2&&master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
			}
		
		/* Go to the next line: */
		gridReader.skipLine();
		gridReader.skipWs();
		++lineIndex;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Read all vertex attribute files given on the command line: */
	bool logNextScalar=false;
	for(++argIt;argIt!=args.end();++argIt)
		{
		if(strcasecmp(argIt->c_str(),"-log")==0)
			logNextScalar=true;
		else
			{
			/* Open the slice file: */
			if(master)
				std::cout<<"Reading slice file "<<*argIt<<"..."<<std::flush;
			IO::ValueSource sliceReader(openFile(*argIt,pipe));
			sliceReader.setPunctuation("#\n");
			sliceReader.skipWs();
			
			/* Parse the slice file header: */
			bool vectorValue=false;
			int sliceIndex=dataSet.getNumSlices();
			lineIndex=0;
			parsedHeaderLines=0;
			while(parsedHeaderLines<2)
				{
				/* Check if the file is already over: */
				if(sliceReader.eof())
					Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in slice file %s",argIt->c_str());
				
				/* Read the next token: */
				std::string token=sliceReader.readString();
				if(token=="gridSize")
					{
					/* Read the grid size: */
					DS::Index sliceNumVertices(0);
					try
						{
						for(int i=0;i<3;++i)
							sliceNumVertices[i]=sliceReader.readInteger();
						}
					catch(IO::ValueSource::NumberError err)
						{
						Misc::throwStdErr("StructuredGridASCII::load: Invalid grid size in line %u in grid file %s",lineIndex,argIt->c_str());
						}
					
					/* Check if the slice grid size matches the grid size: */
					if(sliceNumVertices!=numVertices)
						Misc::throwStdErr("StructuredGridASCII::load: mismatching grid size in slice file %s",argIt->c_str());
					
					++parsedHeaderLines;
					}
				else if(token=="scalar")
					{
					/* Read the scalar value's name: */
					std::string scalarName=sliceReader.readString();
					if(!scalarName.empty()&&scalarName!="\n")
						{
						/* Add another slice to the data set: */
						dataSet.addSlice();
						
						/* Add another scalar variable to the data value: */
						if(logNextScalar)
							scalarName=std::string("log(")+scalarName+")";
						dataValue.addScalarVariable(scalarName.c_str());
						}
					else
						Misc::throwStdErr("StructuredGridASCII::load: empty scalar variable name in line %u in slice file %s",lineIndex,argIt->c_str());
					
					++parsedHeaderLines;
					}
				else if(token=="vector")
					{
					/* Read the vector value's name: */
					std::string vectorName=sliceReader.readString();
					if(!vectorName.empty()&&vectorName!="\n")
						{
						/* Add another vector variable to the data value: */
						int vectorVariableIndex=dataValue.addVectorVariable(vectorName.c_str());
						
						/* Add four new slices to the data set (three components plus magnitude): */
						for(int i=0;i<4;++i)
							{
							dataSet.addSlice();
							int variableIndex=dataValue.addScalarVariable(makeVectorSliceName(vectorName,i).c_str());
							if(i<3)
								dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,variableIndex);
							}
						}
					else
						Misc::throwStdErr("StructuredGridASCII::load: empty vector variable name in line %u in slice file %s",lineIndex,argIt->c_str());
					
					++parsedHeaderLines;
					}
				else if(token!="#")
					Misc::throwStdErr("StructuredGridASCII::load: Unknown header token %s in line %u in grid file %s",token.c_str(),lineIndex,argIt->c_str());
				
				/* Go to the next line: */
				sliceReader.skipLine();
				sliceReader.skipWs();
				++lineIndex;
				}
			
			/* Read all vertex attributes: */
			if(master)
				std::cout<<"   0%"<<std::flush;
			DS::Index index(0);
			while(index[2]<numVertices[2])
				{
				/* Check if the file is already over: */
				if(sliceReader.eof())
					Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in slice file %s",argIt->c_str());
				
				/* Check for empty or comment lines: */
				if(sliceReader.peekc()!='\n'&&sliceReader.peekc()!='#')
					{
					/* Parse the line: */
					if(vectorValue)
						{
						DataValue::VVector vector;
						if(sphericalCoordinates)
							{
							try
								{
								/* Read the vector attribute in spherical coordinates: */
								double longitude=sliceReader.readNumber();
								double latitude=sliceReader.readNumber();
								double radius=sliceReader.readNumber();
								
								/* Convert the vector to Cartesian coordinates: */
								const DS::Point& p=dataSet.getVertexPosition(index);
								double xy=Math::sqr(double(p[0]))+Math::sqr(double(p[1]));
								double r=xy+Math::sqr(double(p[2]));
								xy=Math::sqrt(xy);
								r=Math::sqrt(r);
								double s0=double(p[2])/r;
								double c0=xy/r;
								double s1=double(p[1])/xy;
								double c1=double(p[0])/xy;
								vector[0]=Scalar(c1*(c0*radius-s0*latitude)-s1*longitude);
								vector[1]=Scalar(s1*(c0*radius-s0*latitude)+c1*longitude);
								vector[2]=Scalar(c0*latitude+s0*radius);
								}
							catch(IO::ValueSource::NumberError err)
								{
								Misc::throwStdErr("StructuredGridASCII::load: Invalid spherical vector attribute in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							}
						else
							{
							try
								{
								/* Read the vector attribute in Cartesian coordinates: */
								for(int i=0;i<3;++i)
									vector[i]=DataValue::VVector::Scalar(sliceReader.readNumber());
								}
							catch(IO::ValueSource::NumberError err)
								{
								Misc::throwStdErr("StructuredGridASCII::load: Invalid Cartesian vector attribute in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							}
						
						/* Store the vector's components and magnitude: */
						for(int i=0;i<3;++i)
							dataSet.getVertexValue(sliceIndex+i,index)=vector[i];
						dataSet.getVertexValue(sliceIndex+3,index)=Scalar(Geometry::mag(vector));
						}
					else
						{
						/* Read the scalar attribute: */
						if(logNextScalar)
							{
							try
								{
								double value=sliceReader.readNumber();
								dataSet.getVertexValue(sliceIndex,index)=Scalar(Math::log10(value));
								}
							catch(IO::ValueSource::NumberError err)
								{
								Misc::throwStdErr("StructuredGridASCII::load: Invalid logarithmic scalar vertex attribute in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							}
						else
							{
							try
								{
								dataSet.getVertexValue(sliceIndex,index)=Scalar(sliceReader.readNumber());
								}
							catch(IO::ValueSource::NumberError err)
								{
								Misc::throwStdErr("StructuredGridASCII::load: Invalid scalar vertex attribute in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							}
						}

					/* Go to the next vertex: */
					int incDim;
					for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
						index[incDim]=0;
					++index[incDim];
					if(incDim==2&&master)
						std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
					}
				
				/* Go to the next line: */
				sliceReader.skipLine();
				sliceReader.skipWs();
				++lineIndex;
				}
			if(master)
				std::cout<<"\b\b\b\bdone"<<std::endl;
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
	Visualization::Concrete::StructuredGridASCII* module=new Visualization::Concrete::StructuredGridASCII();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
