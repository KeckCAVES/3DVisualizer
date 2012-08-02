/***********************************************************************
ColoredIsosurface - Wrapper class for color-mapped isosurfaces as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2008-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_COLOREDISOSURFACE_INCLUDED
#define VISUALIZATION_WRAPPERS_COLOREDISOSURFACE_INCLUDED

#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#if 0
#include <Templatized/IndexedTriangleSet.h>
#else
#include <Templatized/TriangleSet.h>
#endif

/* Forward declarations: */
#ifdef VISUALIZATION_USE_SHADERS
class TwoSided1DTexturedSurfaceShader;
#endif

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class ColoredIsosurface:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<VScalar,1,void,0,Scalar,Scalar,dimension> Vertex; // Data type for triangle vertices
	#if 0
	typedef Visualization::Templatized::IndexedTriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#else
	typedef Visualization::Templatized::TriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#endif
	
	/* Elements: */
	private:
	int scalarVariableIndex; // Index of the scalar variable used to color the colored isosurface
	bool lighting; // Flag if the colored isosurface is lit
	#ifdef VISUALIZATION_USE_SHADERS
	TwoSided1DTexturedSurfaceShader* shader; // Shader for the isosurface
	#endif
	Surface surface; // Representation of the colored isosurface
	
	/* Constructors and destructors: */
	public:
	ColoredIsosurface(Visualization::Abstract::VariableManager* sVariableManager,Visualization::Abstract::Parameters* sParameters,int sScalarVariableIndex,bool sLighting,Cluster::MulticastPipe* pipe); // Creates an empty colored isosurface for the given parameters
	private:
	ColoredIsosurface(const ColoredIsosurface& source); // Prohibit copy constructor
	ColoredIsosurface& operator=(const ColoredIsosurface& source); // Prohibit assignment operator
	public:
	virtual ~ColoredIsosurface(void);
	
	/* Methods from Visualization::Abstract::Element: */
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* New methods: */
	Surface& getSurface(void) // Returns the surface representation
		{
		return surface;
		}
	size_t getElementSize(void) const // Returns the number of triangles in the surface representation
		{
		return surface.getNumTriangles();
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_COLOREDISOSURFACE_IMPLEMENTATION
#include <Wrappers/ColoredIsosurface.icpp>
#endif

#endif
