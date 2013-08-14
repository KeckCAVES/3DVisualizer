/***********************************************************************
ConfigurationFileParametersSource - Class for parameter sources reading
from a configuration file section.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <Abstract/ConfigurationFileParametersSource.h>

#include <string>
#include <Misc/ConfigurationFile.h>

namespace Visualization {

namespace Abstract {

/**************************************************
Methods of class ConfigurationFileParametersSource:
**************************************************/

ConfigurationFileParametersSource::ConfigurationFileParametersSource(VariableManager* sVariableManager,const Misc::ConfigurationFileSection& sCfg)
	:ParametersSource(sVariableManager),
	 cfg(sCfg)
	{
	}

void ConfigurationFileParametersSource::read(const char* name,const ReaderBase& value)
	{
	/* Retrieve the named string from the configuration file section: */
	std::string valueString=cfg.retrieveString(name);
	
	/* Read the value from the string: */
	value.read(valueString);
	}

void ConfigurationFileParametersSource::readScalarVariable(const char* name,int& scalarVariableIndex)
	{
	/* Retrieve the variable's name from the configuration file section: */
	std::string scalarVariableName=cfg.retrieveString(name);
	
	/* Get the variable's index: */
	scalarVariableIndex=variableManager->getScalarVariable(scalarVariableName.c_str());
	}

void ConfigurationFileParametersSource::readVectorVariable(const char* name,int& vectorVariableIndex)
	{
	/* Retrieve the variable's name from the configuration file section: */
	std::string vectorVariableName=cfg.retrieveString(name);
	
	/* Get the variable's index: */
	vectorVariableIndex=variableManager->getVectorVariable(vectorVariableName.c_str());
	}

}

}
