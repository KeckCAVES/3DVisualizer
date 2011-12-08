/***********************************************************************
MultiPolyline - Class to represent multiple arbitrary-length polylines.
Copyright (c) 2007-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_MULTIPOLYLINE_INCLUDED
#define VISUALIZATION_TEMPLATIZED_MULTIPOLYLINE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}

namespace Visualization {

namespace Templatized {

template <class VertexParam>
class MultiPolyline:public GLObject
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
	
	struct Polyline // Structure holding state of an individual polyline
		{
		/* Elements: */
		public:
		size_t numVertices; // Number of vertices currently in the polyline
		Chunk* head; // Pointer to first vertex chunk used by polyline
		Chunk* tail; // Pointer to last vertex chunk used by polyline
		size_t tailNumSentVertices; // Number of vertices in last buffer chunk that were already sent across the pipe
		size_t tailRoomLeft; // Number of vertices still available in the tail chunk
		Vertex* nextVertex; // Pointer to next available vertex in polyline

		/* Constructors and destructors: */
		Polyline(void)
			:numVertices(0),
			 head(0),tail(0),
			 tailNumSentVertices(0),
			 tailRoomLeft(0),
			 nextVertex(0)
			{
			}
		~Polyline(void);
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		unsigned int numPolylines; // Number of individual polylines
		GLuint* vertexBufferIds; // Array of IDs of vertex buffer objects for point data (or 0 if extension is not supported)
		unsigned int version; // Version number of the polyline in the vertex buffer
		size_t* numVertices; // Array of numbers of vertices already uploaded to the vertex buffers
		
		/* Constructors and destructors: */
		DataItem(unsigned int sNumPolylines);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	unsigned int numPolylines; // Number of individual polylines
	Cluster::MulticastPipe* pipe; // Pipe to stream polyline data in a cluster environment (owned by caller)
	unsigned int version; // Version number of the multipolyline (incremented on each clear operation)
	Polyline* polylines; // Array of individual polylines
	size_t maxNumVertices; // Maximum number of vertices in any individual polyline
	
	/* Private methods: */
	void addNewChunk(unsigned int polylineIndex); // Adds a new chunk to the vertex buffer of the given polyline
	
	/* Constructors and destructors: */
	public:
	MultiPolyline(unsigned int sNumPolylines,Cluster::MulticastPipe* sPipe); // Creates empty multi-polyline for given multicast pipe (or 0 in single-machine environment)
	private:
	MultiPolyline(const MultiPolyline& source); // Prohibit copy constructor
	MultiPolyline& operator=(const MultiPolyline& source); // Prohibit assignment operator
	public:
	virtual ~MultiPolyline(void); // Destroys multi-polyline
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	void clear(void); // Removes all vertices from the polyline
	Vertex* getNextVertex(unsigned int polylineIndex) // Returns pointer to next vertex in buffer for the given polyline
		{
		/* Check if there is room to add another vertex: */
		if(polylines[polylineIndex].tailRoomLeft==0)
			addNewChunk(polylineIndex);
		
		/* Return pointer to the next vertex: */
		return polylines[polylineIndex].nextVertex;
		}
	void addVertex(unsigned int polylineIndex) // Just advances the vertex counter for the given polyline; assumes caller wrote data into buffer
		{
		/* Increment the vertex count: */
		++polylines[polylineIndex].numVertices;
		if(maxNumVertices<polylines[polylineIndex].numVertices)
			maxNumVertices=polylines[polylineIndex].numVertices;
		--polylines[polylineIndex].tailRoomLeft;
		++polylines[polylineIndex].nextVertex;
		}
	void receive(void); // Receives multi-polyline data via multicast pipe until next flush() point
	void flush(void); // Sends pending multi-polyline data across the multicast pipe and terminates receive() method on slaves
	unsigned int getNumPolylines(void) const // Returns the number of individual polylines
		{
		return numPolylines;
		}
	size_t getNumVertices(unsigned int polylineIndex) const // Returns number of vertices currently in buffer of the given polyline
		{
		return polylines[polylineIndex].numVertices;
		}
	size_t getMaxNumVertices(void) const // Returns the maximum number of vertices in any polyline
		{
		return maxNumVertices;
		}
	void glRenderAction(GLContextData& contextData) const; // Renders the polyline
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_MULTIPOLYLINE_IMPLEMENTATION
#include <Templatized/MultiPolyline.icpp>
#endif

#endif
