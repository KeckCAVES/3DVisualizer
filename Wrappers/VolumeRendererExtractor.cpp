/***********************************************************************
VolumeRendererExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized volume renderer
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

#define VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_IMPLEMENTATION

#include <Wrappers/VolumeRendererExtractor.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>

#include <Abstract/VariableManager.h>
#include <Wrappers/ScalarExtractor.h>

namespace Visualization {

namespace Wrappers {

/****************************************************
Methods of class VolumeRendererExtractor::Parameters:
****************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		scalarVariableIndex=dataSource.template read<int>();
	else
		scalarVariableIndex=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
	sliceFactor=dataSource.template read<Scalar>();
	transparencyGamma=dataSource.template read<float>();
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndex);
	else
		writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndex,variableManager);
	dataSink.template write<Scalar>(sliceFactor);
	dataSink.template write<float>(transparencyGamma);
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
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
		sliceFactor=readParameterAscii<Scalar>(hash,"sliceFactor",sliceFactor);
		transparencyGamma=readParameterAscii<float>(hash,"transparencyGamma",transparencyGamma);
		
		/* Clean up: */
		deleteAsciiParameterFileSectionHash(hash);
		}
	else
		{
		/* Read from binary file: */
		readBinary(file,false,variableManager);
		}
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::ClusterPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read (and ignore) the parameter packet size from the cluster pipe: */
	pipe.read<unsigned int>();
	
	/* Read from cluster pipe: */
	readBinary(pipe,false,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		writeScalarVariableNameAscii<Misc::File>(file,"scalarVariable",scalarVariableIndex,variableManager);
		writeParameterAscii<Misc::File,Scalar>(file,"sliceFactor",sliceFactor);
		writeParameterAscii<Misc::File,float>(file,"transparencyGamma",transparencyGamma);
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
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
VolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	packetSize+=getScalarVariableNameLength(scalarVariableIndex,variableManager);
	packetSize+=sizeof(Scalar)+sizeof(float);
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/************************************************
Static elements of class VolumeRendererExtractor:
************************************************/

template <class DataSetWrapperParam>
const char* VolumeRendererExtractor<DataSetWrapperParam>::name="Volume Renderer";

/****************************************
Methods of class VolumeRendererExtractor:
****************************************/

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::VolumeRendererExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	 Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe),
	 parameters(sVariableManager->getCurrentScalarVariable())
	{
	/* Initialize parameters: */
	parameters.sliceFactor=Scalar(1);
	parameters.transparencyGamma=1.0f;
	}

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::~VolumeRendererExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Create a new volume renderer visualization element: */
	return new VolumeRenderer(this,extractParameters);
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("VolumeRendererExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Create a new volume renderer visualization element: */
	return new VolumeRenderer(this,extractParameters);
	}

}

}
