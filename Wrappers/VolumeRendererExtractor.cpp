/***********************************************************************
VolumeRendererExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized volume renderer
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

#define VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>

#include <Abstract/VariableManager.h>
#include <Wrappers/ScalarExtractor.h>

#include <Wrappers/VolumeRendererExtractor.h>

namespace Visualization {

namespace Wrappers {

/****************************************
Methods of class VolumeRendererExtractor:
****************************************/

template <class DataSetWrapperParam>
inline
const typename VolumeRendererExtractor<DataSetWrapperParam>::DS*
VolumeRendererExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("VolumeRendererExtractor::VolumeRendererExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename VolumeRendererExtractor<DataSetWrapperParam>::SE&
VolumeRendererExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("VolumeRendererExtractor::VolumeRendererExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::VolumeRendererExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	 Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable(),typename DS::Scalar(2),1.0f),
	 ds(getDs(sVariableManager->getDataSetByScalarVariable(parameters.scalarVariableIndex))),
	 se(getSe(sVariableManager->getScalarExtractor(parameters.scalarVariableIndex)))
	{
	}

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::~VolumeRendererExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
bool
VolumeRendererExtractor<DataSetWrapperParam>::hasGlobalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::createElement(
	void)
	{
	/* Send the parameters to all slaves: */
	if(getPipe()!=0)
		{
		parameters.write(*getPipe());
		getPipe()->finishMessage();
		}
	
	/* Create a new volume renderer visualization element: */
	VolumeRenderer* result=new VolumeRenderer(parameters,ds,se,getVariableManager()->getColorMap(parameters.scalarVariableIndex),getPipe());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("VolumeRendererExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Read all parameters from the master: */
	Parameters newParameters=parameters;
	newParameters.read(*getPipe());
	
	/* Create a new volume renderer visualization element: */
	return new VolumeRenderer(newParameters,ds,se,getVariableManager()->getColorMap(newParameters.scalarVariableIndex),getPipe());
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("VolumeRendererExtractor::continueSlaveElement: Cannot be called on master node");
	
	/* It's really a dummy function; startSlaveElement() does all the work... */
	}

}

}
