/***********************************************************************
SingleScalarValue - Class for data values of single value scalar data
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

#ifndef VISUALIZATION_WRAPPERS_SINGLESCALARVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_SINGLESCALARVALUE_INCLUDED

#include <string.h>

#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Wrappers {

template <class DSParam,class VScalarParam>
class SingleScalarValue:public DataValue<DSParam,VScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef DataValue<DSParam,VScalarParam> Base;
	typedef typename Base::SE SE;
	
	/* Elements: */
	private:
	char* scalarVariableName; // Name of the single scalar variable
	
	/* Constructors and destructors: */
	public:
	SingleScalarValue(void) // Dummy constructor
		:scalarVariableName(0)
		{
		}
	SingleScalarValue(const char* sScalarVariableName) // Constructor for given name
		:scalarVariableName(new char[strlen(sScalarVariableName)+1])
		{
		strcpy(scalarVariableName,sScalarVariableName);
		}
	private:
	SingleScalarValue(const SingleScalarValue& source); // Prohibit copy constructor
	SingleScalarValue& operator=(const SingleScalarValue& source); // Prohibit assignment operator
	public:
	~SingleScalarValue(void)
		{
		delete[] scalarVariableName;
		}
	
	/* Methods: */
	void setScalarVariableName(const char* newScalarVariableName) // Sets the scalar variable's name
		{
		delete[] scalarVariableName;
		scalarVariableName=new char[strlen(newScalarVariableName)+1];
		strcpy(scalarVariableName,newScalarVariableName);
		}
	int getNumScalarVariables(void) const
		{
		return 1;
		}
	const char* getScalarVariableName(int) const
		{
		return scalarVariableName;
		}
	SE getScalarExtractor(int) const
		{
		return SE();
		}
	};

}

}

#endif
