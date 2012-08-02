/***********************************************************************
MultiStreamline - Wrapper class for multiple related streamlines as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2006-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_MULTISTREAMLINE_INCLUDED
#define VISUALIZATION_WRAPPERS_MULTISTREAMLINE_INCLUDED

#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#include <Templatized/MultiPolyline.h>

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class MultiStreamline:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<VScalar,1,void,0,Scalar,Scalar,dimension> Vertex; // Data type for streamline vertices
	typedef Visualization::Templatized::MultiPolyline<Vertex> MultiPolyline; // Data structure to represent multi-streamlines
	
	/* Elements: */
	private:
	int scalarVariableIndex; // Index of the scalar variable used to color the streamline
	MultiPolyline multiPolyline; // Multi-streamline representations
	
	/* Constructors and destructors: */
	public:
	MultiStreamline(Visualization::Abstract::VariableManager* sVariableManager,Visualization::Abstract::Parameters* sParameters,int sScalarVariableIndex,unsigned int numStreamlines,Cluster::MulticastPipe* pipe); // Creates an empty multi-streamline for the given parameters
	private:
	MultiStreamline(const MultiStreamline& source); // Prohibit copy constructor
	MultiStreamline& operator=(const MultiStreamline& source); // Prohibit assignment operator
	public:
	virtual ~MultiStreamline(void);
	
	/* Methods from Visualization::Abstract::Element: */
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* New methods: */
	MultiPolyline& getMultiPolyline(void) // Returns the multi-streamline representation
		{
		return multiPolyline;
		}
	size_t getElementSize(void) const // Returns the number of vertices in the longest streamline
		{
		/* Return the maximum number of vertices in any polyline: */
		return multiPolyline.getMaxNumVertices();
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_MULTISTREAMLINE_IMPLEMENTATION
#include <Wrappers/MultiStreamline.icpp>
#endif

#endif
