/***********************************************************************
CartesianRenderer - Class to render cartesian data sets. Implemented
as a specialization of the generic DataSetRenderer class.
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

#define VISUALIZATION_TEMPLATIZED_CARTESIANRENDERER_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <GL/GLGeometryWrappers.h>

#include <Templatized/CartesianRenderer.h>

namespace Visualization {

namespace Templatized {

namespace CartesianRendererImplementation {

/***********************************************************************
Internal helper class to render cartesian grids of different dimensions:
***********************************************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,2,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef Cartesian<ScalarParam,2,ValueParam> DataSet;
	typedef typename DataSet::Point Point;
	typedef typename DataSet::Size Size;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Cell Cell;
	
	/* Methods: */
	inline static void renderGridOutline(const Size& cellSize,const Index& numCells)
		{
		/* Calculate total grid size: */
		Size gridSize;
		for(int i=0;i<2;++i)
			gridSize[i]=cellSize[i]*ScalarParam(numCells[i]);
		
		glBegin(GL_LINE_LOOP);
		Point p=Point::origin;
		glVertex(p);
		p[0]+=gridSize[0];
		glVertex(p);
		p[1]+=gridSize[1];
		glVertex(p);
		p[0]-=gridSize[0];
		glVertex(p);
		glEnd();
		}
	inline static void renderGridFaces(const Size& cellSize,const Index& numCells)
		{
		/* Calculate total grid size: */
		Size gridSize;
		for(int i=0;i<2;++i)
			gridSize[i]=cellSize[i]*ScalarParam(numCells[i]);
		
		glBegin(GL_LINE_LOOP);
		Point p=Point::origin;
		glVertex(p);
		p[0]+=gridSize[0];
		glVertex(p);
		p[1]+=gridSize[1];
		glVertex(p);
		p[0]-=gridSize[0];
		glVertex(p);
		glEnd();
		}
	inline static void renderGridCells(const Size& cellSize,const Index& numCells)
		{
		glBegin(GL_LINES);
		
		/* Render grid lines along x direction: */
		ScalarParam x0=ScalarParam(0);
		ScalarParam x1=ScalarParam(numCells[0])*cellSize[0];
		for(int y=0;y<=numCells[1];++y)
			{
			ScalarParam y0=ScalarParam(y)*cellSize[1];
			glVertex(x0,y0);
			glVertex(x1,y0);
			}
		
		/* Render grid lines along y direction: */
		ScalarParam y0=ScalarParam(0);
		ScalarParam y1=ScalarParam(numCells[1])*cellSize[1];
		for(int x=0;x<=numCells[0];++x)
			{
			ScalarParam x0=ScalarParam(x)*cellSize[0];
			glVertex(x0,y0);
			glVertex(x0,y1);
			}
		
		glEnd();
		}
	inline static void highlightCell(const Cell& cell)
		{
		glBegin(GL_LINE_LOOP);
		glVertex(cell.getVertexPos(0));
		glVertex(cell.getVertexPos(1));
		glVertex(cell.getVertexPos(3));
		glVertex(cell.getVertexPos(2));
		glEnd();
		}
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,3,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef Cartesian<ScalarParam,3,ValueParam> DataSet;
	typedef typename DataSet::Point Point;
	typedef typename DataSet::Size Size;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Cell Cell;
	
	/* Methods: */
	inline static void renderGridOutline(const Size& cellSize,const Index& numCells)
		{
		/* Calculate total grid size: */
		Size gridSize;
		for(int i=0;i<3;++i)
			gridSize[i]=cellSize[i]*ScalarParam(numCells[i]);
		
		glBegin(GL_LINE_STRIP);
		Point p=Point::origin;
		glVertex(p);
		p[0]+=gridSize[0];
		glVertex(p);
		p[1]+=gridSize[1];
		glVertex(p);
		p[0]-=gridSize[0];
		glVertex(p);
		p[1]-=gridSize[1];
		glVertex(p);
		p[2]+=gridSize[2];
		glVertex(p);
		p[0]+=gridSize[0];
		glVertex(p);
		p[1]+=gridSize[1];
		glVertex(p);
		p[0]-=gridSize[0];
		glVertex(p);
		p[1]-=gridSize[1];
		glVertex(p);
		glEnd();
		glBegin(GL_LINES);
		p[0]+=gridSize[0];
		glVertex(p);
		p[2]-=gridSize[2];
		glVertex(p);
		p[1]+=gridSize[1];
		glVertex(p);
		p[2]+=gridSize[2];
		glVertex(p);
		p[0]-=gridSize[0];
		glVertex(p);
		p[2]-=gridSize[2];
		glVertex(p);
		glEnd();
		}
	inline static void renderGridFaces(const Size& cellSize,const Index& numCells)
		{
		/* Calculate total grid size: */
		Size gridSize;
		for(int i=0;i<3;++i)
			gridSize[i]=cellSize[i]*ScalarParam(numCells[i]);
		
		/* Render grid outline first: */
		renderGridOutline(cellSize,numCells);
		
		/* Render grid lines in (x,y)-plane: */
		for(int z=1;z<numCells[2];++z)
			{
			glBegin(GL_LINE_LOOP);
			Point p(ScalarParam(0),ScalarParam(0),cellSize[2]*ScalarParam(z));
			glVertex(p);
			p[0]+=gridSize[0];
			glVertex(p);
			p[1]+=gridSize[1];
			glVertex(p);
			p[0]-=gridSize[0];
			glVertex(p);
			glEnd();
			}
		
		/* Render grid lines in (x,z)-plane: */
		for(int y=1;y<numCells[1];++y)
			{
			glBegin(GL_LINE_LOOP);
			Point p(ScalarParam(0),cellSize[1]*ScalarParam(y),ScalarParam(0));
			glVertex(p);
			p[0]+=gridSize[0];
			glVertex(p);
			p[2]+=gridSize[2];
			glVertex(p);
			p[0]-=gridSize[0];
			glVertex(p);
			glEnd();
			}
		
		/* Render grid lines in (y,z)-plane: */
		for(int x=1;x<numCells[0];++x)
			{
			glBegin(GL_LINE_LOOP);
			Point p(cellSize[0]*ScalarParam(x),ScalarParam(0),ScalarParam(0));
			glVertex(p);
			p[1]+=gridSize[1];
			glVertex(p);
			p[2]+=gridSize[2];
			glVertex(p);
			p[1]-=gridSize[1];
			glVertex(p);
			glEnd();
			}
		}
	inline static void renderGridCells(const Size& cellSize,const Index& numCells)
		{
		/* Calculate total grid size: */
		Size gridSize;
		for(int i=0;i<3;++i)
			gridSize[i]=cellSize[i]*ScalarParam(numCells[i]);
		
		glBegin(GL_LINES);
		
		/* Render grid lines in x-direction: */
		for(int y=0;y<=numCells[1];++y)
			for(int z=0;z<=numCells[2];++z)
				{
				Point p(ScalarParam(0),cellSize[1]*ScalarParam(y),cellSize[2]*ScalarParam(z));
				glVertex(p);
				p[0]+=gridSize[0];
				glVertex(p);
				}
		
		/* Render grid lines in y-direction: */
		for(int x=0;x<=numCells[0];++x)
			for(int z=0;z<=numCells[2];++z)
				{
				Point p(cellSize[0]*ScalarParam(x),ScalarParam(0),cellSize[2]*ScalarParam(z));
				glVertex(p);
				p[1]+=gridSize[1];
				glVertex(p);
				}
		
		/* Render grid lines in z-direction: */
		for(int x=0;x<=numCells[0];++x)
			for(int y=0;y<=numCells[1];++y)
				{
				Point p(cellSize[0]*ScalarParam(x),cellSize[1]*ScalarParam(y),ScalarParam(0));
				glVertex(p);
				p[2]+=gridSize[2];
				glVertex(p);
				}
		
		glEnd();
		}
	inline static void highlightCell(const Cell& cell)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(2));
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(4));
		glVertex(cell.getVertexPosition(5));
		glVertex(cell.getVertexPosition(7));
		glVertex(cell.getVertexPosition(6));
		glVertex(cell.getVertexPosition(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(5));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(7));
		glVertex(cell.getVertexPosition(2));
		glVertex(cell.getVertexPosition(6));
		glEnd();
		}
	};

}

/*******************************************
Methods of class DataSetRenderer<Cartesian>:
*******************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::DataSetRenderer(
	const typename DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::~DataSetRenderer(
	void)
	{
	/* Nothing to do yet... */
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
int
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::getNumRenderingModes(
	void)
	{
	return 3;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
const char*
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=3)
		Misc::throwStdErr("DataSetRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[3]=
		{
		"Grid Outline","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=3)
		Misc::throwStdErr("DataSetRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the grid's outline: */
			CartesianRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridOutline(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		
		case 1:
			/* Render the grid's faces: */
			CartesianRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridFaces(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		
		case 2:
			/* Render the grid's cells: */
			CartesianRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridCells(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::renderCell(
	const typename DataSetRenderer<Cartesian<ScalarParam,dimensionParam,ValueParam> >::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	CartesianRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
