/***********************************************************************
TripleChannelVolumeRenderer - Wrapper class for triple-channel volume
renderers as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDERER_INCLUDED
#define VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDERER_INCLUDED

#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextFieldSlider.h>

#include <Abstract/Element.h>

/* Forward declarations: */
class TripleChannelRaycaster;

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class TripleChannelVolumeRenderer:public Visualization::Abstract::Element
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
	TripleChannelRaycaster* raycaster; // A raycasting volume renderer
	
	/* UI components: */
	GLMotif::ToggleButton* channelEnabledToggles[3]; // Toggle buttons to enable/disable individual channels
	GLMotif::TextFieldSlider* transparencyGammaSliders[3]; // Slider to change current gamma correction factor for each channel
	
	/* Constructors and destructors: */
	public:
	TripleChannelVolumeRenderer(Visualization::Abstract::Algorithm* algorithm,Visualization::Abstract::Parameters* sParameters); // Creates a volume renderer for the given extractor and parameters
	private:
	TripleChannelVolumeRenderer(const TripleChannelVolumeRenderer& source); // Prohibit copy constructor
	TripleChannelVolumeRenderer& operator=(const TripleChannelVolumeRenderer& source); // Prohibit assignment operator
	public:
	virtual ~TripleChannelVolumeRenderer(void);
	
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
	void channelEnabledCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void transparencyGammaCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_TRIPLECHANNELVOLUMERENDERER_IMPLEMENTATION
#include <Wrappers/TripleChannelVolumeRenderer.icpp>
#endif

#endif
