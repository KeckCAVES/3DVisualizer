/***********************************************************************
VolumeRenderer - Wrapper class for volume renderers as visualization
elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2005-2012 Oliver Kreylos

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

#include <GLMotif/TextFieldSlider.h>

#include <Abstract/Element.h>

/* Forward declarations: */
class GLColorMap;
#ifdef VISUALIZATION_USE_SHADERS
class SingleChannelRaycaster;
#else
class PaletteRenderer;
#endif

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
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	
	/* Elements: */
	private:
	int scalarVariableIndex; // Index of the scalar variable visualized by the volume renderer
	#ifdef VISUALIZATION_USE_SHADERS
	SingleChannelRaycaster* renderer; // A raycasting volume renderer
	#else
	const GLColorMap* colorMap; // A transfer function to map scalar values to colors and opacities
	PaletteRenderer* renderer; // A texture-based volume renderer
	float transparencyGamma; // A gamma correction factor to apply to color map opacities
	#endif
	
	/* Constructors and destructors: */
	public:
	VolumeRenderer(Visualization::Abstract::Algorithm* algorithm,Visualization::Abstract::Parameters* sParameters); // Creates a volume renderer for the given algorithm and parameters
	private:
	VolumeRenderer(const VolumeRenderer& source); // Prohibit copy constructor
	VolumeRenderer& operator=(const VolumeRenderer& source); // Prohibit assignment operator
	public:
	virtual ~VolumeRenderer(void);
	
	/* Methods from Visualization::Abstract::Element: */
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual bool usesTransparency(void) const
		{
		return true;
		}
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager); // Returns a new UI widget to change internal settings of the element
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* New methods: */
	void sliceFactorCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void transparencyGammaCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_VOLUMERENDERER_IMPLEMENTATION
#include <Wrappers/VolumeRenderer.icpp>
#endif

#endif
