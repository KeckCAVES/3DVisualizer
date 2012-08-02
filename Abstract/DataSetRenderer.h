/***********************************************************************
DataSetRenderer - Abstract base class to render the structure of data
sets using OpenGL.
Part of the abstract interface to the templatized visualization
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

#ifndef VISUALIZATION_ABSTRACT_DATASETRENDERER_INCLUDED
#define VISUALIZATION_ABSTRACT_DATASETRENDERER_INCLUDED

#include <Abstract/DataSet.h>

/* Forward declarations: */
class GLRenderState;

namespace Visualization {

namespace Abstract {

class DataSetRenderer
	{
	/* Constructors and destructors: */
	public:
	DataSetRenderer(void) // Default constructor
		{
		}
	protected:
	DataSetRenderer(const DataSetRenderer& source) // Protect copy constructor
		{
		}
	private:
	DataSetRenderer& operator=(const DataSetRenderer& source); // Prohibit assignment operator
	public:
	virtual ~DataSetRenderer(void) // Destructor
		{
		}
	
	/* Methods: */
	virtual DataSetRenderer* clone(void) const =0; // Returns an identical copy of the renderer object
	virtual int getNumRenderingModes(void) const; // Returns the number of rendering modes supported by the renderer
	virtual const char* getRenderingModeName(int renderingModeIndex) const; // Returns the name of a supported rendering mode
	virtual int getRenderingMode(void) const; // Returns the current rendering mode
	virtual void setRenderingMode(int renderingModeIndex); // Sets the given rendering mode for future rendering
	virtual void glRenderAction(GLRenderState& renderState) const; // Renders into the given OpenGL context
	virtual void highlightLocator(const DataSet::Locator* locator,GLRenderState& renderState) const; // Highlights the given data set locator
	};

}

}

#endif
