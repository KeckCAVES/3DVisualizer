/***********************************************************************
StructuredGridVTK - Class reading curvilinear grids from files in legacy
VTK format.
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

#include <Concrete/StructuredGridVTK.h>

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
	IO::File& file,
	bool master)
	{
	typedef FileValueParam FileValue;
	
	const DS::Index& size=dataSet.getNumVertices();
	DS::Index index;
	if(master)
		std::cout<<"Reading grid vertices...   0%"<<std::flush;
	for(index[2]=0;index[2]<size[2];++index[2])
		{
		for(index[1]=0;index[1]<size[1];++index[1])
			for(index[0]=0;index[0]<size[0];++index[0])
				{
				DS::Point& vertex=dataSet.getVertexPosition(index);
				for(int i=0;i<3;++i)
					vertex[i]=DS::Scalar(file.read<FileValue>());
				}
		if(master)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[2]+1)*100+size[2]/2)/size[2]<<"%"<<std::flush;
		}
	if(master)
		std::cout<<"\b\b\b\bdone"<<std::endl;
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

/**********************************
Methods of class StructuredGridVTK:
**********************************/

StructuredGridVTK::StructuredGridVTK(void)
	:BaseModule("StructuredGridVTK")
	{
	}

Visualization::Abstract::DataSet* StructuredGridVTK::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	DS& dataSet=result->getDs();
	
	/* Open the input file: */
	IO::FilePtr file(openFile(args[0],pipe));
	
	/* Attach a value source to the file to read the header: */
	DS::Index numVertices;
	bool binary=false;
	std::string gridPointDataType;
	{
	IO::ValueSource headerSource(file);
	headerSource.setPunctuation('\n',true);
	
	/* Read the header line: */
	if(headerSource.readString()!="#"||headerSource.readString()!="vtk"||headerSource.readString()!="DataFile"||headerSource.readString()!="Version")
		Misc::throwStdErr("StructuredGridVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	
	/* Read the file version: */
	int vtkVersionMajor=headerSource.readInteger();
	if(headerSource.readChar()!='.')
		Misc::throwStdErr("StructuredGridVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	int vtkVersionMinor=headerSource.readInteger();
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("StructuredGridVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	if(vtkVersionMajor>3||(vtkVersionMajor==3&&vtkVersionMinor>0))
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s is unsupported version %d.%d",args[0].c_str(),vtkVersionMajor,vtkVersionMinor);
	
	/* Skip the comment line: */
	headerSource.skipLine();
	headerSource.skipWs();
	
	/* Read the data storage type: */
	std::string storageType=headerSource.readString();
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed storage type definition",args[0].c_str());
	if(storageType=="BINARY")
		binary=true;
	else if(storageType!="ASCII")
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has unrecognized storage type %s",args[0].c_str(),storageType.c_str());
	
	/* Read the data set descriptor: */
	if(headerSource.readString()!="DATASET")
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s does not have a data set definition",args[0].c_str());
	std::string dataSetType=headerSource.readString();
	if(dataSetType!="STRUCTURED_GRID")
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has data set type %s instead of STRUCTURED_GRID",args[0].c_str(),dataSetType.c_str());
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed data set definition",args[0].c_str());
	
	/* Read the grid size: */
	if(headerSource.readString()!="DIMENSIONS")
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s does not define data set dimensions",args[0].c_str());
	for(int i=0;i<3;++i)
		{
		if(headerSource.peekc()=='\n')
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has too few data set dimensions",args[0].c_str());
		numVertices[i]=headerSource.readInteger();
		}
	if(headerSource.readChar()!='\n')
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed data set dimensions",args[0].c_str());
	if(numVertices[0]<1||numVertices[1]<1||numVertices[2]<1)
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has invalid data set dimensions",args[0].c_str());
	
	/* Read the grid point data type: */
	if(headerSource.readString()!="POINTS")
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s does not define grid points",args[0].c_str());
	if(headerSource.readInteger()!=numVertices.calcIncrement(-1))
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s defines wrong number of grid points",args[0].c_str());
	gridPointDataType=headerSource.readString();
	if(headerSource.getChar()!='\n')
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed grid point definition",args[0].c_str());
	}
	
	/* Initialize the data set: */
	dataSet.setGrid(numVertices);
	
	/* Read the grid points: */
	if(binary)
		{
		if(gridPointDataType=="unsigned_char")
			readVertexPositions<Misc::UInt8>(dataSet,*file,master);
		else if(gridPointDataType=="char")
			readVertexPositions<Misc::SInt8>(dataSet,*file,master);
		else if(gridPointDataType=="unsigned_short")
			readVertexPositions<Misc::UInt16>(dataSet,*file,master);
		else if(gridPointDataType=="short")
			readVertexPositions<Misc::SInt16>(dataSet,*file,master);
		else if(gridPointDataType=="unsigned_int")
			readVertexPositions<Misc::UInt32>(dataSet,*file,master);
		else if(gridPointDataType=="int")
			readVertexPositions<Misc::SInt32>(dataSet,*file,master);
		else if(gridPointDataType=="unsigned_long")
			readVertexPositions<Misc::UInt64>(dataSet,*file,master);
		else if(gridPointDataType=="long")
			readVertexPositions<Misc::SInt64>(dataSet,*file,master);
		else if(gridPointDataType=="float")
			readVertexPositions<Misc::Float32>(dataSet,*file,master);
		else if(gridPointDataType=="double")
			readVertexPositions<Misc::Float64>(dataSet,*file,master);
		else
			Misc::throwStdErr("StructuredGridVTK::load: unsupported grid point data type %s",gridPointDataType.c_str());
		}
	else
		{
		/* Attach another data source to the file to read grid points: */
		IO::ValueSource gridSource(file);
		gridSource.setPunctuation('\n',true);
		
		if(master)
			std::cout<<"Reading grid vertices...   0%"<<std::flush;
		DS::Index index;
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			{
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				for(index[0]=0;index[0]<numVertices[0];++index[0])
					{
					/* Read the next vertex: */
					gridSource.skipWs();
					DS::Point& vertex=dataSet.getVertexPosition(index);
					for(int i=0;i<3;++i)
						vertex[i]=DS::Scalar(gridSource.readNumber());
					if(gridSource.getChar()!='\n')
						Misc::throwStdErr("StructuredGridVTK::load: Invalid vertex position in VTK data file %s",args[0].c_str());
					}
			if(master)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<((index[2]+1)*100+numVertices[2]/2)/numVertices[2]<<"%"<<std::flush;
			}
		if(master)
			std::cout<<"\b\b\b\bdone"<<std::endl;
		}
	
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
		if(attributeSource.readInteger()!=numVertices.calcIncrement(-1))
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s defines wrong number of point attributes",args[0].c_str());
		if(attributeSource.readChar()!='\n')
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed point attribute definition",args[0].c_str());
		
		/* Read the attribute type, name, and data type: */
		attributeType=attributeSource.readString();
		attributeName=attributeSource.readString();
		attributeScalarType=attributeSource.readString();
		if(attributeSource.peekc()!='\n')
			attributeNumScalars=attributeSource.readInteger();
		if(attributeSource.getChar()!='\n')
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has malformed point attribute definition",args[0].c_str());
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
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has unknown point attribute type %s",args[0].c_str(),attributeType.c_str());
		
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
					Misc::throwStdErr("StructuredGridVTK::load: unsupported attribute scalar data type %s in vector attribute %s",attributeScalarType.c_str(),attributeName.c_str());
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
					Misc::throwStdErr("StructuredGridVTK::load: unsupported attribute scalar data type %s in scalar attribute %s",attributeScalarType.c_str(),attributeName.c_str());
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
								Misc::throwStdErr("StructuredGridVTK::load: Invalid vector attribute in in VTK data file %s",args[0].c_str());
							
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
	Visualization::Concrete::StructuredGridVTK* module=new Visualization::Concrete::StructuredGridVTK();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
