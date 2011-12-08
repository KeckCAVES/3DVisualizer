/***********************************************************************
UnstructuredHexahedralTecplotASCIIFile - Class reading unstructured
hexahedral Tecplot files in ASCII format.
Copyright (c) 2009-2011 Oliver Kreylos

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

#include <Concrete/UnstructuredHexahedralTecplotASCIIFile.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <Cluster/MulticastPipe.h>

#include <Concrete/TecplotASCIIFileHeaderParser.h>

namespace Visualization {

namespace Concrete {

/*******************************************************
Methods of class UnstructuredHexahedralTecplotASCIIFile:
*******************************************************/

UnstructuredHexahedralTecplotASCIIFile::UnstructuredHexahedralTecplotASCIIFile(void)
	:BaseModule("UnstructuredHexahedralTecplotASCIIFile")
	{
	}

Visualization::Abstract::DataSet* UnstructuredHexahedralTecplotASCIIFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Parse the arguments: */
	const char* dataFileName=0;
	const char* coordNames[3]={"X","Y","Z"};
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
					Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing coordinate name on command line");
				}
			else if(strcasecmp(argIt->c_str()+1,"vector")==0)
				{
				/* Create a vector variable: */
				++argIt;
				if(argIt==args.end())
					Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing vector variable name on command line");
				vectorNames.push_back(*argIt);
				++argIt;
				int i;
				for(i=0;i<3&&argIt!=args.end();++i,++argIt)
					vectorComponentNames.push_back(*argIt);
				--argIt;
				if(i<3)
					Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing vector component name on command line");
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
		Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: No input file name provided");
	if(scalarNames.empty()&&vectorNames.empty())
		Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: No scalar or vector variables specified");
	
	/* Create a parser and open the input file: */
	TecplotASCIIFileHeaderParser parser(openFile(dataFileName,pipe));
	
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
			Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing vertex coordinate %s in file %s",coordNames[i],dataFileName);
		
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
			Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing scalar variable %s in file %s",scalarNames[i].c_str(),dataFileName);
		
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
				Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: Missing vector variable component %s in file %s",vectorComponentNames[i*3+j].c_str(),dataFileName);

			vectorColumnIndices[i*3+j]=int(variableIndex);
			ignoreFlags[int(variableIndex)]=false;
			}
		
		/* Add a vector variable: */
		int vectorVariableIndex=dataValue.addVectorVariable(vectorNames[i].c_str());
		
		/* Add four new scalar slices and scalar variables (three components plus magnitude): */
		for(int j=0;j<4;++j)
			{
			vectorSliceIndices[i*4+j]=dataSet.addSlice();
			int variableIndex=dataValue.addScalarVariable(makeVectorSliceName(vectorNames[i],j).c_str());
			if(j<3)
				dataValue.setVectorVariableScalarIndex(vectorVariableIndex,j,variableIndex);
			}
		}
	
	/* Read zones from the file until end-of-file: */
	if(master)
		std::cout<<"Reading input file "<<parser.getTitle()<<std::endl;
	double* columnBuffer=new double[numVariables];
	while(true)
		{
		/* Check for the correct zone type and layout: */
		if(parser.getZoneType()!=TecplotASCIIFileHeaderParser::UNSTRUCTURED)
			Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: File %s has structured zones");
		if(parser.getZoneElementType()!=TecplotASCIIFileHeaderParser::HEXAHEDRON)
			Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: File %s has unsupported element type");
		if(parser.getZoneLayout()!=TecplotASCIIFileHeaderParser::INTERLEAVED)
			Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: File %s has unsupported zone layout");
		
		if(master)
			std::cout<<"Reading grid zone "<<parser.getZoneName()<<" with "<<parser.getZoneNumVertices()<<" vertices and "<<parser.getZoneNumElements()<<" cells..."<<std::flush;
		
		/* Prepare the data set for reading: */
		dataSet.reserveVertices(dataSet.getTotalNumVertices()+parser.getZoneNumVertices());
		dataSet.reserveCells(dataSet.getTotalNumCells()+parser.getZoneNumElements());
		
		/* Read all grid vertices and scalar values for the zone: */
		DS::VertexIndex zoneVertexIndexBase=dataSet.getTotalNumVertices();
		for(int i=0;i<parser.getZoneNumVertices();++i)
			{
			/* Parse the line: */
			try
				{
				parser.readDoubles(numVariables,ignoreFlags,columnBuffer);
				}
			catch(std::runtime_error err)
				{
				Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: %s while reading zone vertices from file %s",err.what(),dataFileName);
				}
			
			/* Extract the vertex position and add a vertex: */
			DS::Point vertexPosition;
			for(int i=0;i<3;++i)
				vertexPosition[i]=Scalar(columnBuffer[posColumnIndices[i]]);
			DS::VertexIndex vertexIndex=dataSet.addVertex(vertexPosition).getIndex();
			
			/* Extract and store all scalar values: */
			for(int i=0;i<numScalars;++i)
				dataSet.setVertexValue(scalarSliceIndices[i],vertexIndex,DS::ValueScalar(columnBuffer[scalarColumnIndices[i]]));
			
			/* Extract and store all vector values: */
			for(int i=0;i<numVectors;++i)
				{
				DataValue::VVector vector;
				for(int j=0;j<3;++j)
					{
					vector[j]=DS::ValueScalar(columnBuffer[vectorColumnIndices[i*3+j]]);
					dataSet.setVertexValue(vectorSliceIndices[i*4+j],vertexIndex,vector[j]);
					}
				dataSet.setVertexValue(vectorSliceIndices[i*4+3],vertexIndex,vector.mag());
				}
			}
		
		/* Read all grid cells for the zone: */
		for(int i=0;i<parser.getZoneNumElements();++i)
			{
			/* Parse the line: */
			int indexBuffer[8]={-1,-1,-1,-1,-1,-1,-1,-1};
			try
				{
				for(int i=0;i<8;++i)
					indexBuffer[i]=parser.readInteger();
				}
			catch(std::runtime_error err)
				{
				Misc::throwStdErr("UnstructuredHexahedralTecplotASCIIFile::load: %s while reading zone cells from file %s",err.what(),dataFileName);
				}
			
			/* Read and unswizzle the cell vertex indices: */
			static const int vertexOrder[8]={0,1,3,2,4,5,7,6}; // Tecplot's cube vertex counting order
			DS::VertexID cellVertices[8];
			for(int i=0;i<8;++i)
				cellVertices[vertexOrder[i]]=DS::VertexID(zoneVertexIndexBase+(indexBuffer[i]-1));
			
			/* Add the cell to the data set: */
			dataSet.addCell(cellVertices);
			}
		if(master)
			std::cout<<" done"<<std::endl;
		
		/* Read the next zone header: */
		if(!parser.readNextZoneHeader())
			break;
		}
	
	delete[] ignoreFlags;
	delete[] scalarColumnIndices;
	delete[] scalarSliceIndices;
	delete[] vectorColumnIndices;
	delete[] vectorSliceIndices;
	delete[] columnBuffer;
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
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
	Visualization::Concrete::UnstructuredHexahedralTecplotASCIIFile* module=new Visualization::Concrete::UnstructuredHexahedralTecplotASCIIFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
