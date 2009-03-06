/***********************************************************************
SeededSliceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized slice extractor
implementation.
Copyright (c) 2005-2009 Oliver Kreylos

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
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>

#include <Abstract/VariableManager.h>
#include <Templatized/SliceExtractorIndexedTriangleSet.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/ElementSizeLimit.h>
#include <Wrappers/AlarmTimer.h>
#include <Wrappers/ParametersIOHelper.h>

#include <Wrappers/SeededSliceExtractor.h>

namespace Visualization {

namespace Wrappers {

/*************************************************
Methods of class SeededSliceExtractor::Parameters:
*************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		scalarVariableIndex=dataSource.template read<int>();
	else
		scalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	typename Plane::Vector normal;
	dataSource.template read<Scalar>(normal.getComponents(),dimension);
	Scalar offset=dataSource.template read<Scalar>();
	plane=Plane(normal,offset);
	dataSource.template read<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndex,variableManager);
	dataSink.template write<Scalar>(plane.getNormal().getComponents(),dimension);
	dataSink.template write<Scalar>(plane.getOffset());
	dataSink.template write<Scalar>(seedPoint.getComponents(),dimension);
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::read(
	Misc::File& file,
	bool ascii,
	Visualization::Abstract::VariableManager* variableManager)
	{
	if(ascii)
		{
		/* Parse the parameter section: */
		AsciiParameterFileSectionHash* hash=parseAsciiParameterFileSection<Misc::File>(file);
		
		/* Extract the parameters: */
		scalarVariableIndex=readScalarVariableNameAscii(hash,"scalarVariable",variableManager);
		plane=readParameterAscii<Plane>(hash,"plane",plane);
		seedPoint=readParameterAscii<Point>(hash,"seedPoint",seedPoint);
		
		/* Clean up: */
		deleteAsciiParameterFileSectionHash(hash);
		}
	else
		{
		/* Read from binary file: */
		readBinary(file,false,variableManager);
		}
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededSliceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededSliceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::ClusterPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read (and ignore) the parameter packet size from the cluster pipe: */
	pipe.read<unsigned int>();
	
	/* Read from cluster pipe: */
	readBinary(pipe,false,variableManager);
	
	/* Get a templatized locator to track the seed point: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(variableManager->getDataSetByScalarVariable(scalarVariableIndex));
	if(myDataSet==0)
		Misc::throwStdErr("SeededSliceExtractor::Parameters::readBinary: Mismatching data set type");
	dsl=myDataSet->getDs().getLocator();
	locatorValid=dsl.locatePoint(seedPoint);
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeScalarVariableNameAscii<Misc::File>(file,"scalarVariable",scalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,Plane>(file,"plane",plane);
		writeParameterAscii<Misc::File,Point>(file,"seedPoint",seedPoint);
		file.write("}\n",2);
		}
	else
		{
		/* Write to binary file: */
		writeBinary(file,false,variableManager);
		}
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getScalarVariableNameLength(scalarVariableIndex,variableManager);
	packetSize+=sizeof(Scalar)*dimension+sizeof(Scalar);
	packetSize+=sizeof(Scalar)*dimension;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/*********************************************
Static elements of class SeededSliceExtractor:
*********************************************/

template <class DataSetWrapperParam>
const char* SeededSliceExtractor<DataSetWrapperParam>::name="Seeded Slice";

/*************************************
Methods of class SeededSliceExtractor:
*************************************/

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
	 parameters(getVariableManager()->getCurrentScalarVariable()),
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
SeededSliceExtractor<DataSetWrapperParam>::setSeedLocator(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("SeededSliceExtractor::setSeedLocator: Mismatching locator type");
	
	/* Calculate the seeding point and the slicing plane: */
	parameters.seedPoint=seedLocator->getPosition();
	parameters.plane=Plane(seedLocator->getOrientation().getDirection(1),parameters.seedPoint);
	
	/* Copy the locator: */
	parameters.dsl=myLocator->getDsl();
	parameters.locatorValid=myLocator->isValid();
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededSliceExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededSliceExtractor::createElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	
	/* Create a new slice visualization element: */
	Slice* result=new Slice(myParameters,getVariableManager()->getColorMap(svi),getPipe());
	
	/* Update the slice extractor: */
	sle.update(getDs(getVariableManager()->getDataSetByScalarVariable(svi)),getSe(getVariableManager()->getScalarExtractor(svi)));
	
	/* Extract the slice into the visualization element: */
	sle.startSeededSlice(myParameters->dsl,myParameters->plane,result->getSurface());
	ElementSizeLimit<Slice> esl(*result,~size_t(0));
	sle.continueSeededSlice(esl);
	sle.finishSeededSlice();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
SeededSliceExtractor<DataSetWrapperParam>::startElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededSliceExtractor::startElement: Mismatching parameter object type");
	int svi=myParameters->scalarVariableIndex;
	
	/* Create a new slice visualization element: */
	currentSlice=new Slice(myParameters,getVariableManager()->getColorMap(svi),getPipe());
	
	/* Update the slice extractor: */
	sle.update(getDs(getVariableManager()->getDataSetByScalarVariable(svi)),getSe(getVariableManager()->getScalarExtractor(svi)));
	
	/* Start extracting the slice into the visualization element: */
	sle.startSeededSlice(myParameters->dsl,myParameters->plane,currentSlice->getSurface());
	
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
	AlarmTimer atcf(alarm);
	return sle.continueSeededSlice(atcf);
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
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("SeededSliceExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Get proper pointer to parameter object: */
	Parameters* myParameters=dynamic_cast<Parameters*>(extractParameters);
	if(myParameters==0)
		Misc::throwStdErr("SeededSliceExtractor::startSlaveElement: Mismatching parameter object type");
	
	/* Create a new slice visualization element: */
	currentSlice=new Slice(myParameters,getVariableManager()->getColorMap(myParameters->scalarVariableIndex),getPipe());
	
	return currentSlice.getPointer();
	}

template <class DataSetWrapperParam>
inline
void
SeededSliceExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(isMaster())
		Misc::throwStdErr("SeededSliceExtractor::continueSlaveElement: Cannot be called on master node");
	
	currentSlice->getSurface().receive();
	}

}

}
