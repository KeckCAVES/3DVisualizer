/***********************************************************************
SliceVolumeRenderer - Generic class to render volumetric images of data
sets using incremental slices.
Copyright (c) 2005-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERER_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLVertex.h>
#include <Templatized/TriangleRenderer.h>
#include <Templatized/SliceExtractor.h>

/* Forward declarations: */
namespace Comm {
class MulticastPipe;
}
class GLContextData;
class GLColorMap;

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class ScalarExtractorParam>
class SliceVolumeRenderer
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the slice extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef Geometry::Plane<Scalar,DataSet::dimension> Plane; // Type for planes in the data set's domain
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	
	private:
	typedef GLVertex<GLfloat,1,void,0,void,GLfloat,3> Vertex; // Type for slice vertices
	typedef TriangleRenderer<Vertex> TR; // Triangle renderer used by slice extractor
	typedef SliceExtractor<DataSet,ScalarExtractor,TR> SE; // Slice extractor to generate slices
	
	/* Elements: */
	mutable SE se; // The slice extractor to generate volume rendering slices
	Scalar sliceFactor; // The distance between two slices in multiples of average cell size
	float transparencyGamma; // A gamma correction factor to apply to color map opacities
	const GLColorMap* colorMap; // A transfer function to map scalar values to colors and opacities
	
	/* Constructors and destructors: */
	public:
	public:
	SliceVolumeRenderer(const DataSet* sDataSet,const ScalarExtractor& sScalarExtractor,const GLColorMap* sColorMap,Comm::MulticastPipe* sPipe); // Creates volume renderer for the given data set and scalar extractor
	private:
	SliceVolumeRenderer(const SliceVolumeRenderer& source); // Prohibit copy constructor
	SliceVolumeRenderer& operator=(const SliceVolumeRenderer& source); // Prohibit assignment operator
	public:
	~SliceVolumeRenderer(void); // Destroys the volume renderer
	
	/* Methods: */
	const DataSet* getDataSet(void) const // Returns the data set
		{
		return se.getDataSet();
		}
	const ScalarExtractor& getScalarExtractor(void) const // Returns the scalar extractor
		{
		return se.getScalarExtractor();
		}
	ScalarExtractor& getScalarExtractor(void) // Ditto
		{
		return se.getScalarExtractor();
		}
	size_t getSize(void) const // Returns the volume renderer's "size"
		{
		return 0; // This one doesn't really have a size
		}
	Scalar getSliceFactor(void) const // Returns the current slice factor
		{
		return sliceFactor;
		}
	void setSliceFactor(Scalar newSliceFactor); // Sets the slice factor
	float getTransparencyGamma(void) const // Returns the current transparency gamma correction factor
		{
		return transparencyGamma;
		}
	void setTransparencyGamma(float newTransparencyGamma); // Sets the transparency gamma correction factor
	void renderVolume(const Point& sliceCenter,const Vector& viewDirection,GLContextData& contextData) const; // Renders the data set
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERER_IMPLEMENTATION
#include <Templatized/SliceVolumeRenderer.cpp>
#endif

#endif
