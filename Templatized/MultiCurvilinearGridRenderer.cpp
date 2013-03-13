/***********************************************************************
MultiCurvilinearGridRenderer - Helper class to render multi-curvilinear
grids.
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

#define VISUALIZATION_TEMPLATIZED_MULTICURVILINEARGRIDRENDERER_IMPLEMENTATION

#include <Templatized/MultiCurvilinearGridRenderer.h>

#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

namespace Visualization {

namespace Templatized {

namespace MultiCurvilinearGridRendererImplementation {

/*******************************************************************************
Internal helper class to render multi-curvilinear grids of different dimensions:
*******************************************************************************/

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
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Grid Grid;
	typedef typename DataSet::Cell Cell;
	
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
	inline static void renderGridOutline(const Grid& grid)
		{
		const Index& numVertices=grid.getNumVertices();
		Index index;
		
		/* Render grid's outline: */
		glBegin(GL_LINE_LOOP);
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0]-1;++index[0])
			glVertex(grid.getVertexPosition(index));
		for(index[1]=0;index[1]<numVertices[1]-1;++index[1])
			glVertex(grid.getVertexPosition(index));
		for(index[0]=numVertices[0]-1;index[0]>0;--index[0])
			glVertex(grid.getVertexPosition(index));
		for(index[1]=numVertices[1]-1;index[1]>0;--index[1])
			glVertex(grid.getVertexPosition(index));
		glEnd();
		}
	inline static void renderGridFaces(const Grid& grid,int faceMask)
		{
		const Index& numVertices=grid.getNumVertices();
		Index index;
		
		/* Render each marked face: */
		if(faceMask&0x1)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=0;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(grid.getVertexPosition(index));
			glEnd();
			}
		if(faceMask&0x2)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=numVertices[0]-1;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(grid.getVertexPosition(index));
			glEnd();
			}
		if(faceMask&0x4)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=0;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(grid.getVertexPosition(index));
			glEnd();
			}
		if(faceMask&0x8)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=numVertices[1]-1;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(grid.getVertexPosition(index));
			glEnd();
			}
		}
	inline static void renderGridCells(const Grid& grid)
		{
		const Index& numVertices=grid.getNumVertices();
		Index index;
		
		/* Render grid lines along y direction: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			glBegin(GL_LINE_STRIP);
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(vertices(index).pos);
			glEnd();
			}
		
		/* Render grid lines along x direction: */
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			glBegin(GL_LINE_STRIP);
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(vertices(index).pos);
			glEnd();
			}
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
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Grid Grid;
	typedef typename DataSet::Cell Cell;
	
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
	inline static void renderGridLine(const Grid& grid,const Index& startIndex,int axis)
		{
		/* Render grid line: */
		glBegin(GL_LINE_STRIP);
		Index index=startIndex;
		for(index[axis]=0;index[axis]<grid.getNumVertices()[axis];++index[axis])
			glVertex(grid.getVertexPosition(index));
		glEnd();
		}
	inline static void renderGridOutline(const Grid& grid)
		{
		const Index& numVertices=grid.getNumVertices();
		
		/* Render grid outlines: */
		Index index(0,0,0);
		renderGridLine(grid,index,0);
		index[1]=numVertices[1]-1;
		renderGridLine(grid,index,0);
		index[2]=numVertices[2]-1;
		renderGridLine(grid,index,0);
		index[1]=0;
		renderGridLine(grid,index,0);
		
		index[2]=0;
		renderGridLine(grid,index,1);
		index[0]=numVertices[0]-1;
		renderGridLine(grid,index,1);
		index[2]=numVertices[2]-1;
		renderGridLine(grid,index,1);
		index[0]=0;
		renderGridLine(grid,index,1);
		
		index[2]=0;
		renderGridLine(grid,index,2);
		index[0]=numVertices[0]-1;
		renderGridLine(grid,index,2);
		index[1]=numVertices[1]-1;
		renderGridLine(grid,index,2);
		index[0]=0;
		renderGridLine(grid,index,2);
		}
	inline static void renderGridFaces(const Grid& grid,int faceMask)
		{
		const Index& numVertices=grid.getNumVertices();
		Index index;
		
		/* Render grid lines in (y,z)-plane: */
		index[1]=0;
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			{
			index[0]=0;
			if(faceMask&0x01)
				renderGridLine(grid,index,1);
			index[0]=numVertices[0]-1;
			if(faceMask&0x02)
				renderGridLine(grid,index,1);
			}
		index[2]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			index[0]=0;
			if(faceMask&0x01)
				renderGridLine(grid,index,2);
			index[0]=numVertices[0]-1;
			if(faceMask&0x02)
				renderGridLine(grid,index,2);
			}
		
		/* Render grid lines in (x,z)-plane: */
		index[0]=0;
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			{
			index[1]=0;
			if(faceMask&0x04)
				renderGridLine(grid,index,0);
			index[1]=numVertices[1]-1;
			if(faceMask&0x08)
				renderGridLine(grid,index,0);
			}
		index[2]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			index[1]=0;
			if(faceMask&0x04)
				renderGridLine(grid,index,2);
			index[1]=numVertices[1]-1;
			if(faceMask&0x08)
				renderGridLine(grid,index,2);
			}
		
		/* Render grid lines in (x,y)-plane: */
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			index[2]=0;
			if(faceMask&0x10)
				renderGridLine(grid,index,0);
			index[2]=numVertices[2]-1;
			if(faceMask&0x20)
				renderGridLine(grid,index,0);
			}
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			index[2]=0;
			if(faceMask&0x10)
				renderGridLine(grid,index,1);
			index[2]=numVertices[2]-1;
			if(faceMask&0x20)
				renderGridLine(grid,index,1);
			}
		}
	inline static void renderGridCells(const Grid& grid)
		{
		const Index& numVertices=grid.getNumVertices();
		Index index;
		
		/* Render grid lines along z-axis: */
		index[2]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				renderGridLine(grid,index,2);
		
		/* Render grid lines along y-axis: */
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderGridLine(grid,index,1);
		
		/* Render grid lines along x-axis: */
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderGridLine(grid,index,0);
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

/*********************************************
Methods of class MultiCurvilinearGridRenderer:
*********************************************/

template <class DataSetParam>
inline
MultiCurvilinearGridRenderer<DataSetParam>::MultiCurvilinearGridRenderer(
	const typename MultiCurvilinearGridRenderer<DataSetParam>::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class DataSetParam>
inline
int
MultiCurvilinearGridRenderer<DataSetParam>::getNumRenderingModes(
	void)
	{
	return 5;
	}

template <class DataSetParam>
inline
const char*
MultiCurvilinearGridRenderer<DataSetParam>::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=5)
		Misc::throwStdErr("MultiCurvilinearGridRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[5]=
		{
		"Bounding Box","Grid Outline","Grid Boundary Faces","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class DataSetParam>
inline
void
MultiCurvilinearGridRenderer<DataSetParam>::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=5)
		Misc::throwStdErr("MultiCurvilinearGridRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class DataSetParam>
inline
void
MultiCurvilinearGridRenderer<DataSetParam>::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the grid's bounding box: */
			MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderBoundingBox(dataSet->getDomainBox());
			break;
			
		case 1:
			/* Render each grid's outline: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridOutline(dataSet->getGrid(gridIndex));
			break;
		
		case 2:
			/* Render the grid's boundary faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				{
				int faceMask=0x0;
				for(int faceIndex=0;faceIndex<DataSetParam::dimension*2;++faceIndex)
					if(dataSet->isBoundaryFace(gridIndex,faceIndex))
						faceMask|=1<<faceIndex;
				MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridFaces(dataSet->getGrid(gridIndex),faceMask);
				}
			break;
		
		case 3:
			/* Render the grid's faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridFaces(dataSet->getGrid(gridIndex),~0x0);
			break;
		
		case 4:
			/* Render the grid's cells: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderGridCells(dataSet->getGrid(gridIndex));
			break;
		}
	}

template <class DataSetParam>
inline
void
MultiCurvilinearGridRenderer<DataSetParam>::renderCell(
	const typename MultiCurvilinearGridRenderer<DataSetParam>::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	MultiCurvilinearGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
