/***********************************************************************
ExtractorLocator - Class for locators applying visualization algorithms
to data sets.
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

#ifndef EXTRACTORLOCATOR_INCLUDED
#define EXTRACTORLOCATOR_INCLUDED

#include <Abstract/DataSet.h>

#include "BaseLocator.h"
#include "Extractor.h"

/* Forward declarations: */
namespace GLMotif {
class Widget;
class PopupWindow;
class Label;
}

class ExtractorLocator:public BaseLocator,public Extractor
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::DataSet DataSet;
	typedef DataSet::Locator Locator;
	
	/* Elements: */
	private:
	GLMotif::Widget* settingsDialog; // The element extractor's settings dialog
	GLMotif::PopupWindow* busyDialog; // Dialog window to show while a non-incremental extractor is busy
	GLMotif::Label* percentageLabel; // Label to display completion percentage in the busy dialog
	Locator* locator; // A locator for the visualization algorithm
	bool dragging; // Flag if the tool's button is currently pressed
	unsigned int lastSeedRequestID; // ID used to identify the last issued seed request
	
	/* Private methods: */
	GLMotif::PopupWindow* createBusyDialog(const char* algorithmName); // Creates the busy dialog
	void busyFunction(float completionPercentage); // Called during long-running operations
	
	/* Constructors and destructors: */
	public:
	ExtractorLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,Algorithm* sExtractor);
	virtual ~ExtractorLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	
	/* Methods from BaseLocator: */
	virtual void highlightLocator(GLContextData& contextData) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
	
	/* Methods from Extractor: */
	virtual void update(void);
	};

#endif
