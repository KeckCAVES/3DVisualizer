/***********************************************************************
DataSetRenderer - Wrapper class to map from the abstract data set
renderer interface to its templatized data set renderer implementation.
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

#ifndef VISUALIZATION_WRAPPERS_DATASETRENDERER_INCLUDED
#define VISUALIZATION_WRAPPERS_DATASETRENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLObject.h>

#include <Abstract/DataSet.h>
#include <Abstract/DataSetRenderer.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class DSParam>
class DataSetRenderer;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class DataSetRenderer:public Visualization::Abstract::DataSetRenderer,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::DataSetRenderer Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DataSetWrapper::DSL DSL; // Type of templatized locator
	typedef Visualization::Templatized::DataSetRenderer<DS> DSR; // Type of templatized data set renderer
	typedef GLColor<GLfloat,4> Color; // Type for colors
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId; // ID of display list to render the data set
		unsigned int displayVersion; // Version of the data set rendering currently in the display list
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	DSR dsr; // The templatized data set renderer
	GLfloat gridLineWidth; // Width of rendered grid lines in pixel
	Color gridLineColor; // Color of rendered grid lines
	unsigned int displayVersion; // Version of the data set rendering
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	
	/* Constructors and destructors: */
	public:
	DataSetRenderer(const Visualization::Abstract::DataSet* sDataSet); // Creates a data set renderer for the given data set
	protected:
	DataSetRenderer(const DataSetRenderer& source); // Protect copy constructor
	private:
	DataSetRenderer& operator=(const DataSetRenderer& source); // Prohibit assignment operator
	public:
	virtual ~DataSetRenderer(void); // Destroys the data set renderer
	
	/* Methods from Abstract::DataSetRenderer: */
	virtual Base* clone(void) const;
	virtual int getNumRenderingModes(void) const;
	virtual const char* getRenderingModeName(int renderingModeIndex) const;
	virtual int getRenderingMode(void) const;
	virtual void setRenderingMode(int renderingModeIndex);
	virtual void glRenderAction(GLRenderState& renderState) const;
	virtual void highlightLocator(const Visualization::Abstract::DataSet::Locator* locator,GLRenderState& renderState) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	const DSR& getDsr(void) const // Returns templatized data set renderer
		{
		return dsr;
		}
	DSR& getDsr(void) // Ditto
		{
		return dsr;
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_DATASETRENDERER_IMPLEMENTATION
#include <Wrappers/DataSetRenderer.icpp>
#endif

#endif
