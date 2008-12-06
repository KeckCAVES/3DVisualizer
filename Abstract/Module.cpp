/***********************************************************************
Module - Abstract base class to represent modules of visualization data
types and algorithms. A module corresponds to a dynamically-linkable
unit of functionality in a 3D visualization application.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>

#include <Abstract/Module.h>

namespace Visualization {

namespace Abstract {

/***********************
Methods of class Module:
***********************/

Module::Module(const char* sClassName)
	:Plugins::Factory(sClassName)
	{
	}

Module::~Module(void)
	{
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

Algorithm* Module::getScalarAlgorithm(int scalarAlgorithmIndex,VariableManager* variableManager,Comm::MulticastPipe* pipe) const
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

Algorithm* Module::getVectorAlgorithm(int vectorAlgorithmIndex,VariableManager* variableManager,Comm::MulticastPipe* pipe) const
	{
	Misc::throwStdErr("Module::getVectorAlgorithm: invalid algorithm index %d",vectorAlgorithmIndex);
	return 0;
	}

}

}
