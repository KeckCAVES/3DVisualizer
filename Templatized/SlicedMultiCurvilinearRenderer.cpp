/***********************************************************************
SlicedMultiCurvilinearRenderer - Class to render sliced multi-grid
curvilinear data sets. Implemented as a specialization of the generic
DataSetRenderer class. Evil hack to test SlicedMultiCurvilinear class;
needs to be joined with regular MultiCurvilinearRenderer, since neither
uses data values.
Copyright (c) 2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_SLICEDMULTICURVILINEARRENDERER_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>

#include <Templatized/SlicedMultiCurvilinearRenderer.h>

namespace Visualization {

namespace Templatized {

namespace SlicedMultiCurvilinearRendererImplementation {

/*************************************************************************
Internal helper class to render curvilinear grids of different dimensions:
*************************************************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class ScalarParam,class ValueScalarParam>
class GridRenderer<ScalarParam,2,ValueScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef SlicedMultiCurvilinear<ScalarParam,2,ValueScalarParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::GridArray GridArray;
	typedef typename DataSet::Index Index;
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
	inline static void renderGridOutline(const GridArray& grid)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
		Index index;
		
		/* Render grid's outline: */
		glBegin(GL_LINE_STRIP);
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			glVertex(grid(index));
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[1]=numVertices[1]-1;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			glVertex(grid(index));
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			glVertex(grid(index));
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[0]=numVertices[0]-1;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			glVertex(grid(index));
		glEnd();
		}
	inline static void renderGridFaces(const GridArray& grid,int faceMask)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
		Index index;
		
		/* Render grid's faces: */
		if(faceMask&0x1)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=0;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(grid(index));
			glEnd();
			}
		if(faceMask&0x2)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=numVertices[0]-1;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(grid(index));
			glEnd();
			}
		if(faceMask&0x4)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=0;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(grid(index));
			glEnd();
			}
		if(faceMask&0x8)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=numVertices[1]-1;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(grid(index));
			glEnd();
			}
		}
	inline static void renderGridCells(const GridArray& grid)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
		Index index;
		
		/* Render grid lines along y direction: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			glBegin(GL_LINE_STRIP);
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(grid(index));
			glEnd();
			}
		
		/* Render grid lines along x direction: */
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			glBegin(GL_LINE_STRIP);
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(grid(index));
			glEnd();
			}
		}
	inline static void highlightCell(const Cell& cell)
		{
		glBegin(GL_LINE_LOOP);
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(2));
		glEnd();
		}
	};

template <class ScalarParam,class ValueScalarParam>
class GridRenderer<ScalarParam,3,ValueScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef SlicedMultiCurvilinear<ScalarParam,3,ValueScalarParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::GridArray GridArray;
	typedef typename DataSet::Index Index;
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
	inline static void renderGridLine(const GridArray& grid,const Index& startIndex,int axis)
		{
		/* Get pointer to first grid vertex: */
		const typename DataSet::Point* vPtr=grid.getAddress(startIndex);
		int numVertices=grid.getSize(axis);
		int stride=grid.getIncrement(axis);
		
		/* Render grid line: */
		glBegin(GL_LINE_STRIP);
		for(int i=0;i<numVertices;++i,vPtr+=stride)
			glVertex(*vPtr);
		glEnd();
		}
	inline static void renderGridOutline(const GridArray& grid)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
		/* Render grid outlines: */
		renderGridLine(grid,Index(0,0,0),0);
		renderGridLine(grid,Index(0,numVertices[1]-1,0),0);
		renderGridLine(grid,Index(0,numVertices[1]-1,numVertices[2]-1),0);
		renderGridLine(grid,Index(0,0,numVertices[2]-1),0);
		renderGridLine(grid,Index(0,0,0),1);
		renderGridLine(grid,Index(numVertices[0]-1,0,0),1);
		renderGridLine(grid,Index(numVertices[0]-1,0,numVertices[2]-1),1);
		renderGridLine(grid,Index(0,0,numVertices[2]-1),1);
		renderGridLine(grid,Index(0,0,0),2);
		renderGridLine(grid,Index(numVertices[0]-1,0,0),2);
		renderGridLine(grid,Index(numVertices[0]-1,numVertices[1]-1,0),2);
		renderGridLine(grid,Index(0,numVertices[1]-1,0),2);
		}
	inline static void renderGridFaces(const GridArray& grid,int faceMask)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
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
	inline static void renderGridCells(const GridArray& grid)
		{
		/* Get vertex array size: */
		const Index& numVertices=grid.getSize();
		
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

/********************************************************
Methods of class DataSetRenderer<SlicedMultiCurvilinear>:
********************************************************/

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::DataSetRenderer(
	const typename DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::~DataSetRenderer(
	void)
	{
	/* Nothing to do yet... */
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
int
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::getNumRenderingModes(
	void)
	{
	return 5;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
const char*
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=5)
		Misc::throwStdErr("DataSetRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[5]=
		{
		"Bounding Box","Grid Outline","Grid Boundary Faces","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=5)
		Misc::throwStdErr("DataSetRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the data set's bounding box: */
			SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::renderBoundingBox(dataSet->getDomainBox());
			break;
		
		case 1:
			/* Render each grid's outline: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
			
SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::renderGridOutline(dataSet->getGrid(gridIndex).getGrid());
			break;
		
		case 2:
			{
			/* Render each grid's faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				{
				int faceMask=0x0;
				for(int faceIndex=0;faceIndex<dimensionParam*2;++faceIndex)
					if(dataSet->isBoundaryFace(gridIndex,faceIndex))
						faceMask|=1<<faceIndex;
				SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::renderGridFaces(dataSet->getGrid(gridIndex).getGrid(),faceMask);
				}
			break;
			}
		
		case 3:
			/* Render each grid's faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::renderGridFaces(dataSet->getGrid(gridIndex).getGrid(),~0x0);
			break;
		
		case 4:
			/* Render each grid's cells: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::renderGridCells(dataSet->getGrid(gridIndex).getGrid());
			break;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueScalarParam>
inline
void
DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::renderCell(
	const typename DataSetRenderer<SlicedMultiCurvilinear<ScalarParam,dimensionParam,ValueScalarParam> >::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	SlicedMultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueScalarParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
