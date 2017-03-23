/***********************************************************************
UnstructuredHexahedralVTK - Class reading unstructured hexahedral data
sets from files in legacy VTK format.
Copyright (c) 2016 Oliver Kreylos

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

#include <Concrete/UnstructuredHexahedralVTK.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <Misc/SizedTypes.h>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Plugins/FactoryManager.h>
#include <IO/OpenFile.h>
#include <IO/ValueSource.h>
#include <Math/Interval.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
****************/

template <class FileValueParam>
inline
void
readVertexPositions(
	DS& dataSet,
	DS::VertexIndex numGridPoints,
	IO::File& file,
	bool master)
	{
	typedef FileValueParam FileValue;
	
	DS::VertexIndex index=0;
	for(int percentRead=1;percentRead<=100;++percentRead)
		{
		DS::VertexIndex indexEnd=(numGridPoints*percentRead+50)/100;
		for(;index<indexEnd;++index)
			{
			/* Read the next vertex: */
			gridSource.skipWs();
			DS::Point vertexPosition;
			for(int i=0;i<3;++i)
				vertexPosition[i]=DS::Scalar(file.read<FileValue>());
			dataSet.addVertex(vertexPosition);
			}
		
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
		}
	}

template <class FileValueParam>
inline
void
readVectorAttributes(
	DS& dataSet,
	const std::string& attributeName,
	int sliceIndex,
	IO::File& file,
	bool master)
	{
	typedef FileValueParam FileValue;
	
	const DS::Index& size=dataSet.getNumVertices();
	DS::Index index;
	if(master)
		std::cout<<"Reading vector attribute "<<attributeName<<"...   0%"<<std::flush;
	Math::Interval<DataValue::VScalar> range[3];
	for(int i=0;i<3;++i)
		range[i]=Math::Interval<DataValue::VScalar>::empty;
	for(index[2]=0;index[2]<size[2];++index[2])
		{
		for(index[1]=0;index[1]<size[1];++index[1])
			for(index[0]=0;index[0]<size[0];++index[0])
				{
				DataValue::VVector vector;
				for(int i=0;i<3;++i)
					{
					vector[i]=DataValue::VVector::Scalar(file.read<FileValue>());
					range[i].addValue(vector[i]);
					}
				
				/* Store the vector's components and magnitude: */
				for(int i=0;i<3;++i)
					dataSet.getVertexValue(sliceIndex+i,index)=vector[i];
				dataSet.getVertexValue(sliceIndex+3,index)=DataValue::VScalar(Geometry::mag(vector));
				}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[2]+1)*100+size[2]/2)/size[2]<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	for(int i=0;i<3;++i)
		std::cout<<range[i].getMin()<<" - "<<range[i].getMax()<<std::endl;
	}

template <class FileValueParam>
inline
void
readScalarAttributes(
	DS& dataSet,
	const std::string& attributeName,
	int attributeNumScalars,
	int sliceIndex,
	IO::File& file,
	bool master)
	{
	typedef FileValueParam FileValue;
	
	const DS::Index& size=dataSet.getNumVertices();
	DS::Index index;
	if(master)
		std::cout<<"Reading "<<attributeNumScalars<<"-component scalar attribute "<<attributeName<<"...   0%"<<std::flush;
	for(index[2]=0;index[2]<size[2];++index[2])
		{
		for(index[1]=0;index[1]<size[1];++index[1])
			for(index[0]=0;index[0]<size[0];++index[0])
				{
				/* Reand and store the next scalar attribute: */
				dataSet.getVertexValue(sliceIndex,index)=DS::ValueScalar(file.read<FileValue>());
				file.skip<FileValue>(attributeNumScalars-1);
				}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[2]+1)*100+size[2]/2)/size[2]<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	}

}

/******************************************
Methods of class UnstructuredHexahedralVTK:
******************************************/

UnstructuredHexahedralVTK::UnstructuredHexahedralVTK(void)
	:BaseModule("UnstructuredHexahedralVTK")
	{
	}

Visualization::Abstract::DataSet* UnstructuredHexahedralVTK::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Open the input file: */
	IO::FilePtr file(openFile(args[0],pipe));
	
	/* Attach a value source to the file to read the header: */
	DS::VertexIndex numGridPoints=0;
	bool binary=false;
	std::string gridPointDataType;
	{
	IO::ValueSource headerSource(file);
	headerSource.setPunctuation('\n',true);
	
	/* Read the header line: */
	if(headerSource.readString()!="#"||headerSource.readString()!="vtk"||headerSource.readString()!="DataFile"||headerSource.readString()!="Version")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	
	/* Read the file version: */
	int vtkVersionMajor=headerSource.readInteger();
	if(headerSource.readChar()!='.')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	int vtkVersionMinor=headerSource.readInteger();
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	if(vtkVersionMajor>3||(vtkVersionMajor==3&&vtkVersionMinor>0))
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s is unsupported version %d.%d",args[0].c_str(),vtkVersionMajor,vtkVersionMinor);
	
	/* Skip the comment line: */
	headerSource.skipLine();
	headerSource.skipWs();
	
	/* Read the data storage type: */
	std::string storageType=headerSource.readString();
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed storage type definition",args[0].c_str());
	if(storageType=="BINARY")
		binary=true;
	else if(storageType!="ASCII")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has unrecognized storage type %s",args[0].c_str(),storageType.c_str());
	
	/* Read the data set descriptor: */
	if(headerSource.readString()!="DATASET")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s does not have a data set definition",args[0].c_str());
	std::string dataSetType=headerSource.readString();
	if(dataSetType!="UNSTRUCTURED_GRID")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has data set type %s instead of UNSTRUCTURED_GRID",args[0].c_str(),dataSetType.c_str());
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed data set definition",args[0].c_str());
	
	/* Read the grid point data type: */
	if(headerSource.readString()!="POINTS")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s does not define grid points",args[0].c_str());
	numGridPoints=headerSource.readInteger();
	gridPointDataType=headerSource.readString();
	if(headerSource.getChar()!='\n')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed grid point definition",args[0].c_str());
	}
	
	/* Read the grid points: */
	dataSet.reserveVertices(numGridPoints);
	if(master)
		std::cout<<"Reading grid vertices...   0%"<<std::flush;
	if(binary)
		{
		if(gridPointDataType=="unsigned_char")
			readVertexPositions<Misc::UInt8>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="char")
			readVertexPositions<Misc::SInt8>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="unsigned_short")
			readVertexPositions<Misc::UInt16>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="short")
			readVertexPositions<Misc::SInt16>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="unsigned_int")
			readVertexPositions<Misc::UInt32>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="int")
			readVertexPositions<Misc::SInt32>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="unsigned_long")
			readVertexPositions<Misc::UInt64>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="long")
			readVertexPositions<Misc::SInt64>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="float")
			readVertexPositions<Misc::Float32>(dataSet,numGridPoints,*file,master);
		else if(gridPointDataType=="double")
			readVertexPositions<Misc::Float64>(dataSet,numGridPoints,*file,master);
		else
			Misc::throwStdErr("UnstructuredHexahedralVTK::load: unsupported grid point data type %s",gridPointDataType.c_str());
		}
	else
		{
		/* Attach another data source to the file to read grid points: */
		IO::ValueSource gridSource(file);
		gridSource.setPunctuation('\n',true);
		
		DS::VertexIndex index=0;
		for(int percentRead=1;percentRead<=100;++percentRead)
			{
			DS::VertexIndex indexEnd=(numGridPoints*percentRead+50)/100;
			for(;index<indexEnd;++index)
				{
				/* Read the next vertex: */
				gridSource.skipWs();
				DS::Point vertexPosition;
				for(int i=0;i<3;++i)
					vertexPosition[i]=DS::Scalar(gridSource.readNumber());
				if(gridSource.getChar()!='\n')
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Invalid vertex position in VTK data file %s",args[0].c_str());
				dataSet.addVertex(vertexPosition);
				}
			
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
			}
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Attach a value source to the file to read the grid cell header: */
	DS::CellIndex numGridCells=0;
	{
	IO::ValueSource headerSource(file);
	headerSource.setPunctuation('\n',true);
	
	/* Read the grid cell descriptor: */
	if(headerSource.readString()!="CELLS")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s does not define grid cells",args[0].c_str());
	numGridCells=headerSource.readInteger();
	DS::CellIndex numNumbers=headerSource.readInteger();
	if(numNumbers!=numGridCells*(1+8))
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s is not a hexahedral grid",args[0].c_str());
	if(headerSource.getChar()!='\n')
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed grid point definition",args[0].c_str());
	}
	
	/* Read the grid cells: */
	dataSet.reserveCells(numGridCells);
	if(master)
		std::cout<<"Reading grid cells...   0%"<<std::flush;
	static const int vertexOrder[8]={0,1,3,2,4,5,7,6}; // VTK's cube vertex counting order
	if(binary)
		{
		DS::CellIndex index=0;
		for(int percentRead=1;percentRead<=100;++percentRead)
			{
			DS::CellIndex indexEnd=(numGridCells*percentRead+50)/100;
			for(;index<indexEnd;++index)
				{
				/* Read the next cell's number of vertices and vertex indices: */
				Misc::UInt32 indices[9];
				file->read(indices,9);
				if(indices[0]!=8)
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Non-hexahedral grid cell in VTK data file %s",args[0].c_str());
				
				/* Unswizzle the cell's vertex indices: */
				DS::VertexID cellVertices[8];
				for(int i=0;i<8;++i)
					cellVertices[vertexOrder[i]]=DS::VertexID(indices[i]+1);
				if(gridSource.getChar()!='\n')
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Invalid grid cell in VTK data file %s",args[0].c_str());
				
				/* Add the cell to the data set: */
				dataSet.addCel(cellVertices);
				}
			
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
			}
		}
	else
		{
		/* Attach another data source to the file to read grid cells: */
		IO::ValueSource gridSource(file);
		gridSource.setPunctuation('\n',true);
		
		DS::CellIndex index=0;
		for(int percentRead=1;percentRead<=100;++percentRead)
			{
			DS::CellIndex indexEnd=(numGridCells*percentRead+50)/100;
			for(;index<indexEnd;++index)
				{
				/* Read the next cell's number of vertices: */
				gridSource.skipWs();
				int numIndices=gridSource.readInteger();
				if(numIndices!=8)
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Non-hexahedral grid cell in VTK data file %s",args[0].c_str());
				
				/* Read and unswizzle the cell's vertex indices: */
				DS::VertexID cellVertices[8];
				for(int i=0;i<8;++i)
					cellVertices[vertexOrder[i]]=DS::VertexID(gridSource.readInteger());
				if(gridSource.getChar()!='\n')
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Invalid grid cell in VTK data file %s",args[0].c_str());
				
				/* Add the cell to the data set: */
				dataSet.addCel(cellVertices);
				}
			
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
			}
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Attach a value source to the file to read the grid cell type header: */
	{
	IO::ValueSource headerSource(file);
	headerSource.setPunctuation('\n',true);
	
	/* Read the grid cell type descriptor: */
	if(headerSource.readString()!="CELL_TYPES")
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s does not define grid cell types",args[0].c_str());
	if(headerSource.readInteger()!=numGridCells)
		Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has mismatching number of grid cell types",args[0].c_str());
	}
	
	/* Read the (redundant) cell type definition: */
	if(master)
		std::cout<<"Checking grid cell types...   0%"<<std::flush;
	if(binary)
		{
		DS::CellIndex index=0;
		for(int percentRead=1;percentRead<=100;++percentRead)
			{
			DS::CellIndex indexEnd=(numGridCells*percentRead+50)/100;
			for(;index<indexEnd;++index)
				{
				/* Check the next cell's type: */
				if(file->read<Misc::SInt32>()!=12) // VTK code for hexahedra
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Non-hexahedral grid cell in VTK data file %s",args[0].c_str());
				}
			
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
			}
		}
	else
		{
		/* Attach another data source to the file to read grid cells: */
		IO::ValueSource gridSource(file);
		gridSource.setPunctuation('\n',true);
		
		DS::CellIndex index=0;
		for(int percentRead=1;percentRead<=100;++percentRead)
			{
			DS::CellIndex indexEnd=(numGridCells*percentRead+50)/100;
			for(;index<indexEnd;++index)
				{
				/* Check the next cell's type: */
				if(gridSource.readInteger()!=12) // VTK code for hexahedra
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Non-hexahedral grid cell in VTK data file %s",args[0].c_str());
				if(gridSource.getChar()!='\n')
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: Invalid grid cell type in VTK data file %s",args[0].c_str());
				}
			
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<percentRead<<"%"<<std::flush;
			}
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	if(master)
		std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	if(master)
		std::cout<<" done"<<std::endl;
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Read all point attributes stored in the file: */
	while(true)
		{
		/* Attach a data source to the file to read an attribute header: */
		std::string attributeType,attributeName,attributeScalarType;
		int attributeNumScalars=1;
		{
		IO::ValueSource attributeSource(file);
		attributeSource.setPunctuation('\n',true);
		attributeSource.skipWs();
		if(attributeSource.readString()!="POINT_DATA")
			{
			/* No more attributes: */
			break;
			}
		
		/* Check the number of attributes: */
		if(attributeSource.readInteger()!=numGridPoints)
			Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s defines wrong number of point attributes",args[0].c_str());
		if(attributeSource.readChar()!='\n')
			Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed point attribute definition",args[0].c_str());
		
		/* Read the attribute type, name, and data type: */
		attributeType=attributeSource.readString();
		attributeName=attributeSource.readString();
		attributeScalarType=attributeSource.readString();
		if(attributeSource.peekc()!='\n')
			attributeNumScalars=attributeSource.readInteger();
		if(attributeSource.getChar()!='\n')
			Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has malformed point attribute definition",args[0].c_str());
		}
		
		/* Create the new attribute: */
		bool attributeVectors=false;
		int sliceIndex=dataSet.getNumSlices();
		if(attributeType=="SCALARS")
			{
			/* Add another slice to the data set: */
			dataSet.addSlice();
			
			/* Add another scalar variable to the data value: */
			dataValue.addScalarVariable(attributeName.c_str());
			}
		else if(attributeType=="VECTORS")
			{
			/* Add another vector variable to the data value: */
			attributeVectors=true;
			int vectorVariableIndex=dataValue.addVectorVariable(attributeName.c_str());
			
			/* Add four new slices to the data set (three components plus magnitude): */
			for(int i=0;i<4;++i)
				{
				dataSet.addSlice();
				int variableIndex=dataValue.addScalarVariable(makeVectorSliceName(attributeName,i).c_str());
				if(i<3)
					dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,variableIndex);
				}
			}
		else
			Misc::throwStdErr("UnstructuredHexahedralVTK::load: VTK data file %s has unknown point attribute type %s",args[0].c_str(),attributeType.c_str());
		
		/* Read the vertex attributes: */
		if(binary)
			{
			if(attributeVectors)
				{
				if(attributeScalarType=="unsigned_char")
					readVectorAttributes<Misc::UInt8>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="char")
					readVectorAttributes<Misc::SInt8>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_short")
					readVectorAttributes<Misc::UInt16>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="short")
					readVectorAttributes<Misc::SInt16>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_int")
					readVectorAttributes<Misc::UInt32>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="int")
					readVectorAttributes<Misc::SInt32>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_long")
					readVectorAttributes<Misc::UInt64>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="long")
					readVectorAttributes<Misc::SInt64>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="float")
					readVectorAttributes<Misc::Float32>(dataSet,attributeName,sliceIndex,*file,master);
				else if(attributeScalarType=="double")
					readVectorAttributes<Misc::Float64>(dataSet,attributeName,sliceIndex,*file,master);
				else
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: unsupported attribute scalar data type %s in vector attribute %s",attributeScalarType.c_str(),attributeName.c_str());
				}
			else
				{
				if(attributeScalarType=="unsigned_char")
					readScalarAttributes<Misc::UInt8>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="char")
					readScalarAttributes<Misc::SInt8>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_short")
					readScalarAttributes<Misc::UInt16>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="short")
					readScalarAttributes<Misc::SInt16>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_int")
					readScalarAttributes<Misc::UInt32>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="int")
					readScalarAttributes<Misc::SInt32>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="unsigned_long")
					readScalarAttributes<Misc::UInt64>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="long")
					readScalarAttributes<Misc::SInt64>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="float")
					readScalarAttributes<Misc::Float32>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else if(attributeScalarType=="double")
					readScalarAttributes<Misc::Float64>(dataSet,attributeName,attributeNumScalars,sliceIndex,*file,master);
				else
					Misc::throwStdErr("UnstructuredHexahedralVTK::load: unsupported attribute scalar data type %s in scalar attribute %s",attributeScalarType.c_str(),attributeName.c_str());
				}
			}
		else
			{
			/* Attach another data source to the file to read point attributes: */
			IO::ValueSource attributeSource(file);
			attributeSource.setPunctuation('\n',true);
			
			if(master)
				std::cout<<"Reading "<<attributeName<<" point attributes...   0%"<<std::flush;
			DS::Index index;
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				{
				if(attributeVectors)
					{
					for(index[1]=0;index[1]<numVertices[1];++index[1])
						for(index[0]=0;index[0]<numVertices[0];++index[0])
							{
							/* Read the next attribute: */
							attributeSource.skipWs();
							
							/* Read the vector value in Cartesian coordinates: */
							DataValue::VVector vector;
							for(int i=0;i<3;++i)
								vector[i]=DataValue::VVector::Scalar(attributeSource.readNumber());
							if(attributeSource.getChar()!='\n')
								Misc::throwStdErr("UnstructuredHexahedralVTK::load: Invalid vector attribute in in VTK data file %s",args[0].c_str());
							
							/* Store the vector's components and magnitude: */
							for(int i=0;i<3;++i)
								dataSet.getVertexValue(sliceIndex+i,index)=vector[i];
							dataSet.getVertexValue(sliceIndex+3,index)=DataValue::VScalar(Geometry::mag(vector));
							}
					}
				else
					{
					for(index[1]=0;index[1]<numVertices[1];++index[1])
						for(index[0]=0;index[0]<numVertices[0];++index[0])
							{
							/* Read the next attribute: */
							attributeSource.skipWs();
							
							/* Read the first scalar attribute from the line: */
							dataSet.getVertexValue(sliceIndex,index)=DS::ValueScalar(attributeSource.readNumber());
							
							/* Skip the rest of the line: */
							attributeSource.skipLine();
							}
					}
				if(master)
					std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[2]+1)*100+numVertices[2]/2)/numVertices[2]<<"%"<<std::flush;
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
	Visualization::Concrete::UnstructuredHexahedralVTK* module=new Visualization::Concrete::UnstructuredHexahedralVTK();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
