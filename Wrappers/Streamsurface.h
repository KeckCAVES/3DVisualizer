/***********************************************************************
Streamsurface - Wrapper class for surfaces spanned by multiple
streamlines as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_STREAMSURFACE_INCLUDED
#define VISUALIZATION_WRAPPERS_STREAMSURFACE_INCLUDED

#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#include <Templatized/IndexedTrianglestripSet.h>

/* Forward declarations: */
class GLColorMap;

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class Streamsurface:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<Scalar,1,void,0,Scalar,Scalar,dimension> Vertex; // Data type for stream surface vertices
	typedef Visualization::Templatized::IndexedTrianglestripSet<Vertex> Surface; // Data structure to represent stream surfaces
	
	/* Elements: */
	private:
	const GLColorMap* colorMap; // Color map for auxiliary streamline vertex values
	Surface surface; // Stream surface representation
	
	/* Constructors and destructors: */
	public:
	Streamsurface(const GLColorMap* sColorMap); // Creates an empty stream surface for the given color map
	private:
	Streamsurface(const Streamsurface& source); // Prohibit copy constructor
	Streamsurface& operator=(const Streamsurface& source); // Prohibit assignment operator
	public:
	virtual ~Streamsurface(void);
	
	/* Methods: */
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	Surface& getSurface(void) // Returns the stream surface representations
		{
		return surface;
		}
	size_t getElementSize(void) const // Returns the number of vertex layers in the stream surface
		{
		return surface.getNumStrips()+1;
		}
	virtual std::string getName(void) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_STREAMSURFACE_IMPLEMENTATION
#include <Wrappers/Streamsurface.icpp>
#endif

#endif
