/***********************************************************************
ColoredIsosurfaceExtractor - Generic class to extract isosurfaces color-
mapped by a secondary scalar extractor from data sets.
Copyright (c) 2008-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_COLOREDISOSURFACEEXTRACTOR_INCLUDED
#define VISUALIZATION_TEMPLATIZED_COLOREDISOSURFACEEXTRACTOR_INCLUDED

#include <Misc/OneTimeQueue.h>

/* Forward declarations: */
namespace Visualization {
namespace Templatized {
template <class CellTopologyParam>
class IsosurfaceCaseTable;
}
}

namespace Visualization {

namespace Templatized {

template <class DataSetParam,class ScalarExtractorParam,class IsosurfaceParam>
class ColoredIsosurfaceExtractor
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet; // Type of the data set the isosurface extractor works on
	typedef typename DataSet::Scalar Scalar; // Scalar type of the data set's domain
	static const int dimension=DataSet::dimension; // Dimension of the data set's domain
	typedef typename DataSet::Point Point; // Type for points in the data set's domain
	typedef typename DataSet::Vector Vector; // Type for vectors in the data set's domain
	typedef typename DataSet::Locator Locator; // Type of data set locators
	typedef ScalarExtractorParam ScalarExtractor; // Type to extract scalar values from a data set
	typedef typename ScalarExtractor::Scalar VScalar; // Value type of scalar extractor
	typedef IsosurfaceParam Isosurface; // Type of isosurface representation
	
	enum ExtractionMode // Enumerated type for isosurface extraction modes
		{
		FLAT,SMOOTH
		};
	
	private:
	typedef typename DataSet::CellTopology CellTopology; // Topology of the data set's cells
	typedef typename DataSet::CellID CellID; // Type of the data set's cell IDs
	typedef typename DataSet::Cell Cell; // Type of the data set's cells
	typedef Misc::OneTimeQueue<CellID,CellID> CellQueue; // Type for queues of cell IDs waiting for expansion
	typedef IsosurfaceCaseTable<CellTopology> CaseTable; // Type of isosurface case table
	typedef typename Isosurface::Vertex Vertex; // Type of vertices stored in isosurface
	
	/* Elements: */
	private:
	const DataSet* dataSet; // Data set the isosurface extractor works on
	ScalarExtractor scalarExtractor; // Scalar extractor working on data set
	ScalarExtractor colorScalarExtractor; // Secondary scalar extractor for color values
	ExtractionMode extractionMode; // Surface extraction mode
	
	/* Isosurface extraction state: */
	VScalar isovalue; // The current isovalue
	Isosurface* isosurface; // Pointer to the isosurface representation storing extracted isosurface fragments
	CellQueue cellQueue; // Queue of cells waiting for fragment extraction
	
	/* Private methods: */
	int extractFlatIsosurfaceFragment(const Cell& cell); // Extracts a flat-shaded isosurface fragment from a cell and stores it in the current isosurface representation
	int extractSmoothIsosurfaceFragment(const Cell& cell); // Extracts a gradient-shaded isosurface fragment from a cell and stores it in the current isosurface representation
	
	/* Constructors and destructors: */
	public:
	ColoredIsosurfaceExtractor(const DataSet* sDataSet,const ScalarExtractor& sScalarExtractor,const ScalarExtractor& sColorScalarExtractor); // Creates an isosurface extractor for the given data set and scalar extractors
	private:
	ColoredIsosurfaceExtractor(const ColoredIsosurfaceExtractor& source); // Prohibit copy constructor
	ColoredIsosurfaceExtractor& operator=(const ColoredIsosurfaceExtractor& source); // Prohibit assignment operator
	public:
	~ColoredIsosurfaceExtractor(void); // Destroys the isosurface extractor
	
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
	const ScalarExtractor& getColorScalarExtractor(void) const // Returns the secondary scalar extractor
		{
		return colorScalarExtractor;
		}
	ScalarExtractor& getColorScalarExtractor(void) // Ditto
		{
		return colorScalarExtractor;
		}
	ExtractionMode getExtractionMode(void) const // Returns the current isosurface extraction mode
		{
		return extractionMode;
		}
	void update(const DataSet* newDataSet,const ScalarExtractor& newScalarExtractor) // Sets a new data set and scalar extractor for subsequent colored isosurface extraction
		{
		dataSet=newDataSet;
		scalarExtractor=newScalarExtractor;
		}
	void setColorScalarExtractor(const ScalarExtractor& newColorScalarExtractor); // Sets the scalar extractor for isosurface color values
	void setExtractionMode(ExtractionMode newExtractionMode); // Sets the current isosurface extraction mode
	void extractIsosurface(VScalar newIsovalue,Isosurface& newIsosurface); // Extracts a global isosurface for the given isovalue and stores it in the given isosurface
	void extractSeededIsosurface(const Locator& seedLocator,Isosurface& newIsosurface); // Extracts a seeded isosurface for the given isovalue from the given cell and stores it in the given isosurface
	void startSeededIsosurface(const Locator& seedLocator,Isosurface& newIsosurface); // Starts extracting a seeded isosurface for the given isovalue from the given cell
	template <class ContinueFunctorParam>
	bool continueSeededIsosurface(const ContinueFunctorParam& cf); // Continues extracting a seeded isosurface while the continue functor returns true; returns true if the isosurface is finished
	void finishSeededIsosurface(void); // Cleans up after creating a seeded isosurface
	};

}

}

#ifndef VISUALIZATION_TEMPLATIZED_COLOREDISOSURFACEEXTRACTOR_IMPLEMENTATION
#include <Templatized/ColoredIsosurfaceExtractor.icpp>
#endif

#endif
