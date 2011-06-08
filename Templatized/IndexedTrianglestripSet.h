/***********************************************************************
IndexedTrianglestripSet - Class to represent surfaces as sets of
triangle strips sharing vertices.
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

#ifndef VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESTRIPSET_INCLUDED
#define VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESTRIPSET_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

namespace Visualization {

namespace Templatized {

template <class VertexParam>
class IndexedTrianglestripSet:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef VertexParam Vertex; // Type for triangle strip vertices
	typedef GLuint Index; // Type for vertex indices
	
	private:
	static const size_t vertexChunkSize=10000; // Number of vertices per vertex chunk
	static const size_t indexChunkSize=10000; // Number of vertex indices per index chunk
	static const size_t stripChunkSize=1000; // Number of triangle strip lengths per strip chunk
	
	struct VertexChunk // Structure for vertex buffer chunks
		{
		/* Elements: */
		public:
		VertexChunk* succ; // Pointer to next vertex buffer chunk
		Vertex vertices[vertexChunkSize]; // Array of vertices
		
		/* Constructors and destructors: */
		VertexChunk(void)
			:succ(0)
			{
			}
		};
	
	struct IndexChunk // Structure for index buffer chunks
		{
		/* Elements: */
		public:
		IndexChunk* succ; // Pointer to next index buffer chunk
		Index indices[indexChunkSize]; // Array of vertex indices
		
		/* Constructors and destructors: */
		IndexChunk(void)
			:succ(0)
			{
			}
		};
	
	struct StripChunk // Structure for strip length buffer chunks
		{
		/* Elements: */
		public:
		StripChunk* succ; // Pointer to next strip length buffer chunk
		GLsizei lengths[stripChunkSize]; // Array of strip lengths
		
		/* Constructors and destructors: */
		StripChunk(void)
			:succ(0)
			{
			}
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of buffer object for vertex data
		GLuint indexBufferId; // ID of buffer object for index data
		unsigned int version; // Version number of the triangle strip set in the buffer objects
		size_t numVertices; // Number of vertices in the vertex buffer
		size_t numIndices; // Number of vertex indices in the index buffer
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	unsigned int version; // Version number of the triangle strip set (incremented on each clear operation)
	size_t numVertices; // Number of vertices in the set
	size_t numIndices; // Number of vertex indices in the set
	size_t numStrips; // Number of triangle strips in the set
	VertexChunk* vertexHead; // Pointer to first vertex buffer chunk
	VertexChunk* vertexTail; // Pointer to last vertex buffer chunk
	IndexChunk* indexHead; // Pointer to first index buffer chunk
	IndexChunk* indexTail; // Pointer to last index buffer chunk
	StripChunk* stripHead; // Pointer to first strip buffer chunk
	StripChunk* stripTail; // Pointer to last strip buffer chunk
	size_t numVerticesLeft; // Number of vertices left in last vertex buffer chunk
	size_t numIndicesLeft; // Number of vertex indices left in last index buffer chunk
	size_t numStripsLeft; // Number of strips left in last strip buffer chunk
	Vertex* nextVertex; // Pointer to next vertex to be stored
	Index* nextIndex; // Pointer to next vertex index to be stored
	GLsizei currentStripLength; // Number of vertices in current triangle strip
	GLsizei* nextStrip; // Pointer to next strip length to be stored
	
	/* Private methods: */
	void addNewVertexChunk(void); // Adds a new chunk to the vertex buffer
	void addNewIndexChunk(void); // Adds a new chunk to the index buffer
	void addNewStripChunk(void); // Adds a new chunk to the strip buffer
	
	/* Constructors and destructors: */
	public:
	IndexedTrianglestripSet(void); // Creates empty triangle strip set
	private:
	IndexedTrianglestripSet(const IndexedTrianglestripSet& source); // Prohibit copy constructor
	IndexedTrianglestripSet& operator=(const IndexedTrianglestripSet& source); // Prohibit assignment operator
	public:
	virtual ~IndexedTrianglestripSet(void); // Destroys triangle strip set
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	void clear(void); // Removes all triangle strips from the set
	Vertex* getNextVertex(void) // Returns pointer to next vertex in buffer
		{
		/* Check if there is room in the last vertex buffer chunk to add another vertex: */
		if(numVerticesLeft==0)
			addNewVertexChunk();
		
		/* Return pointer to the next vertex: */
		return nextVertex;
		}
	Index addVertex(void) // Just advances the vertex counter and returns the most recent index; assumes caller wrote data into buffer
		{
		/* Increment the vertex count: */
		++nextVertex;
		++numVertices;
		--numVerticesLeft;
		
		/* Return the last vertex' index: */
		return Index(numVertices-1);
		}
	Index* getNextIndex(void) // Returns pointer to next vertex index in buffer
		{
		/* Check if there is room in the last index buffer chunk to add another index: */
		if(numIndicesLeft==0)
			addNewIndexChunk();
		
		/* Return pointer to next index: */
		return nextIndex;
		}
	void addIndex(void) // Just advances the index counter; assumes caller wrote data into buffer
		{
		/* Increment the index count: */
		++nextIndex;
		++numIndices;
		--numIndicesLeft;
		
		/* Increment the number of vertices in the current strip: */
		++currentStripLength;
		}
	void addIndex(Index newIndex) // Adds a new vertex index to the current triangle strip
		{
		/* Check if there is room in the last index buffer chunk to add another index: */
		if(numIndicesLeft==0)
			addNewIndexChunk();
		
		/* Store the index and increment the index count: */
		*nextIndex=newIndex;
		++nextIndex;
		++numIndices;
		--numIndicesLeft;
		
		/* Increment the number of vertices in the current strip: */
		++currentStripLength;
		}
	void addStrip(void) // Finishes the current triangle strip
		{
		/* Check if there is room in the last strip buffer chunk to add another strip: */
		if(numStripsLeft==0)
			addNewStripChunk();
		
		/* Store the length of the current strip: */
		*nextStrip=currentStripLength;
		++nextStrip;
		++numStrips;
		--numStripsLeft;
		currentStripLength=0;
		}
	size_t getNumVertices(void) const // Returns number of vertices currently in buffer
		{
		return numVertices;
		}
	size_t getNumIndices(void) const // Returns number of vertex indices currently in buffer
		{
		return numIndices;
		}
	size_t getNumStrips(void) const // Returns number of triangle strips currently in buffer
		{
		return numStrips;
		}
	void glRenderAction(GLContextData& contextData) const; // Renders all triangle strips in the buffer
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESTRIPSET_IMPLEMENTATION
#include <Templatized/IndexedTrianglestripSet.icpp>
#endif

#endif
