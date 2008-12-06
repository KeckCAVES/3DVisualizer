/***********************************************************************
SliceVolumeRendererCartesian - Specialized texture-based version of
SliceVolumeRenderer class for Cartesian data sets.
Copyright (c) 2006-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERERCARTESIAN_INCLUDED
#define VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERERCARTESIAN_INCLUDED

#include <Templatized/SliceVolumeRenderer.h>

/* Forward declarations: */
namespace Comm {
class MulticastPipe;
}
class PaletteRenderer;
namespace Visualization {
namespace Templatized {
template <class ScalarParam,int dimensionParam,class ValueParam>
class Cartesian;
}
}

namespace Visualization {

namespace Templatized {

template <class ScalarParam,class ValueParam,class ScalarExtractorParam>
class SliceVolumeRenderer<Cartesian<ScalarParam,3,ValueParam>,ScalarExtractorParam>
	{
	/* Embedded classes: */
	public:
	typedef Cartesian<ScalarParam,3,ValueParam> DataSet; // Type of the data set the slice extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef Geometry::Plane<Scalar,DataSet::dimension> Plane; // Type for planes in the data set's domain
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Pointer to the rendered data set
	ScalarExtractor scalarExtractor; // The scalar extractor
	const GLColorMap* colorMap; // A transfer function to map scalar values to colors and opacities
	PaletteRenderer* renderer; // A texture-based volume renderer
	float transparencyGamma; // A gamma correction factor to apply to color map opacities
	
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
		return dataSet;
		}
	const ScalarExtractor& getScalarExtractor(void) const // Returns the scalar extractor
		{
		return scalarExtractor;
		}
	ScalarExtractor& getScalarExtractor(void) // Ditto
		{
		return scalarExtractor;
		}
	size_t getSize(void) const; // Returns the volume renderer's "size"
	Scalar getSliceFactor(void) const; // Returns the current slice factor
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

#ifndef VISUALIZATION_TEMPLATIZED_SLICEVOLUMERENDERERCARTESIAN_IMPLEMENTATION
#include <Templatized/SliceVolumeRendererCartesian.cpp>
#endif

#endif
