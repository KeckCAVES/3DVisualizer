/***********************************************************************
SlicedScalarVectorDataValue - Class for data value descriptors giving
access to multiple scalar and/or vector variables stored in a sliced
data set.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_SLICEDSCALARVECTORDATAVALUE_INCLUDED
#define VISUALIZATION_WRAPPERS_SLICEDSCALARVECTORDATAVALUE_INCLUDED

#include <Templatized/SlicedScalarExtractor.h>
#include <Templatized/SlicedVectorExtractor.h>
#include <Wrappers/DataValue.h>

namespace Visualization {

namespace Wrappers {

class SlicedScalarVectorDataValueBase // Base class managing variable naming and indexing
	{
	/* Elements: */
	private:
	int numScalarVariables; // Number of scalar variables in the sliced data set
	char** scalarVariableNames; // Array of names of the individual scalar variables
	int numVectorComponents; // Dimension of vectors
	int numVectorVariables; // Number of vector variables in the sliced data set
	char** vectorVariableNames; // Array of names of the individual vector variables
	int* vectorVariableScalarIndices; // 2D array of indices of scalar variables defining each vector variable
	
	/* Constructors and destructors: */
	public:
	SlicedScalarVectorDataValueBase(void); // Creates uninitialized data value
	~SlicedScalarVectorDataValueBase(void);
	
	/* Methods: */
	void initialize(int sNumScalarVariables,int sNumVectorComponents,int sNumVectorVariables); // Prepares data value for the given number of scalar and vector variables
	void setScalarVariableName(int scalarVariableIndex,const char* newScalarVariableName); // Sets the given scalar variable's name
	int addScalarVariable(const char* newScalarVariableName); // Adds another scalar variable
	void setVectorVariableName(int vectorVariableIndex,const char* newVectorVariableName); // Sets the given vector variable's name
	int addVectorVariable(const char* newVectorVariableName); // Adds another vector variable
	void setVectorVariableScalarIndex(int vectorVariableIndex,int componentIndex,int scalarVariableIndex); // Sets the index-th component of the given vector variable to the given scalar variable
	int getNumScalarVariables(void) const
		{
		return numScalarVariables;
		}
	const char* getScalarVariableName(int scalarVariableIndex) const
		{
		return scalarVariableNames[scalarVariableIndex];
		}
	int getNumVectorVariables(void) const
		{
		return numVectorVariables;
		}
	const char* getVectorVariableName(int vectorVariableIndex) const
		{
		return vectorVariableNames[vectorVariableIndex];
		}
	int getVectorVariableScalarIndex(int vectorVariableIndex,int componentIndex) const
		{
		return vectorVariableScalarIndices[vectorVariableIndex*numVectorComponents+componentIndex];
		}
	};

template <class DSParam,class VScalarParam>
class SlicedScalarVectorDataValue:public SlicedScalarVectorDataValueBase,public DataValue<DSParam,VScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef DataValue<DSParam,VScalarParam> Base;
	typedef typename Base::DS DS;
	using Base::dimension;
	typedef typename Base::SE SE;
	typedef typename Base::VE VE;
	
	/* Elements: */
	private:
	const DS* dataSet; // Pointer to the data set described by this data value
	
	/* Constructors and destructors: */
	public:
	SlicedScalarVectorDataValue(void) // Creates uninitialized data value
		:dataSet(0)
		{
		}
	private:
	SlicedScalarVectorDataValue(const SlicedScalarVectorDataValue& source); // Prohibit copy constructor
	SlicedScalarVectorDataValue& operator=(const SlicedScalarVectorDataValue& source); // Prohibit assignment operator
	
	/* Methods: */
	public:
	void initialize(const DS* sDataSet,int sNumVectorVariables =0) // Prepares data value for the given data set and number of vector variables (number of scalar variables is taken from data set)
		{
		/* Store the data set: */
		dataSet=sDataSet;
		
		/* Initialize the base class: */
		SlicedScalarVectorDataValueBase::initialize(dataSet->getNumSlices(),dimension,sNumVectorVariables);
		}
	using SlicedScalarVectorDataValueBase::getNumScalarVariables;
	using SlicedScalarVectorDataValueBase::getScalarVariableName;
	using SlicedScalarVectorDataValueBase::getNumVectorVariables;
	using SlicedScalarVectorDataValueBase::getVectorVariableName;
	SE getScalarExtractor(int scalarVariableIndex) const
		{
		return SE(scalarVariableIndex,dataSet->getSliceArray(scalarVariableIndex));
		}
	VE getVectorExtractor(int vectorVariableIndex) const
		{
		VE result;
		for(int i=0;i<dimension;++i)
			result.setSlice(i,dataSet->getSliceArray(getVectorVariableScalarIndex(vectorVariableIndex,i)));
		return result;
		}
	};

}

}

#endif
