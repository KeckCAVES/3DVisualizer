/***********************************************************************
DataSetRenderer - Abstract base class to render the structure of data
sets using OpenGL.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2007 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>

#include <Abstract/DataSetRenderer.h>

namespace Visualization {

namespace Abstract {

/********************************
Methods of class DataSetRenderer:
********************************/

int DataSetRenderer::getNumRenderingModes(void) const
	{
	return 0;
	}

const char* DataSetRenderer::getRenderingModeName(int renderingModeIndex) const
	{
	Misc::throwStdErr("DataSetRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	return 0;
	}

int DataSetRenderer::getRenderingMode(void) const
	{
	return 0;
	}

void DataSetRenderer::setRenderingMode(int renderingModeIndex)
	{
	Misc::throwStdErr("DataSetRenderer::setRenderingMode: invalid rendering mode index %d",renderingModeIndex);
	}

void DataSetRenderer::glRenderAction(GLRenderState& renderState) const
	{
	}

void DataSetRenderer::highlightLocator(const DataSet::Locator* locator,GLRenderState& renderState) const
	{
	}

}

}
