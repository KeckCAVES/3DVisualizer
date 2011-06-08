/***********************************************************************
FileParametersSource - Class for parameter sources reading from text
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

#ifndef VISUALIZATION_ABSTRACT_FILEPARAMETERSSOURCE_INCLUDED
#define VISUALIZATION_ABSTRACT_FILEPARAMETERSSOURCE_INCLUDED

#include <string>
#include <Misc/StringHashFunctions.h>
#include <Misc/HashTable.h>

#include <Abstract/VariableManager.h>
#include <Abstract/ParametersSource.h>

/* Forward declarations: */
namespace IO {
class ValueSource;
}

namespace Visualization {

namespace Abstract {

class FileParametersSource:public ParametersSource
	{
	/* Embedded classes: */
	private:
	typedef Misc::HashTable<std::string,std::string> TagValueMap; // Type for hash tables to map parameter names to parameter values
	
	/* Elements: */
	private:
	IO::ValueSource& source; // Value source to read from the text file
	TagValueMap tagValueMap; // The map containing all parameter names and values for the next parameter set
	
	/* Constructors and destructors: */
	public:
	FileParametersSource(VariableManager* sVariableManager,IO::ValueSource& sSource);
	
	/* Methods from ParametersSource: */
	virtual void read(const char* name,const ReaderBase& value);
	virtual void readScalarVariable(const char* name,int& scalarVariableIndex);
	virtual void readVectorVariable(const char* name,int& vectorVariableIndex);
	};

}

}

#endif
