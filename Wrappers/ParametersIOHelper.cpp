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

#include <ctype.h>
#include <string.h>
#include <string>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StringHashFunctions.h>
#include <Misc/HashTable.h>
#include <Misc/ValueCoder.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Plane.h>
#include <Geometry/GeometryValueCoders.h>

#include <Abstract/VariableManager.h>

#include <Wrappers/ParametersIOHelper.h>

namespace Visualization {

namespace Wrappers {

template <class DataSinkParam,class ValueParam>
void
writeParameterAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	const ValueParam& value)
	{
	dataSink.template write<char>('\t');
	dataSink.template write<char>(tagName,strlen(tagName));
	dataSink.template write<char>(' ');
	std::string valueString=Misc::ValueCoder<ValueParam>::encode(value);
	dataSink.template write<char>(valueString.c_str(),valueString.length());
	dataSink.template write<char>('\n');
	}

template <class DataSinkParam,class ValueParam>
void
writeParameterAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	const ValueParam* values,
	size_t numValues)
	{
	dataSink.template write<char>('\t');
	dataSink.template write<char>(tagName,strlen(tagName));
	dataSink.template write<char>(" (",2);
	for(size_t i=0;i<numValues;++i)
		{
		if(i>0)
			dataSink.template write<char>(", ",2);
		std::string valueString=Misc::ValueCoder<ValueParam>::encode(values[i]);
		dataSink.template write<char>(valueString.c_str(),valueString.length());
		}
	dataSink.template write<char>(")\n",2);
	}

template <class DataSourceParam>
AsciiParameterFileSectionHash*
parseAsciiParameterFileSection(
	DataSourceParam& dataSource)
	{
	Misc::SelfDestructPointer<AsciiParameterFileSectionHash> result(new AsciiParameterFileSectionHash(17));
	
	/* Skip whitespace until the opening brace: */
	char c;
	while(isspace(c=dataSource.template read<char>()))
		;
	
	/* Check for opening brace: */
	if(c!='{')
		Misc::throwStdErr("parseAsciiParameterFileSection: expected '{', got '%c'",c);
	
	/* Read newline-separated tag / value pairs: */
	while(true)
		{
		/* Skip whitespace: */
		while(isspace(c=dataSource.template read<char>()))
			;
		
		/* Bail out on closing brace: */
		if(c=='}')
			break;
		
		/* Read the tag: */
		std::string tag;
		tag.push_back(c);
		while(!isspace(c=dataSource.template read<char>()))
			tag.push_back(c);
		
		/* Skip whitespace but not newline: */
		while(isspace(c=dataSource.template read<char>())&&c!='\n')
			;
		
		/* Read the value: */
		std::string value;
		while(c!='\n')
			{
			value.push_back(c);
			c=dataSource.template read<char>();
			}
		
		/* Store the tag/value pair: */
		result->setEntry(AsciiParameterFileSectionHash::Entry(tag,value));
		}
	
	/* Return the hash table: */
	return result.releaseTarget();
	}

template <class ValueParam>
ValueParam
readParameterAscii(
	const AsciiParameterFileSectionHash* hash,
	std::string tag,
	const ValueParam& defaultValue)
	{
	/* Find the tag in the hash table: */
	AsciiParameterFileSectionHash::ConstIterator it=hash->findEntry(tag);
	if(!it.isFinished())
		{
		/* Extract a value of the required type from the tag's value string: */
		const char* start=it->getDest().data();
		const char* end=start+it->getDest().length();
		return Misc::ValueCoder<ValueParam>::decode(start,end);
		}
	else
		{
		/* Return default value if hash table does not contain tag: */
		return defaultValue;
		}
	}

template <class ValueParam>
void
readParameterAscii(
	const AsciiParameterFileSectionHash* hash,
	std::string tag,
	ValueParam* values,
	size_t numValues)
	{
	/* Find the tag in the hash table: */
	AsciiParameterFileSectionHash::ConstIterator it=hash->findEntry(tag);
	if(!it.isFinished())
		{
		/* Extract an array of values of the required type from the tag's value string: */
		const char* start=it->getDest().data();
		const char* end=start+it->getDest().length();
		Misc::ValueCoderArray<ValueParam>::decode(int(numValues),values,start,end);
		}
	}

int readScalarVariableNameAscii(const AsciiParameterFileSectionHash* hash,std::string tag,const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Find the tag in the hash table: */
	AsciiParameterFileSectionHash::ConstIterator it=hash->findEntry(tag);
	if(!it.isFinished())
		{
		/* Read the stored variable name: */
		const char* start=it->getDest().data();
		const char* end=start+it->getDest().length();
		std::string variableName=Misc::ValueCoder<std::string>::decode(start,end);
		
		/* Find the scalar variable of the given name: */
		int result=-1;
		for(int i=0;i<variableManager->getNumScalarVariables();++i)
			if(strcmp(variableManager->getScalarVariableName(i),variableName.c_str())==0)
				result=i;
		return result;
		}
	else
		{
		/* Return an invalid index: */
		return -1;
		}
	}

int readVectorVariableNameAscii(const AsciiParameterFileSectionHash* hash,std::string tag,const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Find the tag in the hash table: */
	AsciiParameterFileSectionHash::ConstIterator it=hash->findEntry(tag);
	if(!it.isFinished())
		{
		/* Read the stored variable name: */
		const char* start=it->getDest().data();
		const char* end=start+it->getDest().length();
		std::string variableName=Misc::ValueCoder<std::string>::decode(start,end);
		
		/* Find the vector variable of the given name: */
		int result=-1;
		for(int i=0;i<variableManager->getNumVectorVariables();++i)
			if(strcmp(variableManager->getVectorVariableName(i),variableName.c_str())==0)
				result=i;
		return result;
		}
	else
		{
		/* Return an invalid index: */
		return -1;
		}
	}

void deleteAsciiParameterFileSectionHash(AsciiParameterFileSectionHash* hash)
	{
	delete hash;
	}

size_t getScalarVariableNameLength(int scalarVariableIndex,const Visualization::Abstract::VariableManager* variableManager)
	{
	return sizeof(unsigned int)+strlen(variableManager->getScalarVariableName(scalarVariableIndex));
	}

size_t getVectorVariableNameLength(int vectorVariableIndex,const Visualization::Abstract::VariableManager* variableManager)
	{
	return sizeof(unsigned int)+strlen(variableManager->getVectorVariableName(vectorVariableIndex));
	}

template <class DataSourceParam>
int
readScalarVariableNameBinary(
	DataSourceParam& dataSource,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	size_t variableNameLength=dataSource.template read<unsigned int>();
	char variableName[256];
	dataSource.template read<char>(variableName,variableNameLength);
	variableName[variableNameLength]='\0';
	int result=-1;
	for(int i=0;i<variableManager->getNumScalarVariables();++i)
		if(strcmp(variableManager->getScalarVariableName(i),variableName)==0)
			result=i;
	return result;
	}

template <class DataSinkParam>
void
writeScalarVariableNameBinary(
	DataSinkParam& dataSink,
	int scalarVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	const char* variableName=variableManager->getScalarVariableName(scalarVariableIndex);
	size_t variableNameLength=strlen(variableName);
	dataSink.template write<unsigned int>(variableNameLength);
	dataSink.template write<char>(variableName,variableNameLength);
	}

template <class DataSinkParam>
void
writeScalarVariableNameAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	int scalarVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	dataSink.template write<char>('\t');
	dataSink.template write<char>(tagName,strlen(tagName));
	dataSink.template write<char>(" \"",2);
	const char* variableName=variableManager->getScalarVariableName(scalarVariableIndex);
	dataSink.template write<char>(variableName,strlen(variableName));
	dataSink.template write<char>("\"\n",2);
	}

template <class DataSourceParam>
int
readVectorVariableNameBinary(
	DataSourceParam& dataSource,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	size_t variableNameLength=dataSource.template read<unsigned int>();
	char variableName[256];
	dataSource.template read<char>(variableName,variableNameLength);
	variableName[variableNameLength]='\0';
	int result=-1;
	for(int i=0;i<variableManager->getNumVectorVariables();++i)
		if(strcmp(variableManager->getVectorVariableName(i),variableName)==0)
			result=i;
	return result;
	}

template <class DataSinkParam>
void
writeVectorVariableNameBinary(
	DataSinkParam& dataSink,
	int vectorVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	const char* variableName=variableManager->getVectorVariableName(vectorVariableIndex);
	size_t variableNameLength=strlen(variableName);
	dataSink.template write<unsigned int>(variableNameLength);
	dataSink.template write<char>(variableName,variableNameLength);
	}

template <class DataSinkParam>
void
writeVectorVariableNameAscii(
	DataSinkParam& dataSink,
	const char* tagName,
	int vectorVariableIndex,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	dataSink.template write<char>('\t');
	dataSink.template write<char>(tagName,strlen(tagName));
	dataSink.template write<char>(" \"",2);
	const char* variableName=variableManager->getVectorVariableName(vectorVariableIndex);
	dataSink.template write<char>(variableName,strlen(variableName));
	dataSink.template write<char>("\"\n",2);
	}

/*********************************************
Force instantiation of all standard functions:
*********************************************/

template void writeParameterAscii<Misc::File,int>(Misc::File&,const char*,const int&);
template void writeParameterAscii<Misc::File,unsigned int>(Misc::File&,const char*,const unsigned int&);
template void writeParameterAscii<Misc::File,float>(Misc::File&,const char*,const float&);
template void writeParameterAscii<Misc::File,double>(Misc::File&,const char*,const double&);
template void writeParameterAscii<Misc::File,Geometry::Point<float,3> >(Misc::File&,const char*,const Geometry::Point<float,3>&);
template void writeParameterAscii<Misc::File,Geometry::Point<double,3> >(Misc::File&,const char*,const Geometry::Point<double,3>&);
template void writeParameterAscii<Misc::File,Geometry::Vector<float,3> >(Misc::File&,const char*,const Geometry::Vector<float,3>&);
template void writeParameterAscii<Misc::File,Geometry::Vector<double,3> >(Misc::File&,const char*,const Geometry::Vector<double,3>&);
template void writeParameterAscii<Misc::File,Geometry::Plane<float,3> >(Misc::File&,const char*,const Geometry::Plane<float,3>&);
template void writeParameterAscii<Misc::File,Geometry::Plane<double,3> >(Misc::File&,const char*,const Geometry::Plane<double,3>&);

template void writeParameterAscii<Misc::File,int>(Misc::File&,const char*,const int*,size_t);
template void writeParameterAscii<Misc::File,unsigned int>(Misc::File&,const char*,const unsigned int*,size_t);
template void writeParameterAscii<Misc::File,float>(Misc::File&,const char*,const float*,size_t);
template void writeParameterAscii<Misc::File,double>(Misc::File&,const char*,const double*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Point<float,3> >(Misc::File&,const char*,const Geometry::Point<float,3>*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Point<double,3> >(Misc::File&,const char*,const Geometry::Point<double,3>*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Vector<float,3> >(Misc::File&,const char*,const Geometry::Vector<float,3>*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Vector<double,3> >(Misc::File&,const char*,const Geometry::Vector<double,3>*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Plane<float,3> >(Misc::File&,const char*,const Geometry::Plane<float,3>*,size_t);
template void writeParameterAscii<Misc::File,Geometry::Plane<double,3> >(Misc::File&,const char*,const Geometry::Plane<double,3>*,size_t);

template AsciiParameterFileSectionHash* parseAsciiParameterFileSection<Misc::File>(Misc::File&);

template int readScalarVariableNameBinary<Misc::File>(Misc::File&,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameBinary<Misc::File>(Misc::File&,int,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameAscii<Misc::File>(Misc::File&,const char*,int,const Visualization::Abstract::VariableManager*);

template int readVectorVariableNameBinary<Misc::File>(Misc::File&,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameBinary<Misc::File>(Misc::File&,int,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameAscii<Misc::File>(Misc::File&,const char*,int,const Visualization::Abstract::VariableManager*);

template void writeParameterAscii<Comm::MulticastPipe,int>(Comm::MulticastPipe&,const char*,const int&);
template void writeParameterAscii<Comm::MulticastPipe,unsigned int>(Comm::MulticastPipe&,const char*,const unsigned int&);
template void writeParameterAscii<Comm::MulticastPipe,float>(Comm::MulticastPipe&,const char*,const float&);
template void writeParameterAscii<Comm::MulticastPipe,double>(Comm::MulticastPipe&,const char*,const double&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Point<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Point<float,3>&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Point<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Point<double,3>&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Vector<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Vector<float,3>&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Vector<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Vector<double,3>&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Plane<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Plane<float,3>&);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Plane<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Plane<double,3>&);

template void writeParameterAscii<Comm::MulticastPipe,int>(Comm::MulticastPipe&,const char*,const int*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,unsigned int>(Comm::MulticastPipe&,const char*,const unsigned int*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,float>(Comm::MulticastPipe&,const char*,const float*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,double>(Comm::MulticastPipe&,const char*,const double*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Point<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Point<float,3>*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Point<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Point<double,3>*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Vector<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Vector<float,3>*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Vector<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Vector<double,3>*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Plane<float,3> >(Comm::MulticastPipe&,const char*,const Geometry::Plane<float,3>*,size_t);
template void writeParameterAscii<Comm::MulticastPipe,Geometry::Plane<double,3> >(Comm::MulticastPipe&,const char*,const Geometry::Plane<double,3>*,size_t);

template AsciiParameterFileSectionHash* parseAsciiParameterFileSection<Comm::MulticastPipe>(Comm::MulticastPipe&);

template int readScalarVariableNameBinary<Comm::MulticastPipe>(Comm::MulticastPipe&,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameBinary<Comm::MulticastPipe>(Comm::MulticastPipe&,int,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameAscii<Comm::MulticastPipe>(Comm::MulticastPipe&,const char*,int,const Visualization::Abstract::VariableManager*);

template int readVectorVariableNameBinary<Comm::MulticastPipe>(Comm::MulticastPipe&,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameBinary<Comm::MulticastPipe>(Comm::MulticastPipe&,int,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameAscii<Comm::MulticastPipe>(Comm::MulticastPipe&,const char*,int,const Visualization::Abstract::VariableManager*);

template void writeParameterAscii<Comm::ClusterPipe,int>(Comm::ClusterPipe&,const char*,const int&);
template void writeParameterAscii<Comm::ClusterPipe,unsigned int>(Comm::ClusterPipe&,const char*,const unsigned int&);
template void writeParameterAscii<Comm::ClusterPipe,float>(Comm::ClusterPipe&,const char*,const float&);
template void writeParameterAscii<Comm::ClusterPipe,double>(Comm::ClusterPipe&,const char*,const double&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Point<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Point<float,3>&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Point<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Point<double,3>&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Vector<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Vector<float,3>&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Vector<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Vector<double,3>&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Plane<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Plane<float,3>&);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Plane<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Plane<double,3>&);

template void writeParameterAscii<Comm::ClusterPipe,int>(Comm::ClusterPipe&,const char*,const int*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,unsigned int>(Comm::ClusterPipe&,const char*,const unsigned int*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,float>(Comm::ClusterPipe&,const char*,const float*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,double>(Comm::ClusterPipe&,const char*,const double*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Point<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Point<float,3>*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Point<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Point<double,3>*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Vector<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Vector<float,3>*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Vector<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Vector<double,3>*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Plane<float,3> >(Comm::ClusterPipe&,const char*,const Geometry::Plane<float,3>*,size_t);
template void writeParameterAscii<Comm::ClusterPipe,Geometry::Plane<double,3> >(Comm::ClusterPipe&,const char*,const Geometry::Plane<double,3>*,size_t);

template AsciiParameterFileSectionHash* parseAsciiParameterFileSection<Comm::ClusterPipe>(Comm::ClusterPipe&);

template int readScalarVariableNameBinary<Comm::ClusterPipe>(Comm::ClusterPipe&,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameBinary<Comm::ClusterPipe>(Comm::ClusterPipe&,int,const Visualization::Abstract::VariableManager*);
template void writeScalarVariableNameAscii<Comm::ClusterPipe>(Comm::ClusterPipe&,const char*,int,const Visualization::Abstract::VariableManager*);

template int readVectorVariableNameBinary<Comm::ClusterPipe>(Comm::ClusterPipe&,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameBinary<Comm::ClusterPipe>(Comm::ClusterPipe&,int,const Visualization::Abstract::VariableManager*);
template void writeVectorVariableNameAscii<Comm::ClusterPipe>(Comm::ClusterPipe&,const char*,int,const Visualization::Abstract::VariableManager*);

template int readParameterAscii<int>(const AsciiParameterFileSectionHash*,std::string,const int&);
template unsigned int readParameterAscii<unsigned int>(const AsciiParameterFileSectionHash*,std::string,const unsigned int&);
template float readParameterAscii<float>(const AsciiParameterFileSectionHash*,std::string,const float&);
template double readParameterAscii<double>(const AsciiParameterFileSectionHash*,std::string,const double&);
template Geometry::Point<float,3> readParameterAscii<Geometry::Point<float,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Point<float,3>&);
template Geometry::Point<double,3> readParameterAscii<Geometry::Point<double,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Point<double,3>&);
template Geometry::Vector<float,3> readParameterAscii<Geometry::Vector<float,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Vector<float,3>&);
template Geometry::Vector<double,3> readParameterAscii<Geometry::Vector<double,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Vector<double,3>&);
template Geometry::Plane<float,3> readParameterAscii<Geometry::Plane<float,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Plane<float,3>&);
template Geometry::Plane<double,3> readParameterAscii<Geometry::Plane<double,3> >(const AsciiParameterFileSectionHash*,std::string,const Geometry::Plane<double,3>&);

template void readParameterAscii<int>(const AsciiParameterFileSectionHash*,std::string,int*,size_t);
template void readParameterAscii<unsigned int>(const AsciiParameterFileSectionHash*,std::string,unsigned int*,size_t);
template void readParameterAscii<float>(const AsciiParameterFileSectionHash*,std::string,float*,size_t);
template void readParameterAscii<double>(const AsciiParameterFileSectionHash*,std::string,double*,size_t);
template void readParameterAscii<Geometry::Point<float,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Point<float,3>*,size_t);
template void readParameterAscii<Geometry::Point<double,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Point<double,3>*,size_t);
template void readParameterAscii<Geometry::Vector<float,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Vector<float,3>*,size_t);
template void readParameterAscii<Geometry::Vector<double,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Vector<double,3>*,size_t);
template void readParameterAscii<Geometry::Plane<float,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Plane<float,3>*,size_t);
template void readParameterAscii<Geometry::Plane<double,3> >(const AsciiParameterFileSectionHash*,std::string,Geometry::Plane<double,3>*,size_t);

}

}
