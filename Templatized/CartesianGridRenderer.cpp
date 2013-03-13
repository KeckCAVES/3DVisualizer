/***********************************************************************
CartesianGridRenderer - Helper class to render Cartesian grids.
Copyright (c) 2009 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_CARTESIANGRIDRENDERER_IMPLEMENTATION

#include <Templatized/CartesianGridRenderer.h>

#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

namespace Visualization {

namespace Templatized {

namespace CartesianGridRendererImplementation {

/***********************************************************************
Internal helper class to render Cartesian grids of different dimensions:
***********************************************************************/

template <int dimensionParam,class DataSetParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class DataSetParam>
class GridRenderer<2,DataSetParam>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef typename DataSet::Scalar Scalar;
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
			gridSize[i]=cellSize[i]*Scalar(numCells[i]);
		
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
		Scalar x0=Scalar(0);
		Scalar x1=Scalar(numCells[0])*cellSize[0];
		for(int y=0;y<=numCells[1];++y)
			{
			Scalar y0=Scalar(y)*cellSize[1];
			glVertex(x0,y0);
			glVertex(x1,y0);
			}
		
		/* Render grid lines along y direction: */
		Scalar y0=Scalar(0);
		Scalar y1=Scalar(numCells[1])*cellSize[1];
		for(int x=0;x<=numCells[0];++x)
			{
			Scalar x0=Scalar(x)*cellSize[0];
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

template <class DataSetParam>
class GridRenderer<3,DataSetParam>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef typename DataSet::Scalar Scalar;
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
			gridSize[i]=cellSize[i]*Scalar(numCells[i]);
		
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
			gridSize[i]=cellSize[i]*Scalar(numCells[i]);
		
		/* Render grid outline first: */
		renderGridOutline(cellSize,numCells);
		
		/* Render grid lines in (x,y)-plane: */
		for(int z=1;z<numCells[2];++z)
			{
			glBegin(GL_LINE_LOOP);
			Point p(Scalar(0),Scalar(0),cellSize[2]*Scalar(z));
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
			Point p(Scalar(0),cellSize[1]*Scalar(y),Scalar(0));
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
			Point p(cellSize[0]*Scalar(x),Scalar(0),Scalar(0));
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
			gridSize[i]=cellSize[i]*Scalar(numCells[i]);
		
		glBegin(GL_LINES);
		
		/* Render grid lines in x-direction: */
		for(int y=0;y<=numCells[1];++y)
			for(int z=0;z<=numCells[2];++z)
				{
				Point p(Scalar(0),cellSize[1]*Scalar(y),cellSize[2]*Scalar(z));
				glVertex(p);
				p[0]+=gridSize[0];
				glVertex(p);
				}
		
		/* Render grid lines in y-direction: */
		for(int x=0;x<=numCells[0];++x)
			for(int z=0;z<=numCells[2];++z)
				{
				Point p(cellSize[0]*Scalar(x),Scalar(0),cellSize[2]*Scalar(z));
				glVertex(p);
				p[1]+=gridSize[1];
				glVertex(p);
				}
		
		/* Render grid lines in z-direction: */
		for(int x=0;x<=numCells[0];++x)
			for(int y=0;y<=numCells[1];++y)
				{
				Point p(cellSize[0]*Scalar(x),cellSize[1]*Scalar(y),Scalar(0));
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

/**************************************
Methods of class CartesianGridRenderer:
**************************************/

template <class DataSetParam>
inline
CartesianGridRenderer<DataSetParam>::CartesianGridRenderer(
	const typename CartesianGridRenderer<DataSetParam>::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class DataSetParam>
inline
int
CartesianGridRenderer<DataSetParam>::getNumRenderingModes(
	void)
	{
	return 3;
	}

template <class DataSetParam>
inline
const char*
CartesianGridRenderer<DataSetParam>::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=3)
		Misc::throwStdErr("CartesianGridRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[3]=
		{
		"Grid Outline","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class DataSetParam>
inline
void
CartesianGridRenderer<DataSetParam>::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=3)
		Misc::throwStdErr("CartesianGridRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class DataSetParam>
inline
void
CartesianGridRenderer<DataSetParam>::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the grid's outline: */
			CartesianGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridOutline(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		
		case 1:
			/* Render the grid's faces: */
			CartesianGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridFaces(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		
		case 2:
			/* Render the grid's cells: */
			CartesianGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridCells(dataSet->getCellSize(),dataSet->getNumCells());
			break;
		}
	}

template <class DataSetParam>
inline
void
CartesianGridRenderer<DataSetParam>::renderCell(
	const typename CartesianGridRenderer<DataSetParam>::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	CartesianGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
