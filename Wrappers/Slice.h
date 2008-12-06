/***********************************************************************
Slice - Wrapper class for slices as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_SLICE_INCLUDED
#define VISUALIZATION_WRAPPERS_SLICE_INCLUDED

#include <Geometry/Plane.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLMaterial.h>

#include <Abstract/Element.h>
#include <Templatized/IndexedTriangleSet.h>

/* Forward declarations: */
class GLColorMap;

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class Slice:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Type for points in the data set's domain
	typedef Geometry::Plane<Scalar,DS::dimension> Plane; // Type for planes in the data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<VScalar,1,void,0,void,Scalar,dimension> Vertex; // Data type for triangle vertices
	typedef Visualization::Templatized::IndexedTriangleSet<Vertex> Surface; // Data structure to represent surfaces
	
	struct Parameters // Extraction parameters defining a slice
		{
		/* Elements: */
		public:
		int scalarVariableIndex; // Index of the scalar variable to color the slice
		size_t maxNumTriangles; // Maximum number of triangles to be extracted
		Point seedPoint; // The slice's seeding point
		Plane plane; // The slice's plane equation
		
		/* Constructors and destructors: */
		Parameters(int sScalarVariableIndex,size_t sMaxNumTriangles) // Initializes permanent parameters
			:scalarVariableIndex(sScalarVariableIndex),
			 maxNumTriangles(sMaxNumTriangles)
			{
			}
		
		/* Methods: */
		template <class DataSinkParam>
		void write(DataSinkParam& dataSink) const // Writes the parameters to a data sink (such as a file or a pipe)
			{
			dataSink.write<int>(scalarVariableIndex);
			dataSink.write<unsigned int>(maxNumTriangles);
			dataSink.write<Scalar>(seedPoint.getComponents(),dimension);
			dataSink.write<Scalar>(plane.getNormal().getComponents(),dimension);
			dataSink.write<Scalar>(plane.getOffset());
			}
		template <class DataSourceParam>
		Parameters& read(DataSourceParam& dataSource) // Reads the parameters from a data sink (such as a file or a pipe)
			{
			scalarVariableIndex=dataSource.read<int>();
			maxNumTriangles=dataSource.read<unsigned int>();
			dataSource.read<Scalar>(seedPoint.getComponents(),dimension);
			typename Plane::Vector normal;
			dataSource.read<Scalar>(normal.getComponents(),dimension);
			Scalar offset=dataSource.read<Scalar>();
			plane=Plane(normal,offset);
			return *this;
			}
		};
	
	/* Elements: */
	private:
	Parameters parameters; // Slice's extraction parameters
	const GLColorMap* colorMap; // Color map for slice vertex values
	Surface surface; // Representation of the slice
	
	/* Constructors and destructors: */
	public:
	Slice(const Parameters& sParameters,const GLColorMap* sColorMap,Comm::MulticastPipe* pipe); // Creates an empty slice for the given parameters
	private:
	Slice(const Slice& source); // Prohibit copy constructor
	Slice& operator=(const Slice& source); // Prohibit assignment operator
	public:
	virtual ~Slice(void);
	
	/* Methods: */
	const Parameters& getParameters(void) const // Returns the slice's extraction parameters
		{
		return parameters;
		}
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	Surface& getSurface(void) // Returns the surface representation
		{
		return surface;
		}
	size_t getElementSize(void) const // Returns the number of triangles in the surface representation
		{
		return surface.getNumTriangles();
		}
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void saveParameters(Misc::File& parameterFile) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_SLICE_IMPLEMENTATION
#include <Wrappers/Slice.cpp>
#endif

#endif
