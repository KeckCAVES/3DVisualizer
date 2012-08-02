/***********************************************************************
EvaluationLocator - Base class for locators evaluating properties of
data sets.
Copyright (c) 2006-2012 Oliver Kreylos

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

#ifndef EVALUATIONLOCATOR_INCLUDED
#define EVALUATIONLOCATOR_INCLUDED

#include <Geometry/Point.h>
#include <Vrui/Geometry.h>

#include <Abstract/DataSet.h>

#include "BaseLocator.h"

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class TextField;
}
namespace Visualization {
namespace Abstract {
class CoordinateTransformer;
}
}

class EvaluationLocator:public BaseLocator
	{
	/* Embedded classes: */
	protected:
	typedef Visualization::Abstract::DataSet::Locator Locator;
	typedef Visualization::Abstract::CoordinateTransformer CoordinateTransformer;
	
	/* Elements: */
	GLMotif::PopupWindow* evaluationDialogPopup; // Pointer to the evaluation dialog window
	GLMotif::RowColumn* evaluationDialog; // Pointer to the evaluation dialog
	GLMotif::TextField* pos[3]; // The coordinate labels for the evaluation position
	Locator* locator; // A locator for evaluation
	Vrui::Point point; // The evaluation point
	bool dragging; // Flag if the locator is currently dragging the evaluation point
	bool hasPoint; // Flag whether the locator has a position

	/* Constructors and destructors: */
	public:
	EvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const char* dialogWindowTitle);
	virtual ~EvaluationLocator(void);

	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	
	/* Methods from class BaseLocator: */
	virtual void highlightLocator(GLRenderState& renderState) const;
	};

#endif
