/***********************************************************************
ColoredIsosurface - Wrapper class for color-mapped isosurfaces as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2008 Oliver Kreylos

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

#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#if 0
#include <Templatized/IndexedTriangleSet.h>
#else
#include <Templatized/TriangleSet.h>
#endif

/* Forward declarations: */
class GLColorMap;

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
	typedef typename DS::Point Point; // Type for points in data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<VScalar,1,void,0,Scalar,Scalar,dimension> Vertex; // Data type for triangle vertices
	#if 0
	typedef Visualization::Templatized::IndexedTriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#else
	typedef Visualization::Templatized::TriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#endif
	
	struct Parameters // Extraction parameters defining an isosurface
		{
		/* Elements: */
		public:
		int scalarVariableIndex; // Index of the scalar variable defining the isosurface
		int colorScalarVariableIndex; // Index of the scalar variable to color the isosurface
		size_t maxNumTriangles; // Maximum number of triangles to be extracted
		bool smoothShaded; // Flag whether the isosurface is smooth shaded
		bool lightSurface; // Flag whether to use a lighting when rendering the isosurface
		Point seedPoint; // The isosurface's seeding point
		VScalar isovalue; // The isosurface's isovalue
		
		/* Constructors and destructors: */
		Parameters(int sScalarVariableIndex,int sColorScalarVariableIndex,size_t sMaxNumTriangles,bool sSmoothShaded,bool sLightSurface) // Initializes permanent parameters
			:scalarVariableIndex(sScalarVariableIndex),
			 colorScalarVariableIndex(sColorScalarVariableIndex),
			 maxNumTriangles(sMaxNumTriangles),
			 smoothShaded(sSmoothShaded),
			 lightSurface(sLightSurface)
			{
			}
		
		/* Methods: */
		template <class DataSinkParam>
		void write(DataSinkParam& dataSink) const // Writes the parameters to a data sink (such as a file or a pipe)
			{
			dataSink.write<int>(scalarVariableIndex);
			dataSink.write<int>(colorScalarVariableIndex);
			dataSink.write<unsigned int>(maxNumTriangles);
			dataSink.write<int>(smoothShaded?1:0);
			dataSink.write<int>(lightSurface?1:0);
			dataSink.write<Scalar>(seedPoint.getComponents(),dimension);
			dataSink.write<VScalar>(isovalue);
			}
		template <class DataSourceParam>
		Parameters& read(DataSourceParam& dataSource) // Reads the parameters from a data sink (such as a file or a pipe)
			{
			scalarVariableIndex=dataSource.read<int>();
			colorScalarVariableIndex=dataSource.read<int>();
			maxNumTriangles=dataSource.read<unsigned int>();
			smoothShaded=dataSource.read<int>()!=0;
			lightSurface=dataSource.read<int>()!=0;
			dataSource.read<Scalar>(seedPoint.getComponents(),dimension);
			isovalue=dataSource.read<VScalar>();
			return *this;
			}
		};
	
	/* Elements: */
	private:
	Parameters parameters; // Colored isosurface's extraction parameters
	const GLColorMap* colorMap; // Color map for isosurface vertex values
	Surface surface; // Representation of the isosurface
	
	/* Constructors and destructors: */
	public:
	ColoredIsosurface(const Parameters& sParameters,const GLColorMap* sColorMap,Comm::MulticastPipe* pipe); // Creates an empty colored isosurface for the given parameters
	private:
	ColoredIsosurface(const ColoredIsosurface& source); // Prohibit copy constructor
	ColoredIsosurface& operator=(const ColoredIsosurface& source); // Prohibit assignment operator
	public:
	virtual ~ColoredIsosurface(void);
	
	/* Methods: */
	const Parameters& getParameters(void) const // Returns the colored isosurface's extraction parameters
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

#ifndef VISUALIZATION_WRAPPERS_COLOREDISOSURFACE_IMPLEMENTATION
#include <Wrappers/ColoredIsosurface.cpp>
#endif

#endif
