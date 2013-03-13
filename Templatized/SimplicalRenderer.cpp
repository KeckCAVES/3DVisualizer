/***********************************************************************
SimplicalRenderer - Class to render simplical data sets. Implemented
as a specialization of the generic DataSetRenderer class.
Copyright (c) 2004-2007 Oliver Kreylos

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

#define VISUALIZATION_SIMPLICALRENDERER_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <GL/GLGeometryWrappers.h>

#include <Templatized/SimplicalRenderer.h>

namespace Visualization {

namespace Templatized {

namespace SimplicalRendererImplementation {

/***********************************************************************
Internal helper class to render simplical grids of different dimensions:
***********************************************************************/

template <class ScalarParam,int dimensionParam,class GridCellParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,2,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef Simplical<ScalarParam,2,ValueParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Cell Cell;
	typedef typename DataSet::CellIterator CellIterator;
	
	/* Methods: */
	inline static void renderBoundingBox(const Box& box)
		{
		glBegin(GL_LINE_LOOP);
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(2));
		glEnd();
		}
	inline static void renderGridOutline(const DataSet& dataSet)
		{
		/* Render all grid cell faces that do not have neighbours: */
		glBegin(GL_LINES);
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			for(int i=0;i<3;++i)
				if(!cIt->getNeighbourID(i).isValid())
					{
					glVertex(cIt->getVertexPosition((i+1)%3));
					glVertex(cIt->getVertexPosition((i+2)%3));
					}
		glEnd();
		}
	inline static void renderGridFaces(const DataSet& dataSet)
		{
		/* Render all grid cell faces that do not have neighbours: */
		glBegin(GL_LINES);
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			for(int i=0;i<3;++i)
				if(!cIt->getNeighbourID(i).isValid())
					{
					glVertex(cIt->getVertexPosition((i+1)%3));
					glVertex(cIt->getVertexPosition((i+2)%3));
					}
		glEnd();
		}
	inline static void renderGridCells(const DataSet& dataSet)
		{
		/* Render all grid cell faces: */
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			{
			glBegin(GL_LINE_LOOP);
			for(int i=0;i<3;++i)
				glVertex(cIt->getVertexPosition(i));
			glEnd();
			}
		}
	inline static void highlightCell(const Cell& cell)
		{
		/* Render all grid cell faces: */
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<3;++i)
			glVertex(cell.getVertexPosition(i));
		glEnd();
		}
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,3,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef Simplical<ScalarParam,3,ValueParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Cell Cell;
	typedef typename DataSet::CellIterator CellIterator;
	
	/* Methods: */
	inline static void renderBoundingBox(const Box& box)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(2));
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(4));
		glVertex(box.getVertex(5));
		glVertex(box.getVertex(7));
		glVertex(box.getVertex(6));
		glVertex(box.getVertex(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(5));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(7));
		glVertex(box.getVertex(2));
		glVertex(box.getVertex(6));
		glEnd();
		}
	inline static void renderGridOutline(const DataSet& dataSet)
		{
		/* Render all grid cell faces that do not have neighbours: */
		glBegin(GL_LINES);
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			for(int i=0;i<4;++i)
				if(!cIt->getNeighbourID(i).isValid())
					{
					for(int j=0;j<3;++j)
						if(j!=i)
							for(int k=j+1;k<4;++k)
								if(k!=i)
									{
									glVertex(cIt->getVertexPosition(j));
									glVertex(cIt->getVertexPosition(k));
									}
					}
		glEnd();
		}
	inline static void renderGridFaces(const DataSet& dataSet)
		{
		/* Render all grid cell faces that do not have neighbours: */
		glBegin(GL_LINES);
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			for(int i=0;i<4;++i)
				if(!cIt->getNeighbourID(i).isValid())
					{
					for(int j=0;j<3;++j)
						if(j!=i)
							for(int k=j+1;k<4;++k)
								if(k!=i)
									{
									glVertex(cIt->getVertexPosition(j));
									glVertex(cIt->getVertexPosition(k));
									}
					}
		glEnd();
		}
	inline static void renderGridCells(const DataSet& dataSet)
		{
		/* Render all grid cell faces: */
		glBegin(GL_LINES);
		for(CellIterator cIt=dataSet.beginCells();cIt!=dataSet.endCells();++cIt)
			for(int i=0;i<3;++i)
				for(int j=i+1;j<4;++j)
					{
					glVertex(cIt->getVertexPosition(i));
					glVertex(cIt->getVertexPosition(j));
					}
		glEnd();
		}
	inline static void highlightCell(const Cell& cell)
		{
		/* Render all grid cell faces: */
		glBegin(GL_LINES);
		for(int i=0;i<3;++i)
			for(int j=i+1;j<4;++j)
				{
				glVertex(cell.getVertexPosition(i));
				glVertex(cell.getVertexPosition(j));
				}
		glEnd();
		}
	};

}

/*******************************************
Methods of class DataSetRenderer<Simplical>:
*******************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::DataSetRenderer(
	const typename DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::~DataSetRenderer(
	void)
	{
	/* Nothing to do yet... */
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
int
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::getNumRenderingModes(
	void)
	{
	return 4;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
const char*
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=4)
		Misc::throwStdErr("DataSetRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[4]=
		{
		"Bounding Box","Grid Outline","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=4)
		Misc::throwStdErr("DataSetRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the grid's bounding box: */
			SimplicalRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderBoundingBox(dataSet->getDomainBox());
			break;
		
		case 1:
			/* Render the grid's outline: */
			SimplicalRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridOutline(*dataSet);
			break;
		
		case 2:
			/* Render the grid's faces: */
			SimplicalRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridFaces(*dataSet);
			break;
		
		case 3:
			/* Render the grid's cells: */
			SimplicalRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridCells(*dataSet);
			break;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::renderCell(
	const typename DataSetRenderer<Simplical<ScalarParam,dimensionParam,ValueParam> >::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	SimplicalRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
