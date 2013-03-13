/***********************************************************************
Polyline - Class to represent arbitrary-length polylines.
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

#define VISUALIZATION_TEMPLATIZED_POLYLINE_IMPLEMENTATION

#include <Comm/MulticastPipe.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/Polyline.h>

namespace Visualization {

namespace Templatized {

/***********************************
Methods of class Polyline::DataItem:
***********************************/

template <class VertexParam>
inline
Polyline<VertexParam>::DataItem::DataItem(
	void)
	:vertexBufferId(0),
	 version(0),
	 numVertices(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		}
	}

template <class VertexParam>
inline
Polyline<VertexParam>::DataItem::~DataItem(
	void)
	{
	if(vertexBufferId!=0)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		}
	}

/*************************
Methods of class Polyline:
*************************/

template <class VertexParam>
inline
void
Polyline<VertexParam>::addNewChunk(
	void)
	{
	if(pipe!=0)
		{
		/* Check how many vertices in the last chunk need to be sent across the pipe: */
		size_t numUnsentVertices;
		if(tail!=0&&(numUnsentVertices=chunkSize-tailNumSentVertices)>0)
			{
			/* Send unsent vertices in the last chunk across the pipe: */
			pipe->write<unsigned int>((unsigned int)numUnsentVertices);
			pipe->write<Vertex>(tail->vertices+tailNumSentVertices,numUnsentVertices);
			pipe->finishMessage();
			}
		
		tailNumSentVertices=0;
		}
	
	/* Add a new vertex chunk to the buffer: */
	Chunk* newChunk=new Chunk;
	if(tail!=0)
		tail->succ=newChunk;
	else
		head=newChunk;
	Chunk* oldTail=tail;
	tail=newChunk;
	
	/* Set up the vertex pointer: */
	tailRoomLeft=chunkSize;
	nextVertex=tail->vertices;
	
	if(newChunk!=head)
		{
		/* Copy the last vertex of the previous chunk to render a continuous polyline: */
		*nextVertex=oldTail->vertices[chunkSize-1];
		++numVertices;
		--tailRoomLeft;
		++nextVertex;
		}
	}

template <class VertexParam>
inline
Polyline<VertexParam>::Polyline(
	Comm::MulticastPipe* sPipe)
	:pipe(sPipe),
	 version(0),
	 numVertices(0),
	 head(0),tail(0),
	 tailNumSentVertices(0),
	 tailRoomLeft(0),
	 nextVertex(0)
	{
	}

template <class VertexParam>
inline
Polyline<VertexParam>::~Polyline(
	void)
	{
	/* Delete all vertex buffer chunks: */
	while(head!=0)
		{
		Chunk* succ=head->succ;
		delete head;
		head=succ;
		}
	}

template <class VertexParam>
inline
void
Polyline<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
void
Polyline<VertexParam>::clear(
	void)
	{
	++version;
	numVertices=0;
	
	/* Delete all vertex buffer chunks: */
	while(head!=0)
		{
		Chunk* succ=head->succ;
		delete head;
		head=succ;
		}
	tail=0;
	tailRoomLeft=0;
	nextVertex=0;
	}

template <class VertexParam>
inline
void
Polyline<VertexParam>::receive(
	void)
	{
	/* Read while the number of vertices in the next batch is positive: */
	size_t numBatchVertices;
	while((numBatchVertices=pipe->read<unsigned int>())>0)
		{
		/* Read the vertex data one chunk at a time: */
		while(numBatchVertices>0)
			{
			if(tailRoomLeft==0)
				{
				/* Add a new vertex chunk to the buffer: */
				Chunk* newChunk=new Chunk;
				if(tail!=0)
					tail->succ=newChunk;
				else
					head=newChunk;
				tail=newChunk;

				/* Set up the vertex pointer: */
				tailRoomLeft=chunkSize;
				nextVertex=tail->vertices;
				}
			
			/* Receive as many vertices as the current chunk can hold: */
			size_t numReadVertices=numBatchVertices;
			if(numReadVertices>tailRoomLeft)
				numReadVertices=tailRoomLeft;
			pipe->read<Vertex>(nextVertex,numReadVertices);
			numBatchVertices-=numReadVertices;
			
			/* Update the vertex storage: */
			numVertices+=numReadVertices;
			tailRoomLeft-=numReadVertices;
			nextVertex+=numReadVertices;
			}
		}
	}

template <class VertexParam>
inline
void
Polyline<VertexParam>::flush(
	void)
	{
	if(pipe!=0)
		{
		/* Send all unsent vertices across the pipe: */
		size_t numUnsentVertices;
		if(tail!=0&&(numUnsentVertices=chunkSize-tailRoomLeft-tailNumSentVertices)>0)
			{
			pipe->write<unsigned int>((unsigned int)numUnsentVertices);
			pipe->write<Vertex>(tail->vertices+tailNumSentVertices,numUnsentVertices);
			tailNumSentVertices+=numUnsentVertices;
			}
		
		/* Send a flush signal: */
		pipe->write<unsigned int>(0);
		pipe->finishMessage();
		}
	}

template <class VertexParam>
inline
void
Polyline<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Save the current number of vertices (for parallel creation and rendering): */
	size_t numRenderVertices=numVertices;
	
	/* Render the current amount of vertices: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	if(dataItem->vertexBufferId!=0)
		{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		
		/* Check if the vertex buffer is current: */
		if(dataItem->version!=version||dataItem->numVertices!=numRenderVertices)
			{
			/* Upload the vertices to the vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,numRenderVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
			GLintptrARB offset=0;
			size_t numVerticesLeft=numRenderVertices;
			for(const Chunk* chPtr=head;numVerticesLeft>0;chPtr=chPtr->succ)
				{
				/* Calculate the number of vertices in this chunk: */
				size_t numChunkVertices=numVerticesLeft;
				if(numChunkVertices>chunkSize)
					numChunkVertices=chunkSize;
				
				/* Upload the vertices: */
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,offset,numChunkVertices*sizeof(Vertex),chPtr->vertices);
				numVerticesLeft-=numChunkVertices;
				offset+=numChunkVertices*sizeof(Vertex);
				}
			
			dataItem->version=version;
			dataItem->numVertices=numRenderVertices;
			}
		
		/* Render the triangles: */
		glVertexPointer(static_cast<const Vertex*>(0));
		glDrawArrays(GL_LINE_STRIP,0,numRenderVertices);
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		for(const Chunk* chPtr=head;numRenderVertices>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of vertices in this chunk: */
			size_t numChunkVertices=numRenderVertices;
			if(numChunkVertices>chunkSize)
				numChunkVertices=chunkSize;

			/* Draw the partial polyline: */
			glVertexPointer(chPtr->vertices);
			glDrawArrays(GL_LINE_STRIP,0,numChunkVertices);
			numRenderVertices-=numChunkVertices;
			}
		}
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
