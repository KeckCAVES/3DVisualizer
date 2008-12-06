/***********************************************************************
SingleVectorValue - Class for data values of single value vector data
sets.
Copyright (c) 2006-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_SINGLEVECTORVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_SINGLEVECTORVALUE_INCLUDED

#include <string.h>

#include <Templatized/VectorExtractor.h>
#include <Templatized/VectorScalarExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Wrappers {

template <class DSParam,class VScalarParam>
class SingleVectorValue:public DataValue<DSParam,VScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef DataValue<DSParam,VScalarParam> Base;
	typedef typename Base::SE SE;
	typedef typename Base::VE VE;
	
	/* Elements: */
	private:
	char* scalarVariableNames[4]; // Names of the four derived scalar variables
	char* vectorVariableName; // Name of the single vector variable
	
	/* Constructors and destructors: */
	public:
	SingleVectorValue(void) // Dummy constructor
		:vectorVariableName(0)
		{
		for(int i=0;i<4;++i)
			scalarVariableNames[i]=0;
		}
	SingleVectorValue(const char* sVectorVariableName) // Constructor for given name
		:vectorVariableName(0)
		{
		for(int i=0;i<4;++i)
			scalarVariableNames[i]=0;
		setVectorVariableName(sVectorVariableName);
		}
	private:
	SingleVectorValue(const SingleVectorValue& source); // Prohibit copy constructor
	SingleVectorValue& operator=(const SingleVectorValue& source); // Prohibit assignment operator
	public:
	~SingleVectorValue(void)
		{
		for(int i=0;i<4;++i)
			delete[] scalarVariableNames[i];
		delete[] vectorVariableName;
		}
	
	/* Methods: */
	void setVectorVariableName(const char* newVectorVariableName) // Sets the vector variable's name
		{
		/* Delete the old variable names: */
		for(int i=0;i<4;++i)
			delete[] scalarVariableNames[i];
		delete[] vectorVariableName;
		
		/* Create the new derived scalar variable names: */
		size_t nameLen=strlen(newVectorVariableName);
		for(int i=0;i<3;++i)
			{
			scalarVariableNames[i]=new char[nameLen+3];
			memcpy(scalarVariableNames[i],newVectorVariableName,nameLen);
			scalarVariableNames[i][nameLen+0]=' ';
			scalarVariableNames[i][nameLen+1]=char(i+'X');
			scalarVariableNames[i][nameLen+2]='\0';
			}
		scalarVariableNames[3]=new char[nameLen+11];
		memcpy(scalarVariableNames[3],newVectorVariableName,nameLen);
		memcpy(scalarVariableNames[3]+nameLen," Magnitude",10);
		scalarVariableNames[3][nameLen+10]='\0';
		
		/* Store the new vector variable name: */
		vectorVariableName=new char[nameLen+1];
		memcpy(vectorVariableName,newVectorVariableName,nameLen+1);
		}
	int getNumScalarVariables(void) const
		{
		return 4;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		return scalarVariableNames[scalarVariableIndex];
		}
	SE getScalarExtractor(int scalarVariableIndex) const
		{
		return SE(scalarVariableIndex);
		}
	int getNumVectorVariables(void) const
		{
		return 1;
		}
	const char* getVectorVariableName(int) const
		{
		return vectorVariableName;
		}
	VE getVectorExtractor(int) const
		{
		return VE();
		}
	};

}

}

#endif
