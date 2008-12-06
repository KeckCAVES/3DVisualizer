/***********************************************************************
GlobalIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
implementation.
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

#ifndef VISUALIZATION_WRAPPERS_GLOBALISOSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_GLOBALISOSURFACEEXTRACTOR_INCLUDED

#include <Misc/Autopointer.h>

#include <Abstract/DataSet.h>
#include <Abstract/Algorithm.h>

/* Forward declarations: */
class GLColorMap;
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class Element;
}
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
namespace Templatized {
template <class DataSetParam,class ScalarExtractorParam>
class IsosurfaceExtractor;
}
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
template <class DataSetParam>
class Isosurface;
}
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class GlobalIsosurfaceExtractor:public Visualization::Abstract::Algorithm
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Algorithm Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DataSetWrapper::SE SE; // Type of templatized scalar extractor
	typedef typename SE::Scalar VScalar; // Value type of scalar extractor
	typedef typename DataSetWrapper::ScalarExtractor ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Isosurface<DataSetWrapper> Isosurface; // Type of created visualization elements
	typedef Misc::Autopointer<Isosurface> IsosurfacePointer; // Type for pointers to created visualization elements
	typedef typename Isosurface::Surface Surface; // Type of low-level surface representation
	typedef Visualization::Templatized::IsosurfaceExtractor<DS,SE,Surface> ISE; // Type of templatized isosurface extractor
	
	/* Elements: */
	private:
	const GLColorMap* colorMap; // The color map for extracted isosurfaces
	ISE ise; // The templatized isosurface extractor
	VScalar isovalue; // The current isovalue
	
	/* Private methods: */
	static const DS* getDs(const Visualization::Abstract::DataSet* sDataSet);
	static const SE& getSe(const Visualization::Abstract::ScalarExtractor* sScalarExtractor);
	
	/* Constructors and destructors: */
	public:
	GlobalIsosurfaceExtractor(const GLColorMap* sColorMap,const Visualization::Abstract::DataSet* sDataSet,const Visualization::Abstract::ScalarExtractor* sScalarExtractor); // Creates an isosurface extractor for the given data set and scalar extractor
	virtual ~GlobalIsosurfaceExtractor(void);
	
	/* Methods: */
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	const ISE& getIse(void) const // Returns the templatized isosurface extractor
		{
		return ise;
		}
	ISE& getIse(void) // Ditto
		{
		return ise;
		}
	void setIsovalue(VScalar newIsovalue); // Sets the isovalue for subsequent element extraction
	virtual bool hasGlobalCreator(void) const;
	virtual Visualization::Abstract::Element* createElement(void);
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_GLOBALISOSURFACEEXTRACTOR_IMPLEMENTATION
#include <Wrappers/GlobalIsosurfaceExtractor.cpp>
#endif

#endif
