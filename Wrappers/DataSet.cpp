/***********************************************************************
DataSet - Wrapper class to map from the abstract data set interface to
its templatized data set implementation.
Copyright (c) 2005-2007 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_DATASET_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>

#include <Templatized/ScalarExtractor.h>
#include <Wrappers/ScalarExtractor.h>
#include <Templatized/VectorExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/CartesianCoordinateTransformer.h>

#include <Wrappers/DataSet.h>

namespace Visualization {

namespace Wrappers {

/*********************************
Methods of class DataSet::Locator:
*********************************/

template <class DSParam,class VScalarParam,class DataValueParam>
inline
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::Locator(
	const typename DataSet<DSParam,VScalarParam,DataValueParam>::DS& ds)
	:dsl(ds.getLocator()),
	 valid(false)
	{
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::Locator(
	const typename DataSet<DSParam,VScalarParam,DataValueParam>::Locator& source)
	:BaseLocator(source),
	 dsl(source.dsl),
	 valid(source.valid)
	{
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
typename DataSet<DSParam,VScalarParam,DataValueParam>::BaseLocator*
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::clone(
	void) const
	{
	return new Locator(*this);
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
bool
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::setPosition(
	const Visualization::Abstract::DataSet::Point& newPosition)
	{
	/* Call the base class method first: */
	bool result=BaseLocator::setPosition(newPosition);
	
	/* Locate the new position in the data set: */
	valid=dsl.locatePoint(newPosition);
	
	return result;
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
typename DataSet<DSParam,VScalarParam,DataValueParam>::DestScalar
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::calcScalar(
	const Visualization::Abstract::ScalarExtractor* scalarExtractor) const
	{
	/* Convert the extractor base class pointer to the proper type: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(scalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("DataSet::Locator::calcScalar: Mismatching scalar extractor type");
	
	/* Check if the locator is valid: */
	if(!valid)
		Misc::throwStdErr("DataSet::Locator::calcScalar: Attempt to evaluate invalid locator");
		
	/* Calculate and return the value: */
	return VScalar(dsl.calcValue(myScalarExtractor->getSe()));
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
typename DataSet<DSParam,VScalarParam,DataValueParam>::DestVector
DataSet<DSParam,VScalarParam,DataValueParam>::Locator::calcVector(
	const Visualization::Abstract::VectorExtractor* vectorExtractor) const
	{
	/* Convert the extractor base class pointer to the proper type: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(vectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("DataSet::Locator::calcVector: Mismatching vector extractor type");
	
	/* Check if the locator is valid: */
	if(!valid)
		Misc::throwStdErr("DataSet::Locator::calcVector: Attempt to evaluate invalid locator");
		
	/* Calculate and return the value: */
	return VVector(dsl.calcValue(myVectorExtractor->getVe()));
	}

/************************
Methods of class DataSet:
************************/

template <class DSParam,class VScalarParam,class DataValueParam>
inline
Visualization::Abstract::CoordinateTransformer*
DataSet<DSParam,VScalarParam,DataValueParam>::getCoordinateTransformer(
	void) const
	{
	return new CartesianCoordinateTransformer;
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
int
DataSet<DSParam,VScalarParam,DataValueParam>::getNumScalarVariables(
	void) const
	{
	return dataValue.getNumScalarVariables();
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
const char*
DataSet<DSParam,VScalarParam,DataValueParam>::getScalarVariableName(
	int scalarVariableIndex) const
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=dataValue.getNumScalarVariables())
		Misc::throwStdErr("DataSet::getScalarVariableName: invalid variable index %d",scalarVariableIndex);
	
	return dataValue.getScalarVariableName(scalarVariableIndex);
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
Visualization::Abstract::ScalarExtractor*
DataSet<DSParam,VScalarParam,DataValueParam>::getScalarExtractor(
	int scalarVariableIndex) const
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=dataValue.getNumScalarVariables())
		Misc::throwStdErr("DataSet::getScalarExtractor: invalid variable index %d",scalarVariableIndex);
	
	return new ScalarExtractor(dataValue.getScalarExtractor(scalarVariableIndex));
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
typename DataSet<DSParam,VScalarParam,DataValueParam>::DestScalarRange
DataSet<DSParam,VScalarParam,DataValueParam>::calcScalarValueRange(
	const Visualization::Abstract::ScalarExtractor* scalarExtractor) const
	{
	/* Convert the extractor base class pointer to the proper type: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(scalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("DataSet::Locator::calcScalar: Mismatching scalar extractor type");
	const SE& se=myScalarExtractor->getSe();
	
	VScalar min,max;
	typename DS::VertexIterator vIt=ds.beginVertices();
	min=max=vIt->getValue(se);
	for(++vIt;vIt!=ds.endVertices();++vIt)
		{
		VScalar v=vIt->getValue(se);
		if(min>v)
			min=v;
		else if(max<v)
			max=v;
		}
	
	return DestScalarRange(min,max);
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
int
DataSet<DSParam,VScalarParam,DataValueParam>::getNumVectorVariables(
	void) const
	{
	return dataValue.getNumVectorVariables();
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
const char*
DataSet<DSParam,VScalarParam,DataValueParam>::getVectorVariableName(
	int vectorVariableIndex) const
	{
	if(vectorVariableIndex<0||vectorVariableIndex>=dataValue.getNumVectorVariables())
		Misc::throwStdErr("DataSet::getVectorVariableName: invalid variable index %d",vectorVariableIndex);
	
	return dataValue.getVectorVariableName(vectorVariableIndex);
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
Visualization::Abstract::VectorExtractor*
DataSet<DSParam,VScalarParam,DataValueParam>::getVectorExtractor(
	int vectorVariableIndex) const
	{
	if(vectorVariableIndex<0||vectorVariableIndex>=dataValue.getNumVectorVariables())
		Misc::throwStdErr("DataSet::getVectorExtractor: invalid variable index %d",vectorVariableIndex);
	
	return new VectorExtractor(dataValue.getVectorExtractor(vectorVariableIndex));
	}

template <class DSParam,class VScalarParam,class DataValueParam>
inline
typename DataSet<DSParam,VScalarParam,DataValueParam>::DestScalarRange
DataSet<DSParam,VScalarParam,DataValueParam>::calcVectorValueMagnitudeRange(
	const Visualization::Abstract::VectorExtractor* vectorExtractor) const
	{
	/* Convert the extractor base class pointer to the proper type: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(vectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("DataSet::Locator::calcVector: Mismatching vector extractor type");
	const VE& ve=myVectorExtractor->getVe();
	
	VScalar min2,max2;
	typename DS::VertexIterator vIt=ds.beginVertices();
	min2=max2=Geometry::sqr(vIt->getValue(ve));
	for(++vIt;vIt!=ds.endVertices();++vIt)
		{
		VScalar v2=Geometry::sqr(vIt->getValue(ve));
		if(min2>v2)
			min2=v2;
		else if(max2<v2)
			max2=v2;
		}
	
	return DestScalarRange(Math::sqrt(min2),Math::sqrt(max2));
	}

}

}
