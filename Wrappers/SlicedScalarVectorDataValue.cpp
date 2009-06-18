/***********************************************************************
SlicedScalarVectorDataValue - Class for data value descriptors giving
access to multiple scalar and/or vector variables stored in a sliced
data set.
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

#include <string.h>

#include <Wrappers/SlicedScalarVectorDataValue.h>

namespace Visualization {

namespace Wrappers {

/************************************************
Methods of class SlicedScalarVectorDataValueBase:
************************************************/

SlicedScalarVectorDataValueBase::SlicedScalarVectorDataValueBase(void)
	:numScalarVariables(0),scalarVariableNames(0),
	 numVectorComponents(0),
	 numVectorVariables(0),vectorVariableNames(0),vectorVariableScalarIndices(0)
	{
	}

SlicedScalarVectorDataValueBase::~SlicedScalarVectorDataValueBase(void)
	{
	for(int i=0;i<numScalarVariables;++i)
		delete[] scalarVariableNames[i];
	delete[] scalarVariableNames;
	for(int i=0;i<numVectorVariables;++i)
		delete[] vectorVariableNames[i];
	delete[] vectorVariableNames;
	delete[] vectorVariableScalarIndices;
	}

void SlicedScalarVectorDataValueBase::initialize(int sNumScalarVariables,int sNumVectorComponents,int sNumVectorVariables)
	{
	/* Initialize scalar value arrays: */
	for(int i=0;i<numScalarVariables;++i)
		delete[] scalarVariableNames[i];
	delete[] scalarVariableNames;
	numScalarVariables=sNumScalarVariables;
	scalarVariableNames=new char*[numScalarVariables];
	for(int i=0;i<numScalarVariables;++i)
		scalarVariableNames[i]=0;
	
	/* Initialize vector variable arrays: */
	for(int i=0;i<numVectorVariables;++i)
		delete[] vectorVariableNames[i];
	delete[] vectorVariableNames;
	delete[] vectorVariableScalarIndices;
	numVectorComponents=sNumVectorComponents;
	numVectorVariables=sNumVectorVariables;
	if(numVectorVariables>0)
		{
		vectorVariableNames=new char*[numVectorVariables];
		vectorVariableScalarIndices=new int[numVectorVariables*numVectorComponents];
		for(int i=0;i<numVectorVariables;++i)
			{
			vectorVariableNames[i]=0;
			for(int j=0;j<numVectorComponents;++j)
				vectorVariableScalarIndices[i*numVectorComponents+j]=-1;
			}
		}
	else
		{
		vectorVariableNames=0;
		vectorVariableScalarIndices=0;
		}
	}

void SlicedScalarVectorDataValueBase::setScalarVariableName(int scalarVariableIndex,const char* newScalarVariableName)
	{
	delete[] scalarVariableNames[scalarVariableIndex];
	scalarVariableNames[scalarVariableIndex]=new char[strlen(newScalarVariableName)+1];
	strcpy(scalarVariableNames[scalarVariableIndex],newScalarVariableName);
	}

int SlicedScalarVectorDataValueBase::addScalarVariable(const char* newScalarVariableName)
	{
	/* Make room in the scalar variable array and copy the old variable names and create the new one: */
	char** newScalarVariableNames=new char*[numScalarVariables+1];
	for(int i=0;i<numScalarVariables;++i)
		newScalarVariableNames[i]=scalarVariableNames[i];
	newScalarVariableNames[numScalarVariables]=new char[strlen(newScalarVariableName)+1];
	strcpy(newScalarVariableNames[numScalarVariables],newScalarVariableName);
	
	/* Install the new scalar variable array: */
	delete[] scalarVariableNames;
	++numScalarVariables;
	scalarVariableNames=newScalarVariableNames;
	
	return numScalarVariables-1;
	}

void SlicedScalarVectorDataValueBase::setVectorVariableName(int vectorVariableIndex,const char* newVectorVariableName)
	{
	delete[] vectorVariableNames[vectorVariableIndex];
	vectorVariableNames[vectorVariableIndex]=new char[strlen(newVectorVariableName)+1];
	strcpy(vectorVariableNames[vectorVariableIndex],newVectorVariableName);
	}

int SlicedScalarVectorDataValueBase::addVectorVariable(const char* newVectorVariableName)
	{
	/* Make room in the vector variable array and copy the old variable names and create the new one: */
	char** newVectorVariableNames=new char*[numVectorVariables+1];
	int* newVectorVariableScalarIndices=new int[(numVectorVariables+1)*numVectorComponents];
	for(int i=0;i<numVectorVariables;++i)
		{
		newVectorVariableNames[i]=vectorVariableNames[i];
		for(int j=0;j<numVectorComponents;++j)
			newVectorVariableScalarIndices[i*numVectorComponents+j]=vectorVariableScalarIndices[i*numVectorComponents+j];
		}
	newVectorVariableNames[numVectorVariables]=new char[strlen(newVectorVariableName)+1];
	strcpy(newVectorVariableNames[numVectorVariables],newVectorVariableName);
	for(int j=0;j<numVectorComponents;++j)
		newVectorVariableScalarIndices[numVectorVariables*numVectorComponents+j]=-1;
	
	/* Install the new vector variable array: */
	delete[] vectorVariableNames;
	delete[] vectorVariableScalarIndices;
	++numVectorVariables;
	vectorVariableNames=newVectorVariableNames;
	vectorVariableScalarIndices=newVectorVariableScalarIndices;
	
	return numVectorVariables-1;
	}

void SlicedScalarVectorDataValueBase::setVectorVariableScalarIndex(int vectorVariableIndex,int componentIndex,int scalarVariableIndex)
	{
	vectorVariableScalarIndices[vectorVariableIndex*numVectorComponents+componentIndex]=scalarVariableIndex;
	}

}

}
