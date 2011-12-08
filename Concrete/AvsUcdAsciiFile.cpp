/***********************************************************************
AvsUcdAsciiFile - Class reading AVS Unstructured Cell Data files in ASCII
format.
Copyright (c) 2011 Oliver Kreylos

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

#include <Concrete/AvsUcdAsciiFile.h>

#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>

namespace Visualization {

namespace Concrete {

/********************************
Methods of class AvsUcdAsciiFile:
********************************/

AvsUcdAsciiFile::AvsUcdAsciiFile(void)
	:BaseModule("AvsUcdAsciiFile")
	{
	}

Visualization::Abstract::DataSet* AvsUcdAsciiFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Open the input file: */
	if(args.empty())
		Misc::throwStdErr("AvsUcdAsciiFile::load: No input file name provided");
	IO::ValueSource data(openFile(args[0],pipe));
	data.setPunctuation("#,\n");
	data.skipWs();
	
	/* Skip comment lines: */
	while(!data.eof()&&data.peekc()=='#')
		{
		data.skipLine();
		data.skipWs();
		}
	
	/* Read the file header: */
	unsigned int numNodes=data.readUnsignedInteger();
	unsigned int numCells=data.readUnsignedInteger();
	unsigned int numDataPerNode=data.readUnsignedInteger();
	unsigned int numDataPerCell=data.readUnsignedInteger();
	unsigned int numDataPerModel=data.readUnsignedInteger();
	data.skipLine();
	data.skipWs();
	
	/* Read the node positions: */
	if(master)
		std::cout<<"Reading "<<numNodes<<" nodes..."<<std::flush;
	dataSet.reserveVertices(numNodes);
	std::vector<std::pair<DS::VertexIndex,unsigned int> > nodeIndexMapper;
	unsigned int lastNodeId=0;
	for(unsigned int ni=0;ni<numNodes;++ni)
		{
		/* Read the next node: */
		unsigned int nodeId=data.readUnsignedInteger();
		DS::Point pos;
		for(int i=0;i<3;++i)
			pos[i]=Scalar(data.readNumber());
		data.skipLine();
		data.skipWs();
		
		/* Add the node to the data set: */
		DS::VertexIndex nodeIndex=dataSet.addVertex(pos).getIndex();
		
		/* Check for gaps in the node ID sequence: */
		if(ni==0||nodeId!=lastNodeId+1)
			{
			if(nodeId<lastNodeId+1)
				Misc::throwStdErr("AvsUcdAsciiFile::load: Non-monotonic node ID sequence in file %s",args[0].c_str());
			nodeIndexMapper.push_back(std::pair<unsigned int,unsigned int>(nodeIndex,nodeId));
			}
		lastNodeId=nodeId;
		}
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Read all cells: */
	if(master)
		std::cout<<"Reading "<<numCells<<" cells..."<<std::flush;
	dataSet.reserveCells(numCells);
	// static const unsigned int vertexOrder[8]={4,5,7,6,0,1,3,2}; // Permutation to convert AVS hexahedron vertex order to SlicedHypercubic hexahedron vertex order
	static const unsigned int vertexOrder[8]={0,1,3,2,4,5,7,6}; // Permutation to convert AVS hexahedron vertex order to SlicedHypercubic hexahedron vertex order
	for(unsigned int ci=0;ci<numCells;++ci)
		{
		/* Read the next cell: */
		data.readUnsignedInteger(); // Skip cell ID
		data.readUnsignedInteger(); // Skip cell material
		
		/* Ignore everything besides hexahedral cells: */
		if(data.isLiteral("hex"))
			{
			/* Read the hexahedron's corner vertices: */
			DS::VertexID cellVertices[8];
			for(int i=0;i<8;++i)
				{
				unsigned int vertexId=data.readUnsignedInteger();
				
				/* Convert the vertex ID into a vertex index by searching for the continous sequence of node IDs containing the vertex ID: */
				unsigned int l=0;
				unsigned int r=nodeIndexMapper.size();
				while(r-l>1)
					{
					unsigned int m=(l+r)>>1;
					if(vertexId<nodeIndexMapper[m].second)
						r=m;
					else
						l=m;
					}
				cellVertices[vertexOrder[i]]=DS::VertexID(nodeIndexMapper[l].first+(vertexId-nodeIndexMapper[l].second));
				}
			
			/* Add the cell: */
			dataSet.addCell(cellVertices);
			}
		
		data.skipLine();
		data.skipWs();
		}
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Finalize the data set's grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	if(numDataPerNode>0)
		{
		/* Read the node data header: */
		unsigned int numNodeDataElements=data.readUnsignedInteger();
		std::vector<unsigned int> numNodeDataComponents;
		numNodeDataComponents.reserve(numNodeDataElements);
		for(unsigned int i=0;i<numNodeDataElements;++i)
			numNodeDataComponents.push_back(data.readUnsignedInteger());
		
		/* Add scalar and vector variables to the data set: */
		for(unsigned int elementIndex=0;elementIndex<numNodeDataElements;++elementIndex)
			{
			if(numNodeDataComponents[elementIndex]==1)
				{
				/* Add a scalar variable: */
				std::string variableName=data.readString();
				data.skipLine();
				data.skipWs();
				dataValue.addScalarVariable(variableName.c_str());
				dataSet.addSlice();
				}
			else if(numNodeDataComponents[elementIndex]==3)
				{
				/* Add a vector variable: */
				std::string variableName=data.readString();
				data.skipLine();
				data.skipWs();
				int variableIndex=dataValue.addVectorVariable(variableName.c_str());
				
				/* Add four new scalar slices and scalar variables (three vector components plus magnitude): */
				for(int i=0;i<4;++i)
					{
					std::string componentName=variableName;
					componentName.push_back(' ');
					if(i<3)
						componentName.push_back('X'+i);
					else
						componentName.append("Magnitude");
					int componentIndex=dataValue.addScalarVariable(componentName.c_str());
					if(i<3)
						dataValue.setVectorVariableScalarIndex(variableIndex,i,componentIndex);
					dataSet.addSlice();
					}
				}
			}
			
		/* Read the node data values: */
		if(master)
			std::cout<<"Reading "<<numNodes<<" node data values..."<<std::flush;
		for(unsigned int ni=0;ni<numNodes;++ni)
			{
			/* Read the node ID: */
			unsigned int nodeId=data.readUnsignedInteger();
			
			/* Convert the node ID into a vertex index by searching for the continous sequence of node IDs containing the vertex ID: */
			unsigned int l=0;
			unsigned int r=nodeIndexMapper.size();
			while(r-l>1)
				{
				unsigned int m=(l+r)>>1;
				if(nodeId<nodeIndexMapper[m].second)
					r=m;
				else
					l=m;
				}
			DS::VertexIndex vertexIndex=DS::VertexIndex(nodeIndexMapper[l].first+(nodeId-nodeIndexMapper[l].second));
			
			/* Read all data value elements: */
			unsigned int sliceIndex=0;
			for(unsigned int elementIndex=0;elementIndex<numNodeDataElements;++elementIndex)
				{
				if(numNodeDataComponents[elementIndex]==1)
					{
					dataSet.setVertexValue(sliceIndex,vertexIndex,DS::ValueScalar(data.readNumber()));
					++sliceIndex;
					}
				else if(numNodeDataComponents[elementIndex]==3)
					{
					DataValue::VVector vector;
					for(unsigned int i=0;i<3;++i)
						{
						vector[i]=DS::ValueScalar(data.readNumber());
						dataSet.setVertexValue(sliceIndex,vertexIndex,vector[i]);
						++sliceIndex;
						}
					dataSet.setVertexValue(sliceIndex,vertexIndex,vector.mag());
					++sliceIndex;
					}
				else
					{
					/* Skip the data element: */
					for(unsigned int i=0;i<numNodeDataComponents[elementIndex];++i)
						data.readNumber();
					}
				}
			data.skipLine();
			data.skipWs();
			}
		if(master)
			std::cout<<" done"<<std::endl;
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
	Visualization::Concrete::AvsUcdAsciiFile* module=new Visualization::Concrete::AvsUcdAsciiFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
