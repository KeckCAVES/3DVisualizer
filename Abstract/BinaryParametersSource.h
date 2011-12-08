/***********************************************************************
BinaryParametersSource - Class for parameter sources reading from a file
abstraction.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_BINARYPARAMETERSSOURCE_INCLUDED
#define VISUALIZATION_ABSTRACT_BINARYPARAMETERSSOURCE_INCLUDED

#include <Abstract/VariableManager.h>
#include <Abstract/ParametersSource.h>

/* Forward declarations: */
namespace IO {
class File;
}

namespace Visualization {

namespace Abstract {

class BinaryParametersSource:public ParametersSource
	{
	/* Elements: */
	private:
	IO::File& source; // The data source
	bool raw; // Flag whether the source reads variable indices (true) or variable names (false)
	
	/* Constructors and destructors: */
	public:
	BinaryParametersSource(VariableManager* sVariableManager,IO::File& sSource,bool sRaw);
	
	/* Methods from ParametersSource: */
	virtual void read(const char* name,const ReaderBase& value);
	virtual void readScalarVariable(const char* name,int& scalarVariableIndex);
	virtual void readVectorVariable(const char* name,int& vectorVariableIndex);
	};

}

}

#endif
