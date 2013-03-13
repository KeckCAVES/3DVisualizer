/***********************************************************************
IndexedTrianglestripSet - Class to represent surfaces as sets of
triangle strips sharing vertices.
Copyright (c) 2006-2007 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESTRIPSET_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/IndexedTrianglestripSet.h>

namespace Visualization {

namespace Templatized {

/**************************************************
Methods of class IndexedTrianglestripSet::DataItem:
**************************************************/

template <class VertexParam>
inline
IndexedTrianglestripSet<VertexParam>::DataItem::DataItem(
	void)
	:vertexBufferId(0),indexBufferId(0),
	 version(0),
	 numVertices(0),numIndices(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		
		/* Create an index buffer object: */
		glGenBuffersARB(1,&indexBufferId);
		}
	else
		Misc::throwStdErr("IndexedTrianglestripSet::DataItem::DataItem: GL_ARB_vertex_buffer_object extension not supported");
	}

template <class VertexParam>
inline
IndexedTrianglestripSet<VertexParam>::DataItem::~DataItem(
	void)
	{
	/* Delete the vertex buffer object: */
	glDeleteBuffersARB(1,&vertexBufferId);
	
	/* Delete the index buffer object: */
	glDeleteBuffersARB(1,&indexBufferId);
	}

/****************************************
Methods of class IndexedTrianglestripSet:
****************************************/

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::addNewVertexChunk(
	void)
	{
	/* Add a new vertex chunk to the buffer: */
	VertexChunk* newVertexChunk=new VertexChunk;
	if(vertexTail!=0)
		vertexTail->succ=newVertexChunk;
	else
		vertexHead=newVertexChunk;
	vertexTail=newVertexChunk;
	
	/* Set up the vertex pointer: */
	numVerticesLeft=vertexChunkSize;
	nextVertex=vertexTail->vertices;
	}

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::addNewIndexChunk(
	void)
	{
	/* Add a new index chunk to the buffer: */
	IndexChunk* newIndexChunk=new IndexChunk;
	if(indexTail!=0)
		indexTail->succ=newIndexChunk;
	else
		indexHead=newIndexChunk;
	indexTail=newIndexChunk;
	
	/* Set up the index pointer: */
	numIndicesLeft=indexChunkSize;
	nextIndex=indexTail->indices;
	}

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::addNewStripChunk(
	void)
	{
	/* Add a new strip chunk to the buffer: */
	StripChunk* newStripChunk=new StripChunk;
	if(stripTail!=0)
		stripTail->succ=newStripChunk;
	else
		stripHead=newStripChunk;
	stripTail=newStripChunk;
	
	/* Set up the strip pointer: */
	numStripsLeft=stripChunkSize;
	nextStrip=stripTail->lengths;
	}

template <class VertexParam>
inline
IndexedTrianglestripSet<VertexParam>::IndexedTrianglestripSet(
	void)
	:version(0),
	 numVertices(0),numIndices(0),numStrips(0),
	 vertexHead(0),vertexTail(0),
	 indexHead(0),indexTail(0),
	 stripHead(0),stripTail(0),
	 numVerticesLeft(0),numIndicesLeft(0),numStripsLeft(0),
	 nextVertex(0),nextIndex(0),currentStripLength(0),nextStrip(0)
	{
	}

template <class VertexParam>
inline
IndexedTrianglestripSet<VertexParam>::~IndexedTrianglestripSet(
	void)
	{
	/* Delete all vertex chunks: */
	while(vertexHead!=0)
		{
		VertexChunk* succ=vertexHead->succ;
		delete vertexHead;
		vertexHead=succ;
		}
	
	/* Delete all index chunks: */
	while(indexHead!=0)
		{
		IndexChunk* succ=indexHead->succ;
		delete indexHead;
		indexHead=succ;
		}
	
	/* Delete all strip chunks: */
	while(stripHead!=0)
		{
		StripChunk* succ=stripHead->succ;
		delete stripHead;
		stripHead=succ;
		}
	}

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::clear(
	void)
	{
	++version;
	numVertices=0;
	numIndices=0;
	numStrips=0;
	
	/* Delete all vertex chunks: */
	while(vertexHead!=0)
		{
		VertexChunk* succ=vertexHead->succ;
		delete vertexHead;
		vertexHead=succ;
		}
	vertexTail=0;
	numVerticesLeft=0;
	nextVertex=0;
	
	/* Delete all index chunks: */
	while(indexHead!=0)
		{
		IndexChunk* succ=indexHead->succ;
		delete indexHead;
		indexHead=succ;
		}
	indexTail=0;
	numIndicesLeft=0;
	nextIndex=0;
	currentStripLength=0;
	
	/* Delete all strip chunks: */
	while(stripHead!=0)
		{
		StripChunk* succ=stripHead->succ;
		delete stripHead;
		stripHead=succ;
		}
	stripTail=0;
	numStripsLeft=0;
	nextStrip=0;
	}

template <class VertexParam>
inline
void
IndexedTrianglestripSet<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Save the current number of vertices, indices, and strips (for parallel creation and rendering): */
	size_t numRenderStrips=numStrips;
	size_t numRenderIndices=numIndices;
	size_t numRenderVertices=numVertices;
	
	/* Render the current amount of triangles: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
	
	/* Update the vertex and index buffers: */
	if(dataItem->version!=version||dataItem->numVertices!=numRenderVertices)
		{
		/* Upload the vertex data into the vertex buffer: */
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,numRenderVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		GLintptrARB offset=0;
		size_t verticesToCopy=numRenderVertices;
		for(const VertexChunk* chPtr=vertexHead;verticesToCopy>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of vertices in this chunk: */
			size_t numChunkVertices=verticesToCopy;
			if(numChunkVertices>vertexChunkSize)
				numChunkVertices=vertexChunkSize;
			
			/* Upload the vertices: */
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,offset,numChunkVertices*sizeof(Vertex),chPtr->vertices);
			verticesToCopy-=numChunkVertices;
			offset+=numChunkVertices*sizeof(Vertex);
			}
		dataItem->numVertices=numRenderVertices;
		}
	
	if(dataItem->version!=version||dataItem->numIndices!=numRenderIndices)
		{
		/* Upload the index data into the index buffer: */
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numRenderIndices*sizeof(Index),0,GL_STATIC_DRAW_ARB);
		GLintptrARB offset=0;
		size_t indicesToCopy=numRenderIndices;
		for(const IndexChunk* chPtr=indexHead;indicesToCopy>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of indices in this chunk: */
			size_t numChunkIndices=indicesToCopy;
			if(numChunkIndices>indexChunkSize)
				numChunkIndices=indexChunkSize;
			
			/* Upload the vertex indices: */
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,offset,numChunkIndices*sizeof(Index),chPtr->indices);
			indicesToCopy-=numChunkIndices;
			offset+=numChunkIndices*sizeof(Index);
			}
		dataItem->numIndices=numRenderIndices;
		}
	
	dataItem->version=version;
	
	/* Render the triangle strips: */
	glVertexPointer(static_cast<const Vertex*>(0));
	const GLubyte* stripBaseIndexPtr=0;
	for(const StripChunk* chPtr=stripHead;numRenderStrips>0;chPtr=chPtr->succ)
		{
		/* Calculate the number of strips in this chunk: */
		size_t numChunkStrips=numRenderStrips;
		if(numChunkStrips>stripChunkSize)
			numChunkStrips=stripChunkSize;
		
		/* Render all triangle strips in this chunk: */
		for(size_t i=0;i<numChunkStrips;++i)
			{
			// glDrawRangeElements(GL_TRIANGLE_STRIP,0,GLuint(numRenderVertices)-1,chPtr->lengths[i],GL_UNSIGNED_INT,stripBaseIndexPtr);
			glDrawElements(GL_TRIANGLE_STRIP,chPtr->lengths[i],GL_UNSIGNED_INT,stripBaseIndexPtr);
			stripBaseIndexPtr+=chPtr->lengths[i]*sizeof(GLuint);
			}
		numRenderStrips-=numChunkStrips;
		}
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
