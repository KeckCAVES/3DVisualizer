/***********************************************************************
Polyhedron - Class to represent convex polyhedra resulting from
intersections of half spaces.
Copyright (c) 2009 Oliver Kreylos

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

#include <Polyhedron.h>

#include <assert.h>
#include <Misc/HashTable.h>
#include <Geometry/Vector.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

/***************************
Methods of class Polyhedron:
***************************/

template <class ScalarParam>
inline
void
Polyhedron<ScalarParam>::check(
	void)
	{
	/* Check each edge for consistency: */
	Card numEdges=edges.size();
	for(Card i=0;i<numEdges;++i)
		{
		/* Check if the opposite pointers are good: */
		assert(edges[edges[i].opposite].opposite==i);
		
		/* Check if the start points are identical: */
		assert(edges[edges[i].next].start==edges[edges[i].opposite].start);
		
		/* Check if the edge is part of a closed face loop: */
		Card faceSize=0;
		for(Card j=edges[i].next;j!=i&&faceSize<1000;j=edges[j].next,++faceSize)
			;
		assert(faceSize<1000);
		}
	}

template <class ScalarParam>
inline
Polyhedron<ScalarParam>::Polyhedron(
	void)
	{
	}

template <class ScalarParam>
inline
Polyhedron<ScalarParam>::Polyhedron(
	const typename Polyhedron<ScalarParam>::Point& min,
	const typename Polyhedron<ScalarParam>::Point& max)
	{
	/* Create the eight box corner points: */
	Point corners[8];
	for(int i=0;i<8;++i)
		for(int j=0;j<3;++j)
			corners[i][j]=(i&(0x1<<j))?max[j]:min[j];
	
	/* Create half-edges for all box faces: */
	edges.push_back(Edge(corners[0], 1,11)); // 0, min[0]
	edges.push_back(Edge(corners[4], 2,23));
	edges.push_back(Edge(corners[6], 3,12));
	edges.push_back(Edge(corners[2], 0,16));
	edges.push_back(Edge(corners[1], 5,18)); // 4, max[0]
	edges.push_back(Edge(corners[3], 6,14));
	edges.push_back(Edge(corners[7], 7,21));
	edges.push_back(Edge(corners[5], 4, 9));
	edges.push_back(Edge(corners[0], 9,19)); // 8, min[1]
	edges.push_back(Edge(corners[1],10, 7));
	edges.push_back(Edge(corners[5],11,20));
	edges.push_back(Edge(corners[4], 8, 0));
	edges.push_back(Edge(corners[2],13, 2)); // 12, max[1]
	edges.push_back(Edge(corners[6],14,22));
	edges.push_back(Edge(corners[7],15, 5));
	edges.push_back(Edge(corners[3],12,17));
	edges.push_back(Edge(corners[0],17, 3)); // 16, min[2]
	edges.push_back(Edge(corners[2],18,15));
	edges.push_back(Edge(corners[3],19, 4));
	edges.push_back(Edge(corners[1],16, 8));
	edges.push_back(Edge(corners[4],21,10)); // 20, max[2]
	edges.push_back(Edge(corners[5],22, 6));
	edges.push_back(Edge(corners[7],23,13));
	edges.push_back(Edge(corners[6],20, 1));
	}

template <class ScalarParam>
inline
Polyhedron<ScalarParam>::Polyhedron(
	const Polyhedron<ScalarParam>& source)
	:edges(source.edges)
	{
	}

template <class ScalarParam>
inline
Polyhedron<ScalarParam>*
Polyhedron<ScalarParam>::clip(
	const typename Polyhedron<ScalarParam>::Plane& plane) const
	{
	typedef Misc::HashTable<Card,void> IndexSet;
	typedef Misc::HashTable<Card,Card> IndexMapper;
	
	/* Find the indices of all half edges that intersect the plane: */
	IndexMapper indexMapper(101);
	Card nextIndex=0;
	Card numIntersectedEdges=0;
	Card intersectedEdgeIndex=0;
	IndexSet ioEdges(17);
	Card numEdges=edges.size();
	for(Card i=0;i<numEdges;++i)
		{
		/* Get the plane distance of start and end points of edge i: */
		Scalar d0=plane.calcDistance(edges[i].start);
		Scalar d1=plane.calcDistance(edges[edges[i].next].start);
		
		/* Classify the edge: */
		if(d0<Scalar(0)&&d1<Scalar(0))
			{
			/* Keep the edge: */
			indexMapper.setEntry(typename IndexMapper::Entry(i,nextIndex));
			++nextIndex;
			}
		else if(d0<Scalar(0)||d1<Scalar(0))
			{
			/* Keep the edge: */
			indexMapper.setEntry(typename IndexMapper::Entry(i,nextIndex));
			++nextIndex;
			
			if(d0<Scalar(0))
				{
				/* Mark the edge as intersected: */
				++numIntersectedEdges;
				intersectedEdgeIndex=i;
				}
			else
				{
				/* Mark the edge as the opposite of an intersected edge: */
				ioEdges.setEntry(typename IndexSet::Entry(i));
				}
			}
		}
	
	/* Remember the number of retained edges: */
	Card newNumEdges=nextIndex;
	
	/* Check for trivial cases: */
	if(newNumEdges==0)
		return new Polyhedron; // Return an empty polyhedron
	else if(numIntersectedEdges==0)
		return new Polyhedron(*this); // Return an identical copy of the polyhedron
	
	/* Sort the list of intersected edges to form a loop: */
	IndexMapper newFaceEdges(17);
	EdgeList newFace;
	while(newFace.size()<numIntersectedEdges)
		{
		Edge newEdge;
		
		/* Calculate the new edge's starting point: */
		const Point& p0=edges[intersectedEdgeIndex].start;
		Scalar d0=plane.calcDistance(p0);
		const Point& p1=edges[edges[intersectedEdgeIndex].next].start;
		Scalar d1=plane.calcDistance(p1);
		newEdge.start=Geometry::affineCombination(p0,p1,(Scalar(0)-d0)/(d1-d0));
		
		/* Find the next intersected edge around the same face as the current last edge: */
		Card edgeIndex;
		for(edgeIndex=edges[intersectedEdgeIndex].next;!ioEdges.isEntry(edgeIndex);edgeIndex=edges[edgeIndex].next)
			;
		newEdge.next=indexMapper.getEntry(edgeIndex).getDest();
		newEdge.opposite=newNumEdges+numIntersectedEdges+newFace.size();
		
		/* Store the index of the new face edge: */
		newFaceEdges.setEntry(IndexMapper::Entry(intersectedEdgeIndex,newFace.size()));
		
		/* Store the new face edge: */
		newFace.push_back(newEdge);
		
		/* Go to the next intersected face: */
		intersectedEdgeIndex=edges[edgeIndex].opposite;
		}
	
	/* Create the result polyhedron: */
	Polyhedron* result=new Polyhedron;
	
	/* Create the edges of the result polyhedron: */
	for(Card i=0;i<numEdges;++i)
		{
		/* Get the plane distance of start and end points of edge i: */
		Scalar d0=plane.calcDistance(edges[i].start);
		Scalar d1=plane.calcDistance(edges[edges[i].next].start);
		
		/* Classify the edge: */
		if(d0<Scalar(0)&&d1<Scalar(0))
			{
			/*****************************************************************
			If an edge is completely inside, its opposite edge must be as
			well, and its next edge must at least be partially inside.
			Therefore, indexMapper will contain the new indices for both
			edges.
			*****************************************************************/
			
			/* Keep the edge: */
			result->edges.push_back(Edge(edges[i].start,indexMapper.getEntry(edges[i].next).getDest(),indexMapper.getEntry(edges[i].opposite).getDest()));
			}
		else if(d0<Scalar(0))
			{
			/*****************************************************************
			If an edge's start point is inside, its opposite edge's index will
			be contained in indexMapper, and its next edge is a new face edge
			whose index can be calculated from newFaceEdges.
			*****************************************************************/
			
			/* Find the index of the next edge from the new face loop: */
			Card next=newNumEdges+newFaceEdges.getEntry(i).getDest();
			
			/* Retain the edge: */
			result->edges.push_back(Edge(edges[i].start,next,indexMapper.getEntry(edges[i].opposite).getDest()));
			}
		else if(d1<Scalar(0))
			{
			/*****************************************************************
			If an edge's end point is inside, its start point must be
			calculated by clipping, and its next edge and opposite edge will
			both be contained in indexMapper.
			*****************************************************************/
			
			/* Calculate the new start point: */
			Point newStart=Geometry::affineCombination(edges[edges[i].next].start,edges[i].start,(Scalar(0)-d1)/(d0-d1));
			
			/* Retain the clipped edge: */
			result->edges.push_back(Edge(newStart,indexMapper.getEntry(edges[i].next).getDest(),indexMapper.getEntry(edges[i].opposite).getDest()));
			}
		}
	
	/* Add the closing edges for all clipped faces to the polyhedron: */
	for(typename EdgeList::const_iterator nfIt=newFace.begin();nfIt!=newFace.end();++nfIt)
		result->edges.push_back(*nfIt);
	
	/* Add the edges of the closing face to the polyhedron: */
	for(Card i=0;i<newFace.size();++i)
		{
		Edge newEdge;
		newEdge.start=newFace[(i+1)%newFace.size()].start;
		newEdge.next=newNumEdges+numIntersectedEdges+(i+newFace.size()-1)%newFace.size();
		newEdge.opposite=newNumEdges+i;
		result->edges.push_back(newEdge);
		}
	
	// DEBUGGING
	// result->check();
	
	return result;
	}

template <class ScalarParam>
inline
void
Polyhedron<ScalarParam>::drawEdges(
	void) const
	{
	glBegin(GL_LINES);
	Card numEdges=edges.size();
	for(Card i=0;i<numEdges;++i)
		{
		/* Only draw the "minor" half of each half edge pair: */
		if(i<edges[i].opposite)
			{
			glVertex(edges[i].start);
			glVertex(edges[edges[i].next].start);
			}
		}
	glEnd();
	}

template <class ScalarParam>
inline
void
Polyhedron<ScalarParam>::drawFaces(
	void) const
	{
	typedef Misc::HashTable<Card,void> IndexSet;
	typedef Geometry::Vector<Scalar,3> Vector;
	
	IndexSet visitedEdges(101);
	Card numEdges=edges.size();
	for(Card i=0;i<numEdges;++i)
		{
		/* Only start a face if the current edge has not been visited yet: */
		if(!visitedEdges.isEntry(i))
			{
			/* Draw a polygon for the face: */
			glBegin(GL_POLYGON);
			
			/* Calculate the face's normal vector: */
			Card i1=edges[i].next;
			Vector d0=edges[i1].start-edges[i].start;
			Card i2=edges[i1].next;
			Vector d1=edges[i2].start-edges[i1].start;
			Vector normal=Geometry::cross(d0,d1);
			normal.normalize();
			glNormal(normal);
			
			/* Traverse all edges of the face: */
			Card j=i;
			do
				{
				glVertex(edges[j].start);
				visitedEdges.setEntry(IndexSet::Entry(j));
				j=edges[j].next;
				}
			while(j!=i);
			
			glEnd();
			}
		}
	}

template <class ScalarParam>
inline
void
Polyhedron<ScalarParam>::drawIntersection(const typename Polyhedron<ScalarParam>::Plane& plane) const
	{
	typedef Misc::HashTable<Card,void> IndexSet;
	
	/* Find any edge that intersects the given plane: */
	Card numEdges=edges.size();
	Card intersectedEdgeIndex=numEdges;
	IndexSet ioEdges(17);
	for(Card i=0;i<numEdges;++i)
		{
		/* Get the plane distance of start and end points of edge i: */
		Scalar d0=plane.calcDistance(edges[i].start);
		Scalar d1=plane.calcDistance(edges[edges[i].next].start);
		
		/* Classify the edge: */
		if(d0<Scalar(0)&&d1>=Scalar(0))
			{
			/* Remember the edge: */
			intersectedEdgeIndex=i;
			}
		else if(d0>=Scalar(0)&&d1<Scalar(0))
			{
			/* Mark the edge as a face exit: */
			ioEdges.setEntry(IndexSet::Entry(i));
			}
		}
	
	/* Bail out if no intersection was found: */
	if(intersectedEdgeIndex==numEdges)
		return;
	
	/* Iterate around the polyhedron along the plane intersection: */
	glBegin(GL_POLYGON);
	glNormal(-plane.getNormal());
	Card edgeIndex=intersectedEdgeIndex;
	do
		{
		/* Calculate and draw the edge's intersection point: */
		const Point& p0=edges[edgeIndex].start;
		Scalar d0=plane.calcDistance(p0);
		const Point& p1=edges[edges[edgeIndex].next].start;
		Scalar d1=plane.calcDistance(p1);
		glVertex(Geometry::affineCombination(p0,p1,(Scalar(0)-d0)/(d1-d0)));
		
		/* Find the next intersected edge around the same face as the current last edge: */
		Card i;
		for(i=edges[edgeIndex].next;!ioEdges.isEntry(i);i=edges[i].next)
			;
		
		/* Go to the next intersected face: */
		edgeIndex=edges[i].opposite;
		}
	while(edgeIndex!=intersectedEdgeIndex);
	glEnd();
	}

/*****************************************************
Force instantiation of all default Polyhedron classes:
*****************************************************/

template class Polyhedron<float>;
template class Polyhedron<double>;
