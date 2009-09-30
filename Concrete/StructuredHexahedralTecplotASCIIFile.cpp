/***********************************************************************
StructuredHexahedralTecplotASCIIFile - Class reading structured
hexahedral multi-block Tecplot files in ASCII format.
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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Threads/GzippedFileCharacterSource.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/TecplotASCIIFileHeaderParser.h>

#include <Concrete/StructuredHexahedralTecplotASCIIFile.h>

namespace Visualization {

namespace Concrete {

/*****************************************************
Methods of class StructuredHexahedralTecplotASCIIFile:
*****************************************************/

StructuredHexahedralTecplotASCIIFile::StructuredHexahedralTecplotASCIIFile(void)
	:BaseModule("StructuredHexahedralTecplotASCIIFile")
	{
	}

Visualization::Abstract::DataSet* StructuredHexahedralTecplotASCIIFile::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Parse the arguments: */
	const char* dataFileName=0;
	const char* coordNames[3]={"X","Y","Z"};
	bool flipGrid=false;
	std::vector<std::string> scalarNames;
	std::vector<std::string> vectorNames;
	std::vector<std::string> vectorComponentNames;
	for(std::vector<std::string>::const_iterator argIt=args.begin();argIt!=args.end();++argIt)
		{
		if((*argIt)[0]=='-')
			{
			if(strcasecmp(argIt->c_str()+1,"coords")==0)
				{
				++argIt;
				int i;
				for(i=0;i<3&&argIt!=args.end();++i,++argIt)
					coordNames[i]=argIt->c_str();
				--argIt;
				if(i<3)
					Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing coordinate name on command line");
				}
			else if(strcasecmp(argIt->c_str()+1,"flip")==0)
				flipGrid=true;
			else if(strcasecmp(argIt->c_str()+1,"vector")==0)
				{
				/* Create a vector variable: */
				++argIt;
				if(argIt==args.end())
					Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing vector variable name on command line");
				vectorNames.push_back(*argIt);
				++argIt;
				int i;
				for(i=0;i<3&&argIt!=args.end();++i,++argIt)
					vectorComponentNames.push_back(*argIt);
				--argIt;
				if(i<3)
					Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing vector component name on command line");
				}
			}
		else if(dataFileName==0)
			dataFileName=argIt->c_str();
		else
			{
			/* Create a scalar variable: */
			scalarNames.push_back(*argIt);
			}
		}
	if(dataFileName==0)
		Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: No input file name provided");
	if(scalarNames.empty()&&vectorNames.empty())
		Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: No scalar or vector variables specified");
	
	/* Create a parser and open the input file: */
	Threads::GzippedFileCharacterSource dataFile(dataFileName);
	TecplotASCIIFileHeaderParser parser(dataFile);
	
	/* Create an array of ignore flags for the file's columns: */
	int numVariables=int(parser.getNumVariables());
	bool* ignoreFlags=new bool[numVariables];
	for(int i=0;i<numVariables;++i)
		ignoreFlags[i]=true;
	
	/* Find the column indices of all position components: */
	int posColumnIndices[3];
	for(int i=0;i<3;++i)
		{
		unsigned int variableIndex;
		for(variableIndex=0;variableIndex<parser.getNumVariables();++variableIndex)
			if(strcasecmp(parser.getVariableName(variableIndex).c_str(),coordNames[i])==0)
				break;
		if(variableIndex>=parser.getNumVariables())
			Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing vertex coordinate %s in file %s",coordNames[i],dataFileName);
		
		posColumnIndices[i]=int(variableIndex);
		ignoreFlags[int(variableIndex)]=false;
		}
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Find the column indices of all scalar variables: */
	int numScalars=int(scalarNames.size());
	int* scalarColumnIndices=new int[numScalars];
	int* scalarSliceIndices=new int[numScalars];
	for(int i=0;i<numScalars;++i)
		{
		unsigned int variableIndex;
		for(variableIndex=0;variableIndex<parser.getNumVariables();++variableIndex)
			if(strcasecmp(parser.getVariableName(variableIndex).c_str(),scalarNames[i].c_str())==0)
				break;
		if(variableIndex>=parser.getNumVariables())
			Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing scalar variable %s in file %s",scalarNames[i].c_str(),dataFileName);
		
		scalarColumnIndices[i]=int(variableIndex);
		ignoreFlags[int(variableIndex)]=false;
		
		/* Add a new scalar slice and scalar variable: */
		scalarSliceIndices[i]=dataSet.addSlice();
		dataValue.addScalarVariable(parser.getVariableName(variableIndex).c_str());
		}
	
	/* Find the column indices of all vector variable components: */
	int numVectors=int(vectorNames.size());
	int* vectorColumnIndices=new int[numVectors*3];
	int* vectorSliceIndices=new int[numVectors*4]; // Contains extra magnitude component
	for(int i=0;i<numVectors;++i)
		{
		for(int j=0;j<3;++j)
			{
			unsigned int variableIndex;
			for(variableIndex=0;variableIndex<parser.getNumVariables();++variableIndex)
				if(strcasecmp(parser.getVariableName(variableIndex).c_str(),vectorComponentNames[i*3+j].c_str())==0)
					break;
			if(variableIndex>=parser.getNumVariables())
				Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: Missing vector variable component %s in file %s",vectorComponentNames[i*3+j].c_str(),dataFileName);

			vectorColumnIndices[i*3+j]=int(variableIndex);
			ignoreFlags[int(variableIndex)]=false;
			}
		
		/* Add a vector variable: */
		int vectorVariableIndex=dataValue.addVectorVariable(vectorNames[i].c_str());
		
		/* Add three new scalar slices and scalar variables: */
		for(int j=0;j<3;++j)
			{
			/* Add a new scalar slice and scalar variable: */
			vectorSliceIndices[i*4+j]=dataSet.addSlice();
			std::string componentName=vectorNames[i];
			componentName.push_back(' ');
			componentName.push_back('X'+j);
			dataValue.setVectorVariableScalarIndex(vectorVariableIndex,j,dataValue.addScalarVariable(componentName.c_str()));
			}
		
		/* Add a magnitude slice: */
		vectorSliceIndices[i*4+3]=dataSet.addSlice();
		std::string componentName=vectorNames[i];
		componentName.append(" Magnitude");
		dataValue.setVectorVariableScalarIndex(vectorVariableIndex,3,dataValue.addScalarVariable(componentName.c_str()));
		}
	
	/* Read zones from the file until end-of-file: */
	std::cout<<"Reading input file "<<parser.getTitle()<<std::endl;
	double* columnBuffer=new double[numVariables];
	int zoneIndex=0;
	while(true)
		{
		/* Check for the correct zone type and layout: */
		if(parser.getZoneType()!=TecplotASCIIFileHeaderParser::STRUCTURED)
			Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: File %s has unstructured zones");
		if(parser.getZoneLayout()!=TecplotASCIIFileHeaderParser::INTERLEAVED)
			Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: File %s has unsupported zone layout");
		
		DS::Index numZoneVertices(parser.getZoneSize());
		std::cout<<"Reading grid zone "<<parser.getZoneName()<<" of size "<<numZoneVertices[0]<<" x "<<numZoneVertices[1]<<" x "<<numZoneVertices[2]<<"..."<<std::flush;
		
		/* Add a new grid to the data set: */
		int gridIndex=dataSet.addGrid(numZoneVertices);
		DS::Grid& grid=dataSet.getGrid(gridIndex);
		
		/* Read all grid vertices and scalar values for the zone: */
		int index0Start=0;
		int index0End=numZoneVertices[0];
		int index0Inc=1;
		if(flipGrid)
			{
			index0Start=numZoneVertices[0]-1;
			index0End=-1;
			index0Inc=-1;
			}
		DS::Index index;
		for(index[0]=index0Start;index[0]!=index0End;index[0]+=index0Inc)
			for(index[1]=0;index[1]<numZoneVertices[1];++index[1])
				for(index[2]=0;index[2]<numZoneVertices[2];++index[2])
					{
					/* Parse the line: */
					try
						{
						parser.readDoubles(numVariables,ignoreFlags,columnBuffer);
						parser.skipLine();
						parser.skipWs();
						}
					catch(std::runtime_error err)
						{
						Misc::throwStdErr("StructuredHexahedralTecplotASCIIFile::load: %s while reading zone from file %s",err.what(),dataFileName);
						}
					
					/* Extract and store the vertex position: */
					DS::Point vertexPosition;
					for(int i=0;i<3;++i)
						vertexPosition[i]=Scalar(columnBuffer[posColumnIndices[i]]);
					grid.getVertexPosition(index)=vertexPosition;
					
					/* Extract and store all scalar values: */
					for(int i=0;i<numScalars;++i)
						dataSet.getVertexValue(scalarSliceIndices[i],gridIndex,index)=DS::ValueScalar(columnBuffer[scalarColumnIndices[i]]);
					
					/* Extract and store all vector values: */
					for(int i=0;i<numVectors;++i)
						{
						DataValue::VVector vector;
						for(int j=0;j<3;++j)
							{
							vector[j]=DS::ValueScalar(columnBuffer[vectorColumnIndices[i*3+j]]);
							dataSet.getVertexValue(vectorSliceIndices[i*4+j],gridIndex,index)=vector[j];
							}
						dataSet.getVertexValue(vectorSliceIndices[i*4+3],gridIndex,index)=vector.mag();
						}
					}
		
		std::cout<<" done"<<std::endl;
		
		/* Read the next zone header: */
		if(!parser.readNextZoneHeader())
			break;
		++zoneIndex;
		}
	
	delete[] ignoreFlags;
	delete[] scalarColumnIndices;
	delete[] scalarSliceIndices;
	delete[] vectorColumnIndices;
	delete[] vectorSliceIndices;
	delete[] columnBuffer;
	
	/* Finalize the grid structure: */
	std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	std::cout<<" done"<<std::endl;
	
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
	Visualization::Concrete::StructuredHexahedralTecplotASCIIFile* module=new Visualization::Concrete::StructuredHexahedralTecplotASCIIFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
