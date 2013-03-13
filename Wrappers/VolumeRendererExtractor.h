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

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>

#include <Abstract/DataSet.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/VolumeRenderer.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class Element;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class VolumeRendererExtractor:public Visualization::Abstract::Algorithm
	{
	friend class Visualization::Wrappers::VolumeRenderer<DataSetWrapperParam>;
	
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::VolumeRenderer<DataSetWrapper> VolumeRenderer; // Type of created visualization elements
	typedef Misc::Autopointer<VolumeRenderer> VolumeRendererPointer; // Type for pointers to created visualization elements
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for global isosurfaces
		{
		friend class Visualization::Wrappers::VolumeRenderer<DataSetWrapperParam>;
		friend class VolumeRendererExtractor;
		
		/* Elements: */
		private:
		int scalarVariableIndex; // Index of the scalar variable for direct volume rendering
		Scalar sliceFactor; // Slice distance for texture- or raycasting-based volume rendering
		float transparencyGamma; // Overall transparency adjustment factor
		
		/* Private methods: */
		template <class DataSourceParam>
		void readBinary(DataSourceParam& dataSource,bool raw,const Visualization::Abstract::VariableManager* variableManager); // Reads parameters from a binary data source
		template <class DataSourceParam>
		void writeBinary(DataSourceParam& dataSink,bool raw,const Visualization::Abstract::VariableManager* variableManager) const; // Writes parameters to a binary data source
		
		/* Constructors and destructors: */
		public:
		Parameters(int sScalarVariableIndex)
			:scalarVariableIndex(sScalarVariableIndex)
			{
			}
		
		/* Methods from Abstract::Parameters: */
		virtual bool isValid(void) const
			{
			return true;
			}
		virtual void read(Misc::File& file,bool ascii,Visualization::Abstract::VariableManager* variableManager);
		virtual void read(Comm::MulticastPipe& pipe,Visualization::Abstract::VariableManager* variableManager);
		virtual void read(Comm::ClusterPipe& pipe,Visualization::Abstract::VariableManager* variableManager);
		virtual void write(Misc::File& file,bool ascii,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual void write(Comm::MulticastPipe& pipe,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual void write(Comm::ClusterPipe& pipe,const Visualization::Abstract::VariableManager* variableManager) const;
		virtual Visualization::Abstract::Parameters* clone(void) const
			{
			return new Parameters(*this);
			}
		};
	
	/* Elements: */
	private:
	static const char* name; // Identifying name of this algorithm
	Parameters parameters; // The volume renderer extraction parameters used by this extractor
	
	/* Constructors and destructors: */
	public:
	VolumeRendererExtractor(Visualization::Abstract::VariableManager* sVariableManager,Comm::MulticastPipe* sPipe); // Creates a volume renderer extractor
	virtual ~VolumeRendererExtractor(void);
	
	/* Methods from Visualization::Abstract::Algorithm: */
	virtual const char* getName(void) const
		{
		return name;
		}
	virtual bool hasGlobalCreator(void) const
		{
		return true;
		}
	virtual Visualization::Abstract::Parameters* cloneParameters(void) const
		{
		return new Parameters(parameters);
		}
	virtual Visualization::Abstract::Element* createElement(Visualization::Abstract::Parameters* extractParameters);
	virtual Visualization::Abstract::Element* startSlaveElement(Visualization::Abstract::Parameters* extractParameters);
	
	/* New methods: */
	static const char* getClassName(void) // Returns the algorithm class name
		{
		return name;
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_IMPLEMENTATION
#include <Wrappers/VolumeRendererExtractor.cpp>
#endif

#endif
