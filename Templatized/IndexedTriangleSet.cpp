/***********************************************************************
IndexedTriangleSet - Class to represent surfaces as sets of triangles
sharing vertices.
Copyright (c) 2006-2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESET_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Comm/MulticastPipe.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/IndexedTriangleSet.h>

namespace Visualization {

namespace Templatized {

/*********************************************
Methods of class IndexedTriangleSet::DataItem:
*********************************************/

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::DataItem::DataItem(
	void)
	:vertexBufferId(0),indexBufferId(0),
	 version(0),
	 numVertices(0),numTriangles(0)
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
		Misc::throwStdErr("IndexedTriangleSet::DataItem::DataItem: GL_ARB_vertex_buffer_object extension not supported");
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::DataItem::~DataItem(
	void)
	{
	if(vertexBufferId!=0||indexBufferId!=0)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		
		/* Delete the index buffer object: */
		glDeleteBuffersARB(1,&indexBufferId);
		}
	}

/***********************************
Methods of class IndexedTriangleSet:
***********************************/

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::addNewVertexChunk(
	void)
	{
	if(pipe!=0)
		{
		/* Check how many vertices in the last chunk need to be sent across the pipe: */
		size_t numUnsentVertices;
		if(vertexTail!=0&&(numUnsentVertices=vertexChunkSize-tailNumSentVertices)>0)
			{
			/* Send unsent vertices in the last chunk across the pipe: */
			pipe->write<unsigned int>((unsigned int)numUnsentVertices);
			pipe->write<unsigned int>(0U);
			pipe->write<Vertex>(vertexTail->vertices+tailNumSentVertices,numUnsentVertices);
			pipe->finishMessage();
			}
		
		tailNumSentVertices=0;
		}
	
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
IndexedTriangleSet<VertexParam>::addNewIndexChunk(
	void)
	{
	if(pipe!=0)
		{
		/* Check how many triangles in the last chunk need to be sent across the pipe: */
		size_t numUnsentTriangles;
		if(indexTail!=0&&(numUnsentTriangles=indexChunkSize-tailNumSentTriangles)>0)
			{
			/* Check how many vertices in the last chunk need to be sent across the pipe to keep the indexed triangle set consistent on the other side: */
			size_t numUnsentVertices=vertexTail!=0?vertexChunkSize-numVerticesLeft-tailNumSentVertices:0;
			
			/* Send unsent vertices and triangles in the last chunks across the pipe: */
			pipe->write<unsigned int>(numUnsentVertices);
			pipe->write<unsigned int>(numUnsentTriangles);
			if(numUnsentVertices>0)
				{
				pipe->write<Vertex>(vertexTail->vertices+tailNumSentVertices,numUnsentVertices);
				tailNumSentVertices+=numUnsentVertices;
				}
			pipe->write<Index>(indexTail->indices+tailNumSentTriangles*3,numUnsentTriangles*3);
			pipe->finishMessage();
			}
		
		tailNumSentTriangles=0;
		}
	
	/* Add a new index chunk to the buffer: */
	IndexChunk* newIndexChunk=new IndexChunk;
	if(indexTail!=0)
		indexTail->succ=newIndexChunk;
	else
		indexHead=newIndexChunk;
	indexTail=newIndexChunk;
	
	/* Set up the triangle (vertex triple) pointer: */
	numTrianglesLeft=indexChunkSize;
	nextTriangle=indexTail->indices;
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::IndexedTriangleSet(
	Comm::MulticastPipe* sPipe)
	:pipe(sPipe),
	 version(0),
	 numVertices(0),numTriangles(0),
	 vertexHead(0),vertexTail(0),
	 indexHead(0),indexTail(0),
	 tailNumSentVertices(0),tailNumSentTriangles(0),
	 numVerticesLeft(0),numTrianglesLeft(0),
	 nextVertex(0),nextTriangle(0)
	{
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::~IndexedTriangleSet(
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
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::clear(
	void)
	{
	++version;
	numVertices=0;
	numTriangles=0;
	
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
	numTrianglesLeft=0;
	nextTriangle=0;
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::receive(
	void)
	{
	while(true)
		{
		/* Read the number of vertices and triangles in the next batch: */
		size_t numBatchVertices=pipe->read<unsigned int>();
		size_t numBatchTriangles=pipe->read<unsigned int>();
		
		/* Stop reading if a flush was signaled: */
		if(numBatchVertices==0&&numBatchTriangles==0)
			break;
		
		/* Read the vertex data one chunk at a time: */
		while(numBatchVertices>0)
			{
			if(numVerticesLeft==0)
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
			
			/* Receive as many vertices as the current chunk can hold: */
			size_t numReadVertices=numBatchVertices;
			if(numReadVertices>numVerticesLeft)
				numReadVertices=numVerticesLeft;
			pipe->read<Vertex>(nextVertex,numReadVertices);
			numBatchVertices-=numReadVertices;
			
			/* Update the vertex storage: */
			numVertices+=numReadVertices;
			numVerticesLeft-=numReadVertices;
			nextVertex+=numReadVertices;
			}
		
		/* Read the triangle data one chunk at a time: */
		while(numBatchTriangles>0)
			{
			if(numTrianglesLeft==0)
				{
				/* Add a new index chunk to the buffer: */
				IndexChunk* newIndexChunk=new IndexChunk;
				if(indexTail!=0)
					indexTail->succ=newIndexChunk;
				else
					indexHead=newIndexChunk;
				indexTail=newIndexChunk;
				
				/* Set up the triangle (vertex triple) pointer: */
				numTrianglesLeft=indexChunkSize;
				nextTriangle=indexTail->indices;
				}
			
			/* Receive as many triangles as the current chunk can hold: */
			size_t numReadTriangles=numBatchTriangles;
			if(numReadTriangles>numTrianglesLeft)
				numReadTriangles=numTrianglesLeft;
			pipe->read<Index>(nextTriangle,numReadTriangles*3);
			numBatchTriangles-=numReadTriangles;
			
			/* Update the triangle storage: */
			numTriangles+=numReadTriangles;
			numTrianglesLeft-=numReadTriangles;
			nextTriangle+=numReadTriangles*3;
			}
		}
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::flush(
	void)
	{
	if(pipe!=0)
		{
		/* Check how many vertices and triangles need to be sent across the pipe: */
		size_t numUnsentVertices=vertexTail!=0?vertexChunkSize-numVerticesLeft-tailNumSentVertices:0;
		size_t numUnsentTriangles=indexTail!=0?indexChunkSize-numTrianglesLeft-tailNumSentTriangles:0;
		if(numUnsentVertices>0||numUnsentTriangles>0)
			{
			/* Send all unsent vertices and triangles across the pipe: */
			pipe->write<unsigned int>((unsigned int)numUnsentVertices);
			pipe->write<unsigned int>((unsigned int)numUnsentTriangles);
			if(numUnsentVertices>0)
				{
				pipe->write<Vertex>(vertexTail->vertices+tailNumSentVertices,numUnsentVertices);
				tailNumSentVertices+=numUnsentVertices;
				}
			if(numUnsentTriangles>0)
				{
				pipe->write<Index>(indexTail->indices+tailNumSentTriangles*3,numUnsentTriangles*3);
				tailNumSentTriangles+=numUnsentTriangles;
				}
			}
		
		/* Send a flush signal: */
		pipe->write<unsigned int>(0);
		pipe->write<unsigned int>(0);
		pipe->finishMessage();
		}
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Save the current number of vertices and triangles (for parallel creation and rendering): */
	size_t numRenderTriangles=numTriangles;
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
	
	if(dataItem->version!=version||dataItem->numTriangles!=numRenderTriangles)
		{
		/* Upload the index data into the index buffer: */
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numRenderTriangles*3*sizeof(Index),0,GL_STATIC_DRAW_ARB);
		GLintptrARB offset=0;
		size_t trianglesToCopy=numRenderTriangles;
		for(const IndexChunk* chPtr=indexHead;trianglesToCopy>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of triangles in this chunk: */
			size_t numChunkTriangles=trianglesToCopy;
			if(numChunkTriangles>indexChunkSize)
				numChunkTriangles=indexChunkSize;
			
			/* Upload the vertex indices: */
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,offset,numChunkTriangles*3*sizeof(Index),chPtr->indices);
			trianglesToCopy-=numChunkTriangles;
			offset+=numChunkTriangles*3*sizeof(Index);
			}
		dataItem->numTriangles=numRenderTriangles;
		}
	
	dataItem->version=version;
	
	/* Render the triangles: */
	glVertexPointer(static_cast<const Vertex*>(0));
	// glDrawRangeElements(GL_TRIANGLES,0,GLuint(numRenderVertices)-1,numRenderTriangles*3,GL_UNSIGNED_INT,static_cast<const Index*>(0));
	glDrawElements(GL_TRIANGLES,numRenderTriangles*3,GL_UNSIGNED_INT,static_cast<const Index*>(0));
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
