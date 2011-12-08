/***********************************************************************
Polyline - Class to represent arbitrary-length polylines.
Copyright (c) 2006-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_POLYLINE_INCLUDED
#define VISUALIZATION_TEMPLATIZED_POLYLINE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}

namespace Visualization {

namespace Templatized {

template <class VertexParam>
class Polyline:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef VertexParam Vertex; // Type for polyline vertices
	
	private:
	static const size_t chunkSize=5000; // Number of vertices per chunk
	
	struct Chunk // Structure for vertex buffer chunks
		{
		/* Elements: */
		public:
		Chunk* succ; // Pointer to next vertex buffer chunk
		Vertex vertices[chunkSize]; // Array of polyline vertices
		
		/* Constructors and destructors: */
		Chunk(void)
			:succ(0)
			{
			}
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of vertex buffer object for point data (or 0 if extension is not supported)
		unsigned int version; // Version number of the polyline in the vertex buffer
		size_t numVertices; // Number of vertices already uploaded to the vertex buffer
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	Cluster::MulticastPipe* pipe; // Pipe to stream polyline data in a cluster environment (owned by caller)
	unsigned int version; // Version number of the polyline (incremented on each clear operation)
	size_t numVertices; // Total number of vertices currently in set
	Chunk* head; // Pointer to first vertex buffer chunk
	Chunk* tail; // Pointer to last vertex buffer chunk
	size_t tailNumSentVertices; // Number of vertices in last buffer chunk that were already sent across the pipe
	size_t tailRoomLeft; // Number of vertices left in last buffer chunk
	Vertex* nextVertex; // Pointer to next vertex to be stored
	
	/* Private methods: */
	void addNewChunk(void); // Adds a new chunk to the vertex buffer
	
	/* Constructors and destructors: */
	public:
	Polyline(Cluster::MulticastPipe* sPipe); // Creates empty polyline for given multicast pipe (or 0 in single-machine environment)
	private:
	Polyline(const Polyline& source); // Prohibit copy constructor
	Polyline& operator=(const Polyline& source); // Prohibit assignment operator
	public:
	virtual ~Polyline(void); // Destroys polyline
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	void clear(void); // Removes all vertices from the polyline
	Vertex* getNextVertex(void) // Returns pointer to next vertex in buffer
		{
		/* Check if there is room to add another vertex: */
		if(tailRoomLeft==0)
			addNewChunk();
		
		/* Return pointer to the next vertex: */
		return nextVertex;
		}
	void addVertex(void) // Just advances the vertex counter; assumes caller wrote data into buffer
		{
		/* Increment the vertex count: */
		++numVertices;
		--tailRoomLeft;
		++nextVertex;
		}
	void receive(void); // Receives polyline data via multicast pipe until next flush() point
	void flush(void); // Sends pending polyline data across the multicast pipe and terminates receive() method on slaves
	size_t getNumVertices(void) const // Returns number of vertices currently in buffer
		{
		return numVertices;
		}
	void glRenderAction(GLContextData& contextData) const; // Renders the polyline
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_POLYLINE_IMPLEMENTATION
#include <Templatized/Polyline.icpp>
#endif

#endif
