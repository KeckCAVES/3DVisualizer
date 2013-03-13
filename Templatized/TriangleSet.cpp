/***********************************************************************
TriangleSet - Class to store visualization elements as sets of
unconnected triangles.
Copyright (c) 2005-2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_TRIANGLESET_IMPLEMENTATION

#include <Comm/MulticastPipe.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/TriangleSet.h>

namespace Visualization {

namespace Templatized {

/**************************************
Methods of class TriangleSet::DataItem:
**************************************/

template <class VertexParam>
inline
TriangleSet<VertexParam>::DataItem::DataItem(
	void)
	:vertexBufferId(0),
	 version(0),
	 numTriangles(0)
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
TriangleSet<VertexParam>::DataItem::~DataItem(
	void)
	{
	if(vertexBufferId!=0)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		}
	}

/****************************
Methods of class TriangleSet:
****************************/

template <class VertexParam>
inline
void
TriangleSet<VertexParam>::addNewChunk(
	void)
	{
	if(pipe!=0)
		{
		/* Check how many triangles in the last chunk need to be sent across the pipe: */
		size_t numUnsentTriangles;
		if(tail!=0&&(numUnsentTriangles=chunkSize-tailNumSentTriangles)>0)
			{
			/* Send unsent triangles in the last chunk across the pipe: */
			pipe->write<unsigned int>((unsigned int)numUnsentTriangles);
			pipe->write<Vertex>(tail->vertices+tailNumSentTriangles*3,numUnsentTriangles*3);
			pipe->finishMessage();
			}
		
		tailNumSentTriangles=0;
		}
	
	/* Add a new triangle chunk to the buffer: */
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

template <class VertexParam>
inline
TriangleSet<VertexParam>::TriangleSet(
	Comm::MulticastPipe* sPipe)
	:pipe(sPipe),
	 version(0),
	 numTriangles(0),
	 head(0),tail(0),
	 tailNumSentTriangles(0),
	 tailRoomLeft(0),
	 nextVertex(0)
	{
	}

template <class VertexParam>
inline
TriangleSet<VertexParam>::~TriangleSet(
	void)
	{
	/* Delete all triangle chunks: */
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
TriangleSet<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
void
TriangleSet<VertexParam>::clear(
	void)
	{
	++version;
	numTriangles=0;
	
	/* Delete all triangle chunks: */
	while(head!=0)
		{
		Chunk* succ=head->succ;
		delete head;
		head=succ;
		}
	tail=0;
	tailNumSentTriangles=0;
	tailRoomLeft=0;
	nextVertex=0;
	}

template <class VertexParam>
inline
void
TriangleSet<VertexParam>::receive(
	void)
	{
	/* Read while the number of triangles in the next batch is positive: */
	size_t numBatchTriangles;
	while((numBatchTriangles=pipe->read<unsigned int>())>0)
		{
		/* Read the triangle data one chunk at a time: */
		while(numBatchTriangles>0)
			{
			if(tailRoomLeft==0)
				{
				/* Add a new triangle chunk to the buffer: */
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
			
			/* Receive as many triangles as the current chunk can hold: */
			size_t numReadTriangles=numBatchTriangles;
			if(numReadTriangles>tailRoomLeft)
				numReadTriangles=tailRoomLeft;
			pipe->read<Vertex>(nextVertex,numReadTriangles*3);
			numBatchTriangles-=numReadTriangles;
			
			/* Update the vertex storage: */
			numTriangles+=numReadTriangles;
			tailRoomLeft-=numReadTriangles;
			nextVertex+=numReadTriangles*3;
			}
		}
	}

template <class VertexParam>
inline
void
TriangleSet<VertexParam>::flush(
	void)
	{
	if(pipe!=0)
		{
		/* Send all unsent triangles across the pipe: */
		size_t numUnsentTriangles;
		if(tail!=0&&(numUnsentTriangles=chunkSize-tailRoomLeft-tailNumSentTriangles)>0)
			{
			pipe->write<unsigned int>((unsigned int)numUnsentTriangles);
			pipe->write<Vertex>(tail->vertices+tailNumSentTriangles*3,numUnsentTriangles*3);
			tailNumSentTriangles+=numUnsentTriangles;
			}
		
		/* Send a flush signal: */
		pipe->write<unsigned int>(0);
		pipe->finishMessage();
		}
	}

template <class VertexParam>
inline
void
TriangleSet<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Save the current number of triangles (for parallel creation and rendering): */
	size_t numRenderTriangles=numTriangles;
	
	/* Render the current amount of triangles: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	if(dataItem->vertexBufferId!=0)
		{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		
		#if 0
		/* Render all triangle set chunks in streaming mode: */
		glVertexPointer(static_cast<const Vertex*>(0));
		size_t numTrianglesLeft=numRenderTriangles;
		for(const Chunk* chPtr=head;numTrianglesLeft>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of triangles in this chunk: */
			size_t numChunkTriangles=numTrianglesLeft;
			if(numChunkTriangles>chunkSize)
				numChunkTriangles=chunkSize;
			
			/* Upload the triangles: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,numChunkTriangles*3*sizeof(Vertex),chPtr->vertices,GL_STREAM_DRAW_ARB);
			glDrawArrays(GL_TRIANGLES,0,numChunkTriangles*3);
			numTrianglesLeft-=numChunkTriangles;
			}
		#else
		/* Check if the vertex buffer is current: */
		if(dataItem->version!=version||dataItem->numTriangles!=numRenderTriangles)
			{
			/* Upload the triangles to the vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,numRenderTriangles*3*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
			GLintptrARB offset=0;
			size_t numTrianglesLeft=numRenderTriangles;
			for(const Chunk* chPtr=head;numTrianglesLeft>0;chPtr=chPtr->succ)
				{
				/* Calculate the number of triangles in this chunk: */
				size_t numChunkTriangles=numTrianglesLeft;
				if(numChunkTriangles>chunkSize)
					numChunkTriangles=chunkSize;
				
				/* Upload the triangles: */
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,offset,numChunkTriangles*3*sizeof(Vertex),chPtr->vertices);
				numTrianglesLeft-=numChunkTriangles;
				offset+=numChunkTriangles*3*sizeof(Vertex);
				}
			
			dataItem->version=version;
			dataItem->numTriangles=numRenderTriangles;
			}
		
		/* Render the triangles: */
		glVertexPointer(static_cast<const Vertex*>(0));
		glDrawArrays(GL_TRIANGLES,0,numRenderTriangles*3);
		#endif
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		for(const Chunk* chPtr=head;numRenderTriangles>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of triangles in this chunk: */
			size_t numChunkTriangles=numRenderTriangles;
			if(numChunkTriangles>chunkSize)
				numChunkTriangles=chunkSize;

			/* Draw the triangles: */
			glVertexPointer(chPtr->vertices);
			glDrawArrays(GL_TRIANGLES,0,numChunkTriangles*3);
			numRenderTriangles-=numChunkTriangles;
			}
		}
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
