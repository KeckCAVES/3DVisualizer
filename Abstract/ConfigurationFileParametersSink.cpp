/***********************************************************************
ConfigurationFileParametersSink - Class for parameter sinks writing into
a configuration file section.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010 Oliver Kreylos

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

#include <Abstract/ConfigurationFileParametersSink.h>

#include <string>
#include <Misc/ConfigurationFile.h>

namespace Visualization {

namespace Abstract {

/************************************************
Methods of class ConfigurationFileParametersSink:
************************************************/

ConfigurationFileParametersSink::ConfigurationFileParametersSink(const VariableManager* sVariableManager,Misc::ConfigurationFileSection& sCfg)
	:ParametersSink(sVariableManager),
	 cfg(sCfg)
	{
	}

void ConfigurationFileParametersSink::write(const char* name,const WriterBase& value)
	{
	/* Write the value into a string: */
	std::string valueString;
	value.write(valueString);
	
	/* Store the string in the configuration file section: */
	cfg.storeString(name,valueString);
	}

void ConfigurationFileParametersSink::writeScalarVariable(const char* name,int scalarVariableIndex)
	{
	/* Get the variable's name: */
	std::string scalarVariableName=variableManager->getScalarVariableName(scalarVariableIndex);
	
	/* Store the variable's name in the configuration file section: */
	cfg.storeString(name,scalarVariableName);
	}

void ConfigurationFileParametersSink::writeVectorVariable(const char* name,int vectorVariableIndex)
	{
	/* Get the variable's name: */
	std::string vectorVariableName=variableManager->getVectorVariableName(vectorVariableIndex);
	
	/* Store the variable's name in the configuration file section: */
	cfg.storeString(name,vectorVariableName);
	}

}

}
