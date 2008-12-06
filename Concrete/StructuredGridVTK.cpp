/***********************************************************************
StructuredGridVTK - Class reading curvilinear grids from files in legacy
VTK format.
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
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/StructuredGridVTK.h>

namespace Visualization {

namespace Concrete {

/**********************************
Methods of class StructuredGridVTK:
**********************************/

StructuredGridVTK::StructuredGridVTK(void)
	:BaseModule("StructuredGridVTK")
	{
	}

Visualization::Abstract::DataSet* StructuredGridVTK::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	
	/* Open the input file: */
	Misc::File dataFile(args[0].c_str(),"rb");
	
	/* Read the header line: */
	char line[258];
	dataFile.gets(line,sizeof(line));
	int vtkVersionMajor,vtkVersionMinor;
	if(sscanf(line,"# vtk DataFile Version %d.%d",&vtkVersionMajor,&vtkVersionMinor)!=2)
		Misc::throwStdErr("StructuredGridVTK::load: Input file %s is not a VTK data file",args[0].c_str());
	if(vtkVersionMajor>3||(vtkVersionMajor==3&&vtkVersionMinor>0))
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s is unsupported version %d.%d",args[0].c_str(),vtkVersionMajor,vtkVersionMinor);
	
	/* Ignore the comment line: */
	dataFile.gets(line,sizeof(line));
	
	/* Read the file storage format: */
	bool binary=false;
	dataFile.gets(line,sizeof(line));
	if(strncasecmp(line,"BINARY",6)==0&&isspace(line[6]))
		binary=true;
	else if(strncasecmp(line,"ASCII",5)!=0||!isspace(line[5]))
		Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s has unrecognized storage type",args[0].c_str());
	
	/* Read the grid type: */
	dataFile.gets(line,sizeof(line));
	if(strncasecmp(line,"DATASET",7)==0&&isspace(line[7]))
		{
		char* typeStart;
		for(typeStart=line+8;*typeStart!='\0'&&isspace(*typeStart);++typeStart)
			;
		char* typeEnd;
		for(typeEnd=typeStart;*typeEnd!='\0'&&!isspace(*typeEnd);++typeEnd)
			;
		*typeEnd='\0';
		if(strcasecmp(typeStart,"STRUCTURED_GRID")!=0)
			Misc::throwStdErr("StructuredGridVTK::load: VTK data file %s contains wrong grid type %s",args[0].c_str(),typeStart);
		}
	else
		Misc::throwStdErr("StructuredGridVTK::load: invalid grid type in VTK data file %s",args[0].c_str());
	
	/* Read the grid size: */
	DS::Index numVertices;
	dataFile.gets(line,sizeof(line));
	if(sscanf(line,"DIMENSIONS %d %d %d",&numVertices[0],&numVertices[1],&numVertices[2])!=3)
		Misc::throwStdErr("StructuredGridVTK::load: invalid grid dimension in VTK data file %s",args[0].c_str());
	
	/* Initialize the data set: */
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	
	/* Read the grid vertices: */
	dataFile.gets(line,sizeof(line));
	int totalNumVertices;
	char vertexScalarType[10];
	if(sscanf(line,"POINTS %d %10s",&totalNumVertices,vertexScalarType)!=2)
		Misc::throwStdErr("StructuredGridVTK::load: invalid grid point definition in VTK data file %s",args[0].c_str());
	if(totalNumVertices!=numVertices.calcIncrement(-1))
		Misc::throwStdErr("StructuredGridVTK::load: mismatching number of grid points in VTK data file %s",args[0].c_str());
	if(strcasecmp(vertexScalarType,"float")!=0&&strcasecmp(vertexScalarType,"double")!=0)
		Misc::throwStdErr("StructuredGridVTK::load: unsupported grid point scalar type %s in VTK data file %s",vertexScalarType,args[0].c_str());
	std::cout<<"Reading grid vertices...   0%"<<std::flush;
	DS::Index index(0);
	while(index[2]<numVertices[2])
		{
		/* Read the next line: */
		dataFile.gets(line,sizeof(line));
		
		/* Parse the line: */
		DS::Point& vertex=dataSet.getVertexPosition(index);
		if(sscanf(line,"%f %f %f",&vertex[0],&vertex[1],&vertex[2])!=3)
			Misc::throwStdErr("StructuredGridVTK::load: Invalid vertex position in VTK data file %s",args[0].c_str());
		
		/* Go to the next vertex: */
		int incDim;
		for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
			index[incDim]=0;
		++index[incDim];
		if(incDim==2)
			std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	std::cout<<" done"<<std::endl;
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	/* Read all point attributes stored in the file: */
	while(true)
		{
		/* Read the attribute header line: */
		dataFile.gets(line,sizeof(line));
		
		/* Check if there is another attribute: */
		int totalNumAttributes;
		if(sscanf(line,"POINT_DATA %d",&totalNumAttributes)!=1)
			break;
		if(totalNumAttributes!=numVertices.calcIncrement(-1))
			Misc::throwStdErr("StructuredGridVTK::load: mismatching number of point attributes in VTK data file %s",args[0].c_str());
		
		/* Read the attribute type and name: */
		dataFile.gets(line,sizeof(line));
		char attributeType[40];
		char attributeName[80];
		char attributeScalarType[10];
		if(sscanf(line,"%40s %80s %10s",attributeType,attributeName,attributeScalarType)!=3)
			Misc::throwStdErr("StructuredGridVTK::load: invalid point attribute definition in VTK data file %s",args[0].c_str());
		
		bool vectorAttribute=false;
		if(strcasecmp(attributeType,"VECTORS")==0)
			vectorAttribute=true;
		else if(strcasecmp(attributeType,"SCALARS")!=0)
			Misc::throwStdErr("StructuredGridVTK::load: unsupported point attribute type %s in VTK data file %s",attributeType,args[0].c_str());
		if(strcasecmp(attributeScalarType,"float")!=0&&strcasecmp(attributeScalarType,"double")!=0)
			Misc::throwStdErr("StructuredGridVTK::load: unsupported point attribute scalar type %s in VTK data file %s",attributeScalarType,args[0].c_str());
		
		/* Create the new attribute: */
		int sliceIndex=dataSet.getNumSlices();
		if(vectorAttribute)
			{
			/* Add another vector variable to the data value: */
			int vectorVariableIndex=dataValue.getNumVectorVariables();
			dataValue.addVectorVariable(attributeName);

			/* Add four new slices to the data set (3 components plus magnitude): */
			char variableName[256];
			for(int i=0;i<3;++i)
				{
				dataSet.addSlice();
				snprintf(variableName,sizeof(variableName),"%s %c",attributeName,'X'+i);
				dataValue.addScalarVariable(variableName);
				dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,sliceIndex+i);
				}
			dataSet.addSlice();
			snprintf(variableName,sizeof(variableName),"%s Magnitude",attributeName);
			dataValue.addScalarVariable(variableName);
			}
		else
			{
			/* Add another slice to the data set: */
			dataSet.addSlice();
			
			/* Add another scalar variable to the data value: */
			dataValue.addScalarVariable(attributeName);
			}
		
		/* Read all vertex attributes: */
		std::cout<<"Reading "<<attributeName<<" point attributes...   0%"<<std::flush;
		DS::Index index(0);
		while(index[2]<numVertices[2])
			{
			/* Read the next line: */
			dataFile.gets(line,sizeof(line));
			
			/* Parse the line: */
			if(vectorAttribute)
				{
				/* Read the vector attribute in Cartesian coordinates: */
				DataValue::VVector vector;
				if(sscanf(line,"%lf %lf %lf",&vector[0],&vector[1],&vector[2])!=3)
					Misc::throwStdErr("StructuredGridVTK::load: Invalid vector attribute in in VTK data file %s",args[0].c_str());
				
				/* Store the vector's components and magnitude: */
				for(int i=0;i<3;++i)
					dataSet.getVertexValue(sliceIndex+i,index)=vector[i];
				dataSet.getVertexValue(sliceIndex+3,index)=DataValue::VScalar(Geometry::mag(vector));
				}
			else
				{
				if(sscanf(line,"%lf",&dataSet.getVertexValue(sliceIndex,index))!=1)
					Misc::throwStdErr("StructuredGridVTK::load: Invalid scalar attribute in in VTK data file %s",args[0].c_str());
				}
			
			/* Go to the next vertex: */
			int incDim;
			for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
				index[incDim]=0;
			++index[incDim];
			if(incDim==2)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
			}
		std::cout<<"\b\b\b\bdone"<<std::endl;
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
