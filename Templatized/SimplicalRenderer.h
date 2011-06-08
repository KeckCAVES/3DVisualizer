/***********************************************************************
SimplicalRenderer - Class to render simplical data sets. Implemented
as a specialization of the generic DataSetRenderer class.
Copyright (c) 2004-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_SIMPLICALRENDERER_INCLUDED
#define VISUALIZATION_SIMPLICALRENDERER_INCLUDED

#include <Templatized/DataSetRenderer.h>
#include <Templatized/Simplical.h>

/* Forward declarations: */
class GLContextData;

namespace Visualization {

namespace Templatized {

template <class ScalarParam,int dimensionParam,class ValueParam>
class DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >
	{
	/* Embedded classes: */
	public:
	typedef Simplical<ScalarParam,dimensionParam,ValueParam> DataSet; // Type of rendered data set
	typedef typename DataSet::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=dimensionParam; // Dimension of data set's domain
	typedef typename DataSet::Point Point; // Type for points in data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in data set's domain
	typedef typename DataSet::Box Box; // Type for axis-aligned boxes in data set's domain
	typedef typename DataSet::CellID CellID; // Type for cell IDs in data set
	typedef typename DataSet::Cell Cell; // Type for cells in data set
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Pointer to the data set to be rendered
	int renderingModeIndex; // Index of currently selected rendering mode
	
	/* Constructors and destructors: */
	public:
	DataSetRenderer(const DataSet* sDataSet); // Creates a renderer for the given data set
	~DataSetRenderer(void);
	
	/* Methods: */
	static int getNumRenderingModes(void); // Returns the number of supported rendering modes
	static const char* getRenderingModeName(int renderingModeIndex); // Returns name of given rendering mode
	int getRenderingMode(void) const // Returns the current rendering mode
		{
		return renderingModeIndex;
		}
	void setRenderingMode(int newRenderingModeIndex); // Sets a new rendering mode
	void glRenderAction(GLContextData& contextData) const; // Renders the data set
	void renderCell(const CellID& cellID,GLContextData& contextData) const; // Highlights the given cell
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SIMPLICALRENDERER_IMPLEMENTATION
#include <Templatized/SimplicalRenderer.icpp>
#endif

#endif
