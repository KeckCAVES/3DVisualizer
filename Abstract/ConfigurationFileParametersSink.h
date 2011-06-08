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

#ifndef VISUALIZATION_ABSTRACT_CONFIGURATIONFILEPARAMETERSSINK_INCLUDED
#define VISUALIZATION_ABSTRACT_CONFIGURATIONFILEPARAMETERSSINK_INCLUDED

#include <Abstract/VariableManager.h>
#include <Abstract/ParametersSink.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Visualization {

namespace Abstract {

class ConfigurationFileParametersSink:public ParametersSink
	{
	/* Elements: */
	private:
	Misc::ConfigurationFileSection& cfg; // The configuration file section into which to write
	
	/* Constructors and destructors: */
	public:
	ConfigurationFileParametersSink(const VariableManager* sVariableManager,Misc::ConfigurationFileSection& sCfg);
	
	/* Methods from ParametersSink: */
	virtual void write(const char* name,const WriterBase& value);
	virtual void writeScalarVariable(const char* name,int scalarVariableIndex);
	virtual void writeVectorVariable(const char* name,int vectorVariableIndex);
	};

}

}

#endif
