/***********************************************************************
TriangleSet - Class to represent surfaces as sets of unconnected
triangles.
Copyright (c) 2005-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_TRIANGLESET_INCLUDED
#define VISUALIZATION_TEMPLATIZED_TRIANGLESET_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}

namespace Visualization {

namespace Templatized {

template <class VertexParam>
class TriangleSet:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef VertexParam Vertex; // Type for triangle vertices
	
	private:
	static const size_t chunkSize=3333; // Number of triangles per chunk
	
	struct Chunk // Structure for triangle buffer chunks
		{
		/* Elements: */
		public:
		Chunk* succ; // Pointer to next triangle buffer chunk
		Vertex vertices[chunkSize*3]; // Array of triangle vertices
		
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
		unsigned int version; // Version number of the triangle set in the vertex buffer
		size_t numTriangles; // Number of triangles already uploaded to the vertex buffer
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	Cluster::MulticastPipe* pipe; // Pipe to stream triangle set data in a cluster environment (owned by caller)
	unsigned int version; // Version number of the triangle set (incremented on each clear operation)
	size_t numTriangles; // Total number of triangles currently in set
	Chunk* head; // Pointer to first triangle buffer chunk
	Chunk* tail; // Pointer to last triangle buffer chunk
	size_t tailNumSentTriangles; // Number of triangles in last buffer chunk that were already sent across the pipe
	size_t tailRoomLeft; // Number of triangles left in last buffer chunk
	Vertex* nextVertex; // Pointer to next vertex to be stored
	
	/* Private methods: */
	void addNewChunk(void); // Adds a new chunk to the triangle buffer
	
	/* Constructors and destructors: */
	public:
	TriangleSet(Cluster::MulticastPipe* sPipe); // Creates empty triangle set for given multicast pipe (or 0 in single-machine environment)
	private:
	TriangleSet(const TriangleSet& source); // Prohibit copy constructor
	TriangleSet& operator=(const TriangleSet& source); // Prohibit assignment operator
	public:
	virtual ~TriangleSet(void); // Destroys triangle set
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	void clear(void); // Removes all triangles from the set
	Vertex* getNextTriangleVertices(void) // Returns pointer to next vertex triple in buffer
		{
		/* Check if there is room to add another triangle: */
		if(tailRoomLeft==0)
			addNewChunk();
		
		/* Return pointer to the next vertex: */
		return nextVertex;
		}
	void addTriangle(void) // Just advances the triangle counter; assumes caller wrote data into buffer
		{
		/* Increment the triangle count: */
		++numTriangles;
		--tailRoomLeft;
		nextVertex+=3;
		}
	void receive(void); // Receives triangle set data via multicast pipe until next flush() point
	void flush(void); // Sends pending triangle set data across the multicast pipe and terminates receive() method on slaves
	size_t getNumTriangles(void) const // Returns number of triangles currently in buffer
		{
		return numTriangles;
		}
	void glRenderAction(GLContextData& contextData) const; // Renders all triangles in the buffer
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_TRIANGLESET_IMPLEMENTATION
#include <Templatized/TriangleSet.icpp>
#endif

#endif
