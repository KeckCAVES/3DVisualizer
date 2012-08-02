/***********************************************************************
TripleChannelVolumeRendererExtractor - Wrapper class to map from the
abstract visualization algorithm interface to a templatized volume
renderer implementation.
Copyright (c) 2009-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDEREREXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDEREREXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/TextFieldSlider.h>

#include <Abstract/DataSet.h>
#include <Abstract/Parameters.h>
#include <Abstract/Algorithm.h>

#include <Wrappers/TripleChannelVolumeRenderer.h>

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
class TripleChannelVolumeRendererExtractor:public Visualization::Abstract::Algorithm
	{
	friend class Visualization::Wrappers::TripleChannelVolumeRenderer<DataSetWrapperParam>;
	
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar value type
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Wrappers::TripleChannelVolumeRenderer<DataSetWrapper> TripleChannelVolumeRenderer; // Type of created visualization elements
	typedef Misc::Autopointer<TripleChannelVolumeRenderer> TripleChannelVolumeRendererPointer; // Type for pointers to created visualization elements
	
	private:
	class Parameters:public Visualization::Abstract::Parameters // Class to store extraction parameters for global isosurfaces
		{
		friend class Visualization::Wrappers::TripleChannelVolumeRenderer<DataSetWrapperParam>;
		friend class TripleChannelVolumeRendererExtractor;
		
		/* Elements: */
		private:
		int scalarVariableIndices[3]; // Indices of the scalar variables for direct volume rendering
		VScalar outOfDomainValues[3]; // Values to assign to volume renderer voxels that are outside the data set's domain
		Scalar sliceFactor; // Slice distance for texture- or raycasting-based volume rendering
		bool channelEnableds[3]; // Enable flags for each channel
		float transparencyGammas[3]; // Overall transparency adjustment factors for each channel
		
		/* Methods from Abstract::Parameters: */
		virtual bool isValid(void) const
			{
			return true;
			}
		virtual Visualization::Abstract::Parameters* clone(void) const
			{
			return new Parameters(*this);
			}
		virtual void write(Visualization::Abstract::ParametersSink& sink) const;
		virtual void read(Visualization::Abstract::ParametersSource& source);
		};
	
	/* Elements: */
	private:
	static const char* name; // Identifying name of this algorithm
	Parameters parameters; // The volume renderer extraction parameters used by this extractor
	
	/* UI components: */
	GLMotif::DropdownBox* scalarVariableBoxes[3]; // Dropdown boxes to select the scalar variables for each channel
	GLMotif::TextFieldSlider* outOfDomainValueSliders[3]; // Sliders to select out-of-domain values for each channel
	
	/* Constructors and destructors: */
	public:
	TripleChannelVolumeRendererExtractor(Visualization::Abstract::VariableManager* sVariableManager,Cluster::MulticastPipe* sPipe); // Creates a volume renderer extractor
	virtual ~TripleChannelVolumeRendererExtractor(void);
	
	/* Methods from Visualization::Abstract::Algorithm: */
	virtual const char* getName(void) const
		{
		return name;
		}
	virtual bool hasGlobalCreator(void) const
		{
		return true;
		}
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual void readParameters(Visualization::Abstract::ParametersSource& source);
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
	void scalarVariableBoxCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void outOfDomainValueCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDEREREXTRACTOR_IMPLEMENTATION
#include <Wrappers/TripleChannelVolumeRendererExtractor.icpp>
#endif

#endif
