/***********************************************************************
BaseLocator - Base class for locators in visualizer application.
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

#ifndef BASELOCATOR_INCLUDED
#define BASELOCATOR_INCLUDED

#include <Vrui/LocatorToolAdapter.h>

/* Forward declarations: */
class GLContextData;
class Visualizer;

class BaseLocator:public Vrui::LocatorToolAdapter
	{
	/* Elements: */
	protected:
	Visualizer* application;

	/* Constructors and destructors: */
	public:
	BaseLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication);

	/* New methods: */
	virtual void highlightLocator(GLContextData& contextData) const; // Renders the locator itself
	virtual void glRenderAction(GLContextData& contextData) const; // Renders opaque elements and other objects controlled by the locator
	virtual void glRenderActionTransparent(GLContextData& contextData) const; // Renders transparent elements and other objects controlled by the locator
	};

#endif
