/***********************************************************************
MultiPolyline - Class to represent multiple arbitrary-length polylines.
Copyright (c) 2007-2008 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_MULTIPOLYLINE_IMPLEMENTATION

#include <Comm/MulticastPipe.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/MultiPolyline.h>

namespace Visualization {

namespace Templatized {

/****************************************
Methods of class MultiPolyline::Polyline:
****************************************/

template <class VertexParam>
inline
MultiPolyline<VertexParam>::Polyline::~Polyline(
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

/****************************************
Methods of class MultiPolyline::DataItem:
****************************************/

template <class VertexParam>
inline
MultiPolyline<VertexParam>::DataItem::DataItem(
	unsigned int sNumPolylines)
	:numPolylines(sNumPolylines),
	 vertexBufferIds(new GLuint[numPolylines]),
	 version(0),
	 numVertices(new size_t[numPolylines])
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a set of vertex buffer objects: */
		glGenBuffersARB(numPolylines,vertexBufferIds);
		}
	else
		vertexBufferIds[0]=0; // Serves as flag for the rest of the buffer
	
	/* Initialize polyline storage: */
	for(unsigned int i=0;i<numPolylines;++i)
		numVertices[i]=0;
	}

template <class VertexParam>
inline
MultiPolyline<VertexParam>::DataItem::~DataItem(
	void)
	{
	if(vertexBufferIds[0]!=0)
		{
		/* Delete the vertex buffer objects: */
		glDeleteBuffersARB(numPolylines,vertexBufferIds);
		}
	
	delete[] vertexBufferIds;
	delete[] numVertices;
	}

/******************************
Methods of class MultiPolyline:
******************************/

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::addNewChunk(
	unsigned int polylineIndex)
	{
	Polyline& p=polylines[polylineIndex];
	
	if(pipe!=0)
		{
		/* Check how many vertices in the last chunk need to be sent across the pipe: */
		size_t numUnsentVertices;
		if(p.tail!=0&&(numUnsentVertices=chunkSize-p.tailNumSentVertices)>0)
			{
			/* Send unsent vertices in the last chunk across the pipe: */
			pipe->write<unsigned int>(polylineIndex);
			pipe->write<unsigned int>((unsigned int)numUnsentVertices);
			pipe->write<Vertex>(p.tail->vertices+p.tailNumSentVertices,numUnsentVertices);
			pipe->finishMessage();
			}
		
		p.tailNumSentVertices=0;
		}
	
	/* Add a new vertex chunk to the buffer: */
	Chunk* newChunk=new Chunk;
	if(p.tail!=0)
		p.tail->succ=newChunk;
	else
		p.head=newChunk;
	Chunk* oldTail=p.tail;
	p.tail=newChunk;
	
	/* Set up the vertex pointer: */
	p.tailRoomLeft=chunkSize;
	p.nextVertex=p.tail->vertices;
	
	if(newChunk!=p.head)
		{
		/* Copy the last vertex of the previous chunk to render a continuous polyline: */
		*p.nextVertex=oldTail->vertices[chunkSize-1];
		++p.numVertices;
		--p.tailRoomLeft;
		++p.nextVertex;
		}
	}

template <class VertexParam>
inline
MultiPolyline<VertexParam>::MultiPolyline(
	unsigned int sNumPolylines,
	Comm::MulticastPipe* sPipe)
	:numPolylines(sNumPolylines),
	 pipe(sPipe),
	 version(0),
	 polylines(new Polyline[numPolylines]),
	 maxNumVertices(0)
	{
	}

template <class VertexParam>
inline
MultiPolyline<VertexParam>::~MultiPolyline(
	void)
	{
	delete[] polylines;
	}

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem(numPolylines);
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::clear(
	void)
	{
	++version;
	
	for(unsigned int polylineIndex=0;polylineIndex<numPolylines;++polylineIndex)
		{
		Polyline& p=polylines[polylineIndex];
		
		p.numVertices=0;
		
		/* Delete all vertex buffer chunks: */
		while(p.head!=0)
			{
			Chunk* succ=p.head->succ;
			delete p.head;
			p.head=succ;
			}
		p.tail=0;
		p.tailRoomLeft=0;
		p.nextVertex=0;
		}
	
	maxNumVertices=0;
	}

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::receive(
	void)
	{
	/* Read while the polyline index of the next batch is valid: */
	unsigned int polylineIndex;
	while((polylineIndex=pipe->read<unsigned int>())<numPolylines)
		{
		/* Read the number of vertices in the next batch: */
		size_t numBatchVertices=pipe->read<unsigned int>();
		
		/* Read the vertex data one chunk at a time: */
		Polyline& p=polylines[polylineIndex];
		while(numBatchVertices>0)
			{
			if(p.tailRoomLeft==0)
				{
				/* Add a new vertex chunk to the buffer: */
				Chunk* newChunk=new Chunk;
				if(p.tail!=0)
					p.tail->succ=newChunk;
				else
					p.head=newChunk;
				p.tail=newChunk;
				
				/* Set up the vertex pointer: */
				p.tailRoomLeft=chunkSize;
				p.nextVertex=p.tail->vertices;
				}
			
			/* Receive as many vertices as the current chunk can hold: */
			size_t numReadVertices=numBatchVertices;
			if(numReadVertices>p.tailRoomLeft)
				numReadVertices=p.tailRoomLeft;
			pipe->read<Vertex>(p.nextVertex,numReadVertices);
			numBatchVertices-=numReadVertices;
			
			/* Update the vertex storage: */
			p.numVertices+=numReadVertices;
			p.tailRoomLeft-=numReadVertices;
			p.nextVertex+=numReadVertices;
			}
		
		if(maxNumVertices<p.numVertices)
			maxNumVertices=p.numVertices;
		}
	}

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::flush(
	void)
	{
	if(pipe!=0)
		{
		for(unsigned int polylineIndex=0;polylineIndex<numPolylines;++polylineIndex)
			{
			Polyline& p=polylines[polylineIndex];
			
			/* Send all unsent vertices across the pipe: */
			size_t numUnsentVertices;
			if(p.tail!=0&&(numUnsentVertices=chunkSize-p.tailRoomLeft-p.tailNumSentVertices)>0)
				{
				pipe->write<unsigned int>(polylineIndex);
				pipe->write<unsigned int>((unsigned int)numUnsentVertices);
				pipe->write<Vertex>(p.tail->vertices+p.tailNumSentVertices,numUnsentVertices);
				p.tailNumSentVertices+=numUnsentVertices;
				}
			}
		
		/* Send a flush signal: */
		pipe->write<unsigned int>(numPolylines);
		pipe->finishMessage();
		}
	}

template <class VertexParam>
inline
void
MultiPolyline<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	if(dataItem->vertexBufferIds[0]!=0)
		{
		for(unsigned int polylineIndex=0;polylineIndex<numPolylines;++polylineIndex)
			{
			const Polyline& p=polylines[polylineIndex];
			
			/* Save the current number of vertices (for parallel creation and rendering): */
			size_t numRenderVertices=p.numVertices;
			
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferIds[polylineIndex]);
			
			/* Check if the vertex buffer is current: */
			if(dataItem->version!=version||dataItem->numVertices[polylineIndex]!=numRenderVertices)
				{
				/* Upload the vertices to the vertex buffer: */
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,numRenderVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
				GLintptrARB offset=0;
				size_t numVerticesLeft=numRenderVertices;
				for(const Chunk* chPtr=p.head;numVerticesLeft>0;chPtr=chPtr->succ)
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
				
				dataItem->numVertices[polylineIndex]=numRenderVertices;
				}
			
			/* Render the poly line: */
			glVertexPointer(static_cast<const Vertex*>(0));
			glDrawArrays(GL_LINE_STRIP,0,numRenderVertices);
			}
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		dataItem->version=version;
		}
	else
		{
		for(unsigned int polylineIndex=0;polylineIndex<numPolylines;++polylineIndex)
			{
			const Polyline& p=polylines[polylineIndex];
			
			/* Save the current number of vertices (for parallel creation and rendering): */
			size_t numRenderVertices=p.numVertices;
			
			for(const Chunk* chPtr=p.head;numRenderVertices>0;chPtr=chPtr->succ)
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
		}
	
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}	

}

}
