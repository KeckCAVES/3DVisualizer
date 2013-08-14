/***********************************************************************
ScalarEvaluationLocator - Class for locators evaluating scalar
properties of data sets.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef SCALAREVALUATIONLOCATOR_INCLUDED
#define SCALAREVALUATIONLOCATOR_INCLUDED

#include <Abstract/ScalarExtractor.h>

#include "EvaluationLocator.h"

/* Forward declarations: */
namespace Misc {
class CallbackData;
class ConfigurationFileSection;
}
namespace GLMotif {
class TextField;
}

class ScalarEvaluationLocator:public EvaluationLocator
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::ScalarExtractor ScalarExtractor;
	typedef ScalarExtractor::Scalar Scalar;
	
	/* Elements: */
	const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value
	GLMotif::TextField* value; // The value text field
	bool valueValid; // Flag if the evaluation value is valid
	Scalar currentValue; // The current evaluation value

	/* Constructors and destructors: */
	public:
	ScalarEvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg =0);
	virtual ~ScalarEvaluationLocator(void);

	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	
	/* New methods: */
	void insertControlPointCallback(Misc::CallbackData* cbData);
	};

#endif
