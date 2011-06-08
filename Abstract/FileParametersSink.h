/***********************************************************************
FileParametersSink - Class for parameter sinks writing into simple text
files.
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

#ifndef VISUALIZATION_ABSTRACT_FILEPARAMETERSSINK_INCLUDED
#define VISUALIZATION_ABSTRACT_FILEPARAMETERSSINK_INCLUDED

#include <Abstract/VariableManager.h>
#include <Abstract/ParametersSink.h>

/* Forward declarations: */
namespace Misc {
class File;
}

namespace Visualization {

namespace Abstract {

class FileParametersSink:public ParametersSink
	{
	/* Elements: */
	private:
	Misc::File& file; // The text file
	
	/* Constructors and destructors: */
	public:
	FileParametersSink(const VariableManager* sVariableManager,Misc::File& sFile);
	
	/* Methods from ParametersSink: */
	virtual void write(const char* name,const WriterBase& value);
	virtual void writeScalarVariable(const char* name,int scalarVariableIndex);
	virtual void writeVectorVariable(const char* name,int vectorVariableIndex);
	};

}

}

#endif
