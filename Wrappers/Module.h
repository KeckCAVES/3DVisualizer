/***********************************************************************
Module - Wrapper class to combine templatized data set representations
and templatized algorithms into a polymorphic visualization module.
Copyright (c) 2006-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_MODULE_INCLUDED
#define VISUALIZATION_WRAPPERS_MODULE_INCLUDED

#include <Abstract/Module.h>

/* Forward declarations: */
namespace Visualization {
namespace Wrappers {
template <class DSParam,class VScalarParam,class DataValueParam>
class DataSet;
template <class DataSetWrapperParam>
class DataSetRenderer;
template <class DataSetWrapperParam>
class SeededSliceExtractor;
template <class DataSetWrapperParam>
class SeededIsosurfaceExtractor;
template <class DataSetWrapperParam>
class GlobalIsosurfaceExtractor;
template <class DataSetWrapperParam>
class SeededColoredIsosurfaceExtractor;
template <class DataSetWrapperParam>
class VolumeRendererExtractor;
template <class DataSetWrapperParam>
class TripleChannelVolumeRendererExtractor;
template <class DataSetWrapperParam>
class ArrowRakeExtractor;
template <class DataSetWrapperParam>
class StreamlineExtractor;
template <class DataSetWrapperParam>
class MultiStreamlineExtractor;
template <class DataSetWrapperParam>
class StreamsurfaceExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DSParam,class DataValueParam>
class Module:public Visualization::Abstract::Module
	{
	/* Embedded classes: */
	public:
	typedef DSParam DS; // Templatized data set type
	typedef DataValueParam DataValue; // Data value descriptor type
	typedef typename DataValue::VScalar VScalar; // Data type for scalar values
	typedef Visualization::Wrappers::DataSet<DS,VScalar,DataValue> DataSet; // Data set class
	typedef Visualization::Wrappers::DataSetRenderer<DataSet> DataSetRenderer; // Data set renderer class
	typedef Visualization::Wrappers::SeededSliceExtractor<DataSet> SeededSliceExtractor; // Slice extractor class
	typedef Visualization::Wrappers::SeededIsosurfaceExtractor<DataSet> SeededIsosurfaceExtractor; // Seeded isosurface extractor class
	typedef Visualization::Wrappers::GlobalIsosurfaceExtractor<DataSet> GlobalIsosurfaceExtractor; // Global isosurface extractor class
	typedef Visualization::Wrappers::SeededColoredIsosurfaceExtractor<DataSet> SeededColoredIsosurfaceExtractor; // Colored seeded isosurface extractor class
	typedef Visualization::Wrappers::VolumeRendererExtractor<DataSet> VolumeRendererExtractor; // Volume renderer extractor class
	typedef Visualization::Wrappers::TripleChannelVolumeRendererExtractor<DataSet> TripleChannelVolumeRendererExtractor; // Volume renderer extractor class with three scalar channels
	typedef Visualization::Wrappers::ArrowRakeExtractor<DataSet> ArrowRakeExtractor; // Arrow rake extractor class
	typedef Visualization::Wrappers::StreamlineExtractor<DataSet> StreamlineExtractor; // Streamline extractor class
	typedef Visualization::Wrappers::MultiStreamlineExtractor<DataSet> MultiStreamlineExtractor; // Streamline bundle extractor class
	typedef Visualization::Wrappers::StreamsurfaceExtractor<DataSet> StreamsurfaceExtractor; // Stream surface extractor class
	
	/* Constructors and destructors: */
	Module(const char* sClassName);
	
	/* Methods: */
	virtual Visualization::Abstract::DataSetRenderer* getRenderer(const Visualization::Abstract::DataSet* dataSet) const;
	virtual int getNumScalarAlgorithms(void) const;
	virtual const char* getScalarAlgorithmName(int scalarAlgorithmIndex) const;
	virtual Visualization::Abstract::Algorithm* getScalarAlgorithm(int scalarAlgorithmIndex,Visualization::Abstract::VariableManager* variableManager,Cluster::MulticastPipe* pipe) const;
	virtual int getNumVectorAlgorithms(void) const;
	virtual const char* getVectorAlgorithmName(int vectorAlgorithmIndex) const;
	virtual Visualization::Abstract::Algorithm* getVectorAlgorithm(int vectorAlgorithmIndex,Visualization::Abstract::VariableManager* variableManager,Cluster::MulticastPipe* pipe) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_MODULE_IMPLEMENTATION
#include <Wrappers/Module.icpp>
#endif

#endif
