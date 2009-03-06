/***********************************************************************
ParametersIOHelper - Helper functions to read / write visualization
algorithm parameters from / to ASCII streams.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_PARAMETERSIOHELPER_INCLUDED
#define VISUALIZATION_WRAPPERS_PARAMETERSIOHELPER_INCLUDED

#include <string>

/* Forward declarations: */
namespace Misc {
template <class Source>
class StandardHashFunction;
template <class Source,class Dest,class HashFunction>
class HashTable;
}
namespace Visualization {
namespace Abstract {
class VariableManager;
}
}

namespace Visualization {

namespace Wrappers {

/*****************
Type declarations:
*****************/

typedef Misc::HashTable<std::string,std::string,Misc::StandardHashFunction<std::string> > AsciiParameterFileSectionHash; // Data type to map setting names in parameter file sections to string values

/**********************************
Functions to read/write parameters:
**********************************/

template <class DataSinkParam,class ValueParam>
void
writeParameterAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	const ValueParam& value);

template <class DataSinkParam,class ValueParam>
void
writeParameterAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	const ValueParam* values,
	size_t numValues);

template <class DataSourceParam>
AsciiParameterFileSectionHash*
parseAsciiParameterFileSection(
	DataSourceParam& dataSource);

template <class ValueParam>
ValueParam
readParameterAscii(
	const AsciiParameterFileSectionHash* hash,
	std::string tag,
	const ValueParam& defaultValue);

template <class ValueParam>
void
readParameterAscii(
	const AsciiParameterFileSectionHash* hash,
	std::string tag,
	ValueParam* values,
	size_t numValues);

int readScalarVariableNameAscii(const AsciiParameterFileSectionHash* hash,std::string tag,const Visualization::Abstract::VariableManager* variableManager);
int readVectorVariableNameAscii(const AsciiParameterFileSectionHash* hash,std::string tag,const Visualization::Abstract::VariableManager* variableManager);
void deleteAsciiParameterFileSectionHash(AsciiParameterFileSectionHash* hash);
size_t getScalarVariableNameLength(int scalarVariableIndex,const Visualization::Abstract::VariableManager* variableManager);
size_t getVectorVariableNameLength(int vectorVariableIndex,const Visualization::Abstract::VariableManager* variableManager);

template <class DataSourceParam>
int
readScalarVariableNameBinary(
	DataSourceParam& dataSource,
	const Visualization::Abstract::VariableManager* variableManager);

template <class DataSinkParam>
void
writeScalarVariableNameBinary(
	DataSinkParam& dataSink,
	int scalarVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager);

template <class DataSinkParam>
void
writeScalarVariableNameAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	int scalarVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager);

template <class DataSourceParam>
int
readVectorVariableNameBinary(
	DataSourceParam& dataSource,
	const Visualization::Abstract::VariableManager* variableManager);

template <class DataSinkParam>
void
writeVectorVariableNameBinary(
	DataSinkParam& dataSink,
	int vectorVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager);

template <class DataSinkParam>
void
writeVectorVariableNameAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	int vectorVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager);

}

}

#endif
