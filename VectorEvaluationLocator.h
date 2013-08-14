/***********************************************************************
VectorEvaluationLocator - Class for locators evaluating vector
properties of data sets.
Copyright (c) 2008-2012 Oliver Kreylos

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

#ifndef VECTOREVALUATIONLOCATOR_INCLUDED
#define VECTOREVALUATIONLOCATOR_INCLUDED

#include <GLMotif/TextFieldSlider.h>

#include <Abstract/ScalarExtractor.h>
#include <Abstract/VectorExtractor.h>

#include "EvaluationLocator.h"

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLColorMap;

class VectorEvaluationLocator:public EvaluationLocator
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::ScalarExtractor ScalarExtractor;
	typedef ScalarExtractor::Scalar Scalar;
	typedef Visualization::Abstract::VectorExtractor VectorExtractor;
	typedef VectorExtractor::Vector Vector;
	
	/* Elements: */
	const VectorExtractor* vectorExtractor; // Extractor for the evaluated vector value
	const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value (to color arrow rendering)
	const GLColorMap* colorMap; // Color map for the evaluated scalar value
	GLMotif::TextField* values[3]; // The vector component value text field
	bool valueValid; // Flag if the evaluation value is valid
	Vector currentValue; // The current evaluation value
	Scalar currentScalarValue; // The current scalar value
	Scalar arrowLengthScale; // Scaling factor for arrow rendering
	
	/* Constructors and destructors: */
	public:
	VectorEvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg =0);
	virtual ~VectorEvaluationLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	
	/* Methods from class BaseLocator: */
	virtual void highlightLocator(GLRenderState& renderState) const;
	
	/* New methods: */
	void arrowScaleCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	};

#endif
