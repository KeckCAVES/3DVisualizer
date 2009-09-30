/***********************************************************************
TripleChannelTripleChannelVolumeRendererExtractor - Wrapper class to map from the
abstract visualization algorithm interface to a templatized volume
renderer implementation.
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

#define VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDEREREXTRACTOR_IMPLEMENTATION

#include <Wrappers/TripleChannelVolumeRendererExtractor.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Comm/ClusterPipe.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/RowColumn.h>

#include <Abstract/VariableManager.h>
#include <Wrappers/ScalarExtractor.h>

namespace Visualization {

namespace Wrappers {

/*****************************************************************
Methods of class TripleChannelVolumeRendererExtractor::Parameters:
*****************************************************************/

template <class DataSetWrapperParam>
template <class DataSourceParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::readBinary(
	DataSourceParam& dataSource,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read all elements: */
	if(raw)
		dataSource.template read<int>(scalarVariableIndices,3);
	else
		{
		for(int channel=0;channel<3;++channel)
			scalarVariableIndices[channel]=readScalarVariableNameBinary<DataSourceParam>(dataSource,variableManager);
		}
	sliceFactor=dataSource.template read<Scalar>();
	for(int channel=0;channel<3;++channel)
		channelEnableds[channel]=dataSource.template read<unsigned char>()!=0;
	dataSource.template read<float>(transparencyGammas,3);
	}

template <class DataSetWrapperParam>
template <class DataSinkParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::writeBinary(
	DataSinkParam& dataSink,
	bool raw,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write all elements: */
	if(raw)
		dataSink.template write<int>(scalarVariableIndices,3);
	else
		{
		for(int channel=0;channel<3;++channel)
			writeScalarVariableNameBinary<DataSinkParam>(dataSink,scalarVariableIndices[channel],variableManager);
		}
	dataSink.template write<Scalar>(sliceFactor);
	for(int channel=0;channel<3;++channel)
		dataSink.template write<unsigned char>(channelEnableds[channel]?1:0);
	dataSink.template write<float>(transparencyGammas,3);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
	Misc::File& file,
	bool ascii,
	Visualization::Abstract::VariableManager* variableManager)
	{
	if(ascii)
		{
		/* Parse the parameter section: */
		AsciiParameterFileSectionHash* hash=parseAsciiParameterFileSection<Misc::File>(file);
		
		/* Extract the parameters: */
		for(int channel=0;channel<3;++channel)
			{
			char name[40];
			snprintf(name,sizeof(name),"scalarVariable%d",channel);
			scalarVariableIndices[channel]=readScalarVariableNameAscii(hash,name,variableManager);
			}
		sliceFactor=readParameterAscii<Scalar>(hash,"sliceFactor",sliceFactor);
		for(int channel=0;channel<3;++channel)
			{
			char name[40];
			snprintf(name,sizeof(name),"channelEnabled%d",channel);
			channelEnableds[channel]=readParameterAscii<int>(hash,name,channelEnableds[channel]?1:0)!=0;
			snprintf(name,sizeof(name),"transparencyGamma%d",channel);
			transparencyGammas[channel]=readParameterAscii<float>(hash,name,transparencyGammas[channel]);
			}
		
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
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
	Comm::MulticastPipe& pipe,
	Visualization::Abstract::VariableManager* variableManager)
	{
	/* Read from multicast pipe: */
	readBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::read(
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
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Misc::File& file,
	bool ascii,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Write to ASCII file: */
		file.write("{\n",2);
		for(int channel=0;channel<3;++channel)
			{
			char name[40];
			snprintf(name,sizeof(name),"scalarVariable%d",channel);
			writeScalarVariableNameAscii<Misc::File>(file,name,scalarVariableIndices[channel],variableManager);
			}
		writeParameterAscii<Misc::File,Scalar>(file,"sliceFactor",sliceFactor);
		for(int channel=0;channel<3;++channel)
			{
			char name[40];
			snprintf(name,sizeof(name),"channelEnabled%d",channel);
			writeParameterAscii<Misc::File,int>(file,name,channelEnableds[channel]?1:0);
			snprintf(name,sizeof(name),"transparencyGamma%d",channel);
			writeParameterAscii<Misc::File,float>(file,name,transparencyGammas[channel]);
			}
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
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::MulticastPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Write to multicast pipe: */
	writeBinary(pipe,true,variableManager);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::Parameters::write(
	Comm::ClusterPipe& pipe,
	const Visualization::Abstract::VariableManager* variableManager) const
	{
	/* Calculate the byte size of the marshalled parameter packet: */
	size_t packetSize=0;
	for(int channel=0;channel<3;++channel)
		packetSize+=getScalarVariableNameLength(scalarVariableIndices[channel],variableManager);
	packetSize+=sizeof(Scalar)+sizeof(unsigned char)*3+sizeof(float)*3;
	
	/* Write the packet size to the cluster pipe: */
	pipe.write<unsigned int>(packetSize);
	
	/* Write to cluster pipe: */
	writeBinary(pipe,false,variableManager);
	}

/*************************************************************
Static elements of class TripleChannelVolumeRendererExtractor:
*************************************************************/

template <class DataSetWrapperParam>
const char* TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::name="Triple-Channel Volume Renderer";

/*****************************************************
Methods of class TripleChannelVolumeRendererExtractor:
*****************************************************/

template <class DataSetWrapperParam>
inline
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::TripleChannelVolumeRendererExtractor(
	Visualization::Abstract::VariableManager* sVariableManager,
	 Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sVariableManager,sPipe)
	{
	/* Initialize parameters: */
	for(int channel=0;channel<3;++channel)
		parameters.scalarVariableIndices[channel]=(sVariableManager->getCurrentScalarVariable()+channel)%sVariableManager->getNumScalarVariables();
	parameters.sliceFactor=Scalar(1);
	for(int channel=0;channel<3;++channel)
		{
		parameters.channelEnableds[channel]=true;
		parameters.transparencyGammas[channel]=1.0f;
		}
	
	/* Initialize the UI components: */
	for(int channel=0;channel<3;++channel)
		scalarVariableBoxes[channel]=0;
	}

template <class DataSetWrapperParam>
inline
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::~TripleChannelVolumeRendererExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("TripleChannelVolumeRendererExtractorSettingsDialogPopup",widgetManager,"Triple-Channel Volume Renderer Extractor Settings");
	settingsDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("SettingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(2);
	
	std::vector<std::string> scalarVariables;
	for(int i=0;i<getVariableManager()->getNumScalarVariables();++i)
		scalarVariables.push_back(getVariableManager()->getScalarVariableName(i));
	for(int channel=0;channel<3;++channel)
		{
		char name[40],label[20];
		snprintf(name,sizeof(name),"ScalarVariableLabel%d",channel);
		snprintf(label,sizeof(label),"Scalar Channel %d",channel+1);
		new GLMotif::Label(name,settingsDialog,label);
		
		snprintf(name,sizeof(name),"ScalarVariableMargin%d",channel);
		GLMotif::Margin* scalarVariableMargin=new GLMotif::Margin(name,settingsDialog,false);
		scalarVariableMargin->setAlignment(GLMotif::Alignment::LEFT);
		
		snprintf(name,sizeof(name),"ScalarVariableBox%d",channel);
		scalarVariableBoxes[channel]=new GLMotif::DropdownBox(name,scalarVariableMargin,scalarVariables);
		scalarVariableBoxes[channel]->setSelectedItem(parameters.scalarVariableIndices[channel]);
		scalarVariableBoxes[channel]->getValueChangedCallbacks().add(this,&TripleChannelVolumeRendererExtractor::scalarVariableBoxCallback);
		
		scalarVariableMargin->manageChild();
		}
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::createElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	/* Create a new volume renderer visualization element: */
	return new TripleChannelVolumeRenderer(this,extractParameters);
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::startSlaveElement(
	Visualization::Abstract::Parameters* extractParameters)
	{
	if(isMaster())
		Misc::throwStdErr("TripleChannelVolumeRendererExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Create a new volume renderer visualization element: */
	return new TripleChannelVolumeRenderer(this,extractParameters);
	}

template <class DataSetWrapperParam>
inline
void
TripleChannelVolumeRendererExtractor<DataSetWrapperParam>::scalarVariableBoxCallback(
	GLMotif::DropdownBox::ValueChangedCallbackData* cbData)
	{
	/* Find the index of the dropdown box: */
	int channel;
	for(channel=0;channel<3&&scalarVariableBoxes[channel]!=cbData->dropdownBox;++channel)
		;
	if(channel<3)
		{
		/* Get the new selected item: */
		parameters.scalarVariableIndices[channel]=cbData->newSelectedItem;
		}
	}

}

}
