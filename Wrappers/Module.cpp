/***********************************************************************
Module - Wrapper class to combine templatized data set representations
and templatized algorithms into a polymorphic visualization module.
Copyright (c) 2006-2008 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_MODULE_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>

#include <Abstract/VariableManager.h>
#include <Wrappers/DataSet.h>
#include <Wrappers/DataSetRenderer.h>
#include <Wrappers/SeededSliceExtractor.h>
#include <Wrappers/SeededIsosurfaceExtractor.h>
#include <Wrappers/GlobalIsosurfaceExtractor.h>
#include <Wrappers/SeededColoredIsosurfaceExtractor.h>
#include <Wrappers/VolumeRendererExtractor.h>
#include <Wrappers/TripleChannelVolumeRendererExtractor.h>
#include <Wrappers/ArrowRakeExtractor.h>
#include <Wrappers/StreamlineExtractor.h>
#include <Wrappers/MultiStreamlineExtractor.h>
// #include <Wrappers/StreamsurfaceExtractor.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Wrappers {

/***********************
Methods of class Module:
***********************/

template <class DSParam,class DataValueParam>
inline
Module<DSParam,DataValueParam>::Module(
	const char* sClassName)
	:Visualization::Abstract::Module(sClassName)
	{
	}

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::DataSetRenderer*
Module<DSParam,DataValueParam>::getRenderer(
	const Visualization::Abstract::DataSet* dataSet) const
	{
	return new DataSetRenderer(dataSet);
	}

template <class DSParam,class DataValueParam>
inline
int
Module<DSParam,DataValueParam>::getNumScalarAlgorithms(
	void) const
	{
	return 6;
	}

template <class DSParam,class DataValueParam>
inline
const char*
Module<DSParam,DataValueParam>::getScalarAlgorithmName(
	int scalarAlgorithmIndex) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=6)
		Misc::throwStdErr("Module::getAlgorithmName: invalid algorithm index %d",scalarAlgorithmIndex);
	
	const char* result=0;
	switch(scalarAlgorithmIndex)
		{
		case 0:
			result=SeededSliceExtractor::getClassName();
			break;
		
		case 1:
			result=SeededIsosurfaceExtractor::getClassName();
			break;
		
		case 2:
			result=GlobalIsosurfaceExtractor::getClassName();
			break;
		
		case 3:
			result=SeededColoredIsosurfaceExtractor::getClassName();
			break;
		
		case 4:
			result=VolumeRendererExtractor::getClassName();
			break;
		
		case 5:
			result=TripleChannelVolumeRendererExtractor::getClassName();
			break;
		}
	return result;
	}

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::Algorithm*
Module<DSParam,DataValueParam>::getScalarAlgorithm(
	int scalarAlgorithmIndex,
	Visualization::Abstract::VariableManager* variableManager,
	Comm::MulticastPipe* pipe) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=6)
		Misc::throwStdErr("Module::getAlgorithm: invalid algorithm index %d",scalarAlgorithmIndex);
	
	Visualization::Abstract::Algorithm* result=0;
	switch(scalarAlgorithmIndex)
		{
		case 0:
			result=new SeededSliceExtractor(variableManager,pipe);
			break;
		
		case 1:
			result=new SeededIsosurfaceExtractor(variableManager,pipe);
			break;
		
		case 2:
			result=new GlobalIsosurfaceExtractor(variableManager,pipe);
			break;
		
		case 3:
			result=new SeededColoredIsosurfaceExtractor(variableManager,pipe);
			break;
		
		case 4:
			result=new VolumeRendererExtractor(variableManager,pipe);
			break;
		
		case 5:
			result=new TripleChannelVolumeRendererExtractor(variableManager,pipe);
			break;
		}
	return result;
	}

template <class DSParam,class DataValueParam>
inline
int
Module<DSParam,DataValueParam>::getNumVectorAlgorithms(
	void) const
	{
	return 3;
	}

template <class DSParam,class DataValueParam>
inline
const char*
Module<DSParam,DataValueParam>::getVectorAlgorithmName(
	int vectorAlgorithmIndex) const
	{
	if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getVectorAlgorithmName: invalid algorithm index %d",vectorAlgorithmIndex);
	
	const char* result=0;
	switch(vectorAlgorithmIndex)
		{
		case 0:
			result=ArrowRakeExtractor::getClassName();
			break;
		
		case 1:
			result=StreamlineExtractor::getClassName();
			break;
		
		case 2:
			result=MultiStreamlineExtractor::getClassName();
			break;
		
		#if 0
		case 3:
			result=StreamsurfaceExtractor::getClassName();
			break;
		#endif
		}
	return result;
	}

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::Algorithm*
Module<DSParam,DataValueParam>::getVectorAlgorithm(
	int vectorAlgorithmIndex,
	Visualization::Abstract::VariableManager* variableManager,
	Comm::MulticastPipe* pipe) const
	{
	if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getAlgorithm: invalid algorithm index %d",vectorAlgorithmIndex);
	
	Visualization::Abstract::Algorithm* result=0;
	switch(vectorAlgorithmIndex)
		{
		case 0:
			result=new ArrowRakeExtractor(variableManager,pipe);
			break;
		
		case 1:
			result=new StreamlineExtractor(variableManager,pipe);
			break;
		
		case 2:
			result=new MultiStreamlineExtractor(variableManager,pipe);
			break;
		
		#if 0
		case 3:
			result=new StreamsurfaceExtractor(variableManager,pipe);
			break;
		#endif
		}
	return result;
	}

}

}
