/***********************************************************************
SeededSliceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized slice extractor
implementation.
Copyright (c) 2005-2008 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_SEEDEDSLICEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>

#include <Abstract/VariableManager.h>
#include <Templatized/SliceExtractorIndexedTriangleSet.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/SeededSliceExtractor.h>

namespace Visualization {

namespace Wrappers {

/*******************************
Methods of class SliceExtractor:
*******************************/

template <class DataSetWrapperParam>
inline
const typename SeededSliceExtractor<DataSetWrapperParam>::DS*
SeededSliceExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("SeededSliceExtractor::SeededSliceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename SeededSliceExtractor<DataSetWrapperParam>::SE&
SeededSliceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("SeededSliceExtractor::SeededSliceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
SeededSliceExtractor<DataSetWrapperParam>::SeededSliceExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable(),500000),
	 sle(getDs(sVariableManager->getDataSetByScalarVariable(parameters.scalarVariableIndex)),getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex))),
	 currentSlice(0)
	{
	}

template <class DataSetWrapperParam>
inline
SeededSliceExtractor<DataSetWrapperParam>::~SeededSliceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::setMaxNumTriangles(
	size_t newMaxNumTriangles)
	{
	/* Update the extraction parameter: */
	parameters.maxNumTriangles=newMaxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
bool
SeededSliceExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
SeededSliceExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededSliceExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededSliceExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and the slicing plane: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	Plane slicePlane(seedLocator->getOrientation().getDirection(1),seedLocator->getPosition());
	newParameters.plane=slicePlane;
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new slice visualization element: */
	Slice* result=new Slice(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	/* Extract the slice into the visualization element: */
	sle.extractSeededSlice(dsl,slicePlane,result->getSurface());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededSliceExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededSliceExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Calculate the seeding point and the slicing plane: */
	Parameters newParameters=parameters;
	newParameters.seedPoint=seedLocator->getPosition();
	Plane slicePlane(seedLocator->getOrientation().getDirection(1),seedLocator->getPosition());
	newParameters.plane=slicePlane;
	
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		newParameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new slice visualization element: */
	currentSlice=new Slice(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	/* Start extracting the slice into the visualization element: */
	sle.startSeededSlice(dsl,slicePlane,currentSlice->getSurface());
	
	/* Return the result: */
	return currentSlice.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
SeededSliceExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Continue extracting the slice into the visualization element: */
	AlarmTimerElement<Slice> atcf(alarm,*currentSlice,parameters.maxNumTriangles);
	return sle.continueSeededSlice(atcf)||currentSlice->getElementSize()>=parameters.maxNumTriangles;
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	sle.finishSeededSlice();
	currentSlice=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededSliceExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededSliceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Create a new slice visualization element: */
	currentSlice=new Slice(newParameters,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	
	return currentSlice.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("SeededSliceExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentSlice->getSurface().receive();
	}

}

}
