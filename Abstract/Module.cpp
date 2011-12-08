/***********************************************************************
Module - Abstract base class to represent modules of visualization data
types and algorithms. A module corresponds to a dynamically-linkable
unit of functionality in a 3D visualization application.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2011 Oliver Kreylos

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

#include <Abstract/Module.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <IO/OpenFile.h>
#include <Cluster/MulticastPipe.h>
#include <Cluster/OpenFile.h>

namespace Visualization {

namespace Abstract {

/***********************
Methods of class Module:
***********************/

std::string Module::makeVectorSliceName(std::string vectorName,int sliceIndex)
	{
	std::string result=vectorName;
	
	if(sliceIndex<3)
		{
		result.push_back(' ');
		result.push_back('X'+sliceIndex);
		}
	else
		result.append(" Magnitude");
	
	return result;
	}

std::string Module::getFullPath(std::string fileName) const
	{
	/* Check if the file name is fully-qualified: */
	if(fileName.empty()||fileName[0]=='/')
		{
		/* Return the file name unchanged: */
		return fileName;
		}
	else
		{
		/* Concatenate the file name to the base directory: */
		std::string result=baseDirectory;
		result.append(fileName);
		return result;
		}
	}

IO::FilePtr Module::openFile(std::string fileName,Cluster::MulticastPipe* pipe) const
	{
	if(pipe!=0)
		return Cluster::openFile(pipe->getMultiplexer(),getFullPath(fileName).c_str());
	else
		return IO::openFile(getFullPath(fileName).c_str());
	}

IO::SeekableFilePtr Module::openSeekableFile(std::string fileName,Cluster::MulticastPipe* pipe) const
	{
	if(pipe!=0)
		return Cluster::openSeekableFile(pipe->getMultiplexer(),getFullPath(fileName).c_str());
	else
		return IO::openSeekableFile(getFullPath(fileName).c_str());
	}

Module::Module(const char* sClassName)
	:Plugins::Factory(sClassName),
	 baseDirectory("")
	{
	}

Module::~Module(void)
	{
	}

void Module::setBaseDirectory(std::string newBaseDirectory)
	{
	/* Copy the base directory: */
	baseDirectory=newBaseDirectory;
	
	/* If the base directory is not empty, terminate it with a slash: */
	if(!baseDirectory.empty()&&baseDirectory[baseDirectory.length()-1]!='/')
		baseDirectory.push_back('/');
	}

int Module::getNumScalarAlgorithms(void) const
	{
	return 0;
	}

const char* Module::getScalarAlgorithmName(int scalarAlgorithmIndex) const
	{
	Misc::throwStdErr("Module::getScalarAlgorithmName: invalid algorithm index %d",scalarAlgorithmIndex);
	return 0;
	}

Algorithm* Module::getScalarAlgorithm(int scalarAlgorithmIndex,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const
	{
	Misc::throwStdErr("Module::getScalarAlgorithm: invalid algorithm index %d",scalarAlgorithmIndex);
	return 0;
	}

int Module::getNumVectorAlgorithms(void) const
	{
	return 0;
	}

const char* Module::getVectorAlgorithmName(int vectorAlgorithmIndex) const
	{
	Misc::throwStdErr("Module::getVectorAlgorithmName: invalid algorithm index %d",vectorAlgorithmIndex);
	return 0;
	}

Algorithm* Module::getVectorAlgorithm(int vectorAlgorithmIndex,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const
	{
	Misc::throwStdErr("Module::getVectorAlgorithm: invalid algorithm index %d",vectorAlgorithmIndex);
	return 0;
	}

Algorithm* Module::getAlgorithm(const char* algorithmName,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const
	{
	/* Try all scalar algorithms: */
	int numScalarAlgorithms=getNumScalarAlgorithms();
	for(int i=0;i<numScalarAlgorithms;++i)
		if(strcmp(algorithmName,getScalarAlgorithmName(i))==0)
			return getScalarAlgorithm(i,variableManager,pipe);
	
	/* Try all vector algorithms: */
	int numVectorAlgorithms=getNumVectorAlgorithms();
	for(int i=0;i<numVectorAlgorithms;++i)
		if(strcmp(algorithmName,getVectorAlgorithmName(i))==0)
			return getVectorAlgorithm(i,variableManager,pipe);
	
	/* Return an invalid algorithm: */
	return 0;
	}

}

}
