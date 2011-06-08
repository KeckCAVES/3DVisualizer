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

#include <Abstract/FileParametersSource.h>

#include <Misc/ThrowStdErr.h>
#include <IO/ValueSource.h>

namespace Visualization {

namespace Abstract {

/*************************************
Methods of class FileParametersSource:
*************************************/

FileParametersSource::FileParametersSource(VariableManager* sVariableManager,IO::ValueSource& sSource)
	:ParametersSource(sVariableManager),
	 source(sSource),
	 tagValueMap(17)
	{
	/* Check for opening brace: */
	if(!source.isLiteral('{'))
		Misc::throwStdErr("FileParameterSource::FileParameterSource: Missing opening brace in input file");
	
	/* Read all tag/value pairs: */
	while(!source.eof()&&source.peekc()!='}')
		{
		/* Read the next tag: */
		std::string tag=source.readString();
		
		/* Read the next value until the end of the current line: */
		std::string value=source.readLine();
		source.skipWs();
		
		/* Store the tag/value pair: */
		tagValueMap.setEntry(TagValueMap::Entry(tag,value));
		}
	
	/* Check for closing brace: */
	if(!source.isLiteral('}'))
		Misc::throwStdErr("FileParameterSource::FileParameterSource: Missing closing brace in input file");
	}

void FileParametersSource::read(const char* name,const ReaderBase& value)
	{
	/* Retrieve the named string from the tag/value map: */
	const std::string& valueString=tagValueMap.getEntry(name).getDest();
	
	/* Read the value from the string: */
	value.read(valueString);
	}

void FileParametersSource::readScalarVariable(const char* name,int& scalarVariableIndex)
	{
	/* Retrieve the variable's name from the tag/value map: */
	const std::string& scalarVariableName=tagValueMap.getEntry(name).getDest();
	
	/* Get the variable's index: */
	scalarVariableIndex=variableManager->getScalarVariable(scalarVariableName.c_str());
	}

void FileParametersSource::readVectorVariable(const char* name,int& vectorVariableIndex)
	{
	/* Retrieve the variable's name from the tag/value map: */
	const std::string& vectorVariableName=tagValueMap.getEntry(name).getDest();
	
	/* Get the variable's index: */
	vectorVariableIndex=variableManager->getVectorVariable(vectorVariableName.c_str());
	}

}

}
