/***********************************************************************
CuttingPlaneLocator - Class for locators rendering cutting planes.
Copyright (c) 2006-2010 Oliver Kreylos

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

#ifndef CUTTINGPLANELOCATOR_INCLUDED
#define CUTTINGPLANELOCATOR_INCLUDED

#include "BaseLocator.h"

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class CuttingPlane;

class CuttingPlaneLocator:public BaseLocator
	{
	/* Elements: */
	private:
	CuttingPlane* cuttingPlane; // Pointer to the cutting plane allocated for this tool
	
	/* Constructors and destructors: */
	public:
	CuttingPlaneLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg =0);
	virtual ~CuttingPlaneLocator(void);
	
	/* Methods from Vrui::LocatorToolAdapter: */
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
	virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
	virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
	};

#endif
