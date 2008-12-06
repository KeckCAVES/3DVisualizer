/***********************************************************************
VolumeRenderer - Wrapper class for volume renderers as visualization
elements.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDERER_INCLUDED
#define VISUALIZATION_WRAPPERS_VOLUMERENDERER_INCLUDED

#include <GLMotif/Slider.h>

#include <Abstract/Element.h>
#if 1
#include <Templatized/SliceVolumeRendererSampling.h>
#else
#include <Templatized/SliceVolumeRenderer.h>
#endif

/* Forward declarations: */
class GLColorMap;
namespace GLMotif {
class TextField;
}
namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class VolumeRenderer:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef Visualization::Templatized::SliceVolumeRenderer<DS,SE> SVR; // Type of templatized volume renderer
	
	struct Parameters // Extraction parameters defining a volume renderer
		{
		/* Elements: */
		public:
		int scalarVariableIndex; // Index of the scalar variable defining the isosurface
		Scalar sliceFactor; // Average number of slices to generate per grid cell
		float transparencyGamma; // Overall gamma value to apply to palette transparencies
		
		/* Constructors and destructors: */
		Parameters(int sScalarVariableIndex,Scalar sSliceFactor,float sTransparencyGamma) // Initializes permanent parameters
			:scalarVariableIndex(sScalarVariableIndex),
			 sliceFactor(sSliceFactor),
			 transparencyGamma(sTransparencyGamma)
			{
			}
		
		/* Methods: */
		template <class DataSinkParam>
		void write(DataSinkParam& dataSink) const // Writes the parameters to a data sink (such as a file or a pipe)
			{
			dataSink.write<int>(scalarVariableIndex);
			dataSink.write<Scalar>(sliceFactor);
			dataSink.write<float>(transparencyGamma);
			}
		template <class DataSourceParam>
		Parameters& read(DataSourceParam& dataSource) // Reads the parameters from a data sink (such as a file or a pipe)
			{
			scalarVariableIndex=dataSource.read<int>();
			sliceFactor=dataSource.read<Scalar>();
			transparencyGamma=dataSource.read<float>();
			return *this;
			}
		};
	
	/* Elements: */
	private:
	Parameters parameters; // The volume renderer's extraction parameters
	SVR svr; // The slice volume renderer
	
	/* UI components: */
	GLMotif::TextField* sliceFactorValue; // Text field to display current slice factor
	GLMotif::Slider* sliceFactorSlider; // Slider to change current slice factor value
	GLMotif::TextField* transparencyGammaValue; // Text field to display current gamma correction factor
	GLMotif::Slider* transparencyGammaSlider; // Slider to change current gamma correction factor value
	
	/* Constructors and destructors: */
	public:
	VolumeRenderer(const Parameters& sParameters,const DS* sDs,const SE& sSe,const GLColorMap* sColorMap,Comm::MulticastPipe* pipe); // Creates a volume renderer for the given data set and parameters
	private:
	VolumeRenderer(const VolumeRenderer& source); // Prohibit copy constructor
	VolumeRenderer& operator=(const VolumeRenderer& source); // Prohibit assignment operator
	public:
	virtual ~VolumeRenderer(void);
	
	/* Methods: */
	const Parameters& getParameters(void) const // Returns the volume renderers's extraction parameters
		{
		return parameters;
		}
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual bool usesTransparency(void) const;
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager);
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void saveParameters(Misc::File& parameterFile) const;
	void sliderValueChangedCallback(GLMotif::Slider::ValueChangedCallbackData* cbData); // Callback when the sliders in the settings dialog change value
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDERER_IMPLEMENTATION
#include <Wrappers/VolumeRenderer.cpp>
#endif

#endif
