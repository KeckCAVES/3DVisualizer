/***********************************************************************
MultiStreamline - Wrapper class for multiple related streamlines as
visualization elements.
Part of the wrapper layer of the templatized visualization
components.
Copyright (c) 2006-2008 Oliver Kreylos

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

#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#include <Templatized/MultiPolyline.h>

/* Forward declarations: */
class GLColorMap;

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
	typedef typename DS::Point Point; // Type for points in data set's domain
	typedef typename DS::Vector Vector; // Type for vectors in data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<Scalar,1,void,0,Scalar,Scalar,dimension> Vertex; // Data type for streamline vertices
	typedef Visualization::Templatized::MultiPolyline<Vertex> MultiPolyline; // Data structure to represent multi-streamlines
	
	struct Parameters
		{
		/* Elements: */
		public:
		int vectorVariableIndex; // Index of the vector variable defining the arrow rake
		int colorScalarVariableIndex; // Index of the scalar variable used to color the arrows
		size_t maxNumVertices; // Maximum number of vertices to be extracted
		Scalar epsilon; // Per-step accuracy threshold for streamline integration
		unsigned int numStreamlines; // Number of individual streamlines in multi-streamline
		Scalar diskRadius; // Radius of disk of streamline seed positions around original query position
		Point base; // The multi-streamline's original query position
		Vector frame[2]; // Frame vectors of the multi-streamline's seeding disk
		
		/* Constructors and destructors: */
		Parameters(int sVectorVariableIndex,int sColorScalarVariableIndex,size_t sMaxNumVertices,Scalar sEpsilon,unsigned int sNumStreamlines,Scalar sDiskRadius) // Initializes permanent parameters
			:vectorVariableIndex(sVectorVariableIndex),
			 colorScalarVariableIndex(sColorScalarVariableIndex),
			 maxNumVertices(sMaxNumVertices),
			 epsilon(sEpsilon),
			 numStreamlines(sNumStreamlines),
			 diskRadius(sDiskRadius)
			{
			}
		
		/* Methods: */
		template <class DataSinkParam>
		void write(DataSinkParam& dataSink) const // Writes the parameters to a data sink (such as a file or a pipe)
			{
			dataSink.write<int>(vectorVariableIndex);
			dataSink.write<int>(colorScalarVariableIndex);
			dataSink.write<unsigned int>(maxNumVertices);
			dataSink.write<Scalar>(epsilon);
			dataSink.write<unsigned int>(numStreamlines);
			dataSink.write<Scalar>(diskRadius);
			dataSink.write<Scalar>(base.getComponents(),dimension);
			for(int i=0;i<2;++i)
				dataSink.write<Scalar>(frame[i].getComponents(),dimension);
			}
		template <class DataSourceParam>
		Parameters& read(DataSourceParam& dataSource) // Reads the parameters from a data sink (such as a file or a pipe)
			{
			vectorVariableIndex=dataSource.read<int>();
			colorScalarVariableIndex=dataSource.read<int>();
			maxNumVertices=dataSource.read<unsigned int>();
			epsilon=dataSource.read<Scalar>();
			numStreamlines=dataSource.read<unsigned int>();
			diskRadius=dataSource.read<Scalar>();
			dataSource.read<Scalar>(base.getComponents(),dimension);
			for(int i=0;i<2;++i)
				dataSource.read<Scalar>(frame[i].getComponents(),dimension);
			return *this;
			}
		};
	
	/* Elements: */
	private:
	Parameters parameters; // Streamline's extraction parameters
	const GLColorMap* colorMap; // Color map for auxiliary streamline vertex values
	MultiPolyline multiPolyline; // Multi-streamline representations
	
	/* Constructors and destructors: */
	public:
	MultiStreamline(const Parameters& sParameters,const GLColorMap* sColorMap,Comm::MulticastPipe* pipe); // Creates an empty multi-streamline for the given parameters
	private:
	MultiStreamline(const MultiStreamline& source); // Prohibit copy constructor
	MultiStreamline& operator=(const MultiStreamline& source); // Prohibit assignment operator
	public:
	virtual ~MultiStreamline(void);
	
	/* Methods: */
	const Parameters& getParameters(void) const // Returns the multi-streamline's extraction parameters
		{
		return parameters;
		}
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	MultiPolyline& getMultiPolyline(void) // Returns the multi-streamline representation
		{
		return multiPolyline;
		}
	size_t getElementSize(void) const // Returns the number of vertices in the longest streamline
		{
		/* Return the maximum number of vertices in any polyline: */
		return multiPolyline.getMaxNumVertices();
		}
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void saveParameters(Misc::File& parameterFile) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_MULTISTREAMLINE_IMPLEMENTATION
#include <Wrappers/MultiStreamline.cpp>
#endif

#endif
