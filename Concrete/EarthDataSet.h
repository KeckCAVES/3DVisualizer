/***********************************************************************
EarthDataSet - Wrapper class to add an Earth renderer to an arbitrary
visualization module working on whole-Earth grids.
Copyright (c) 2007-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_CONCRETE_EARTHDATASET_INCLUDED
#define VISUALIZATION_CONCRETE_EARTHDATASET_INCLUDED

#include <string>
#include <vector>
#include <GL/gl.h>
#include <GL/GLColor.h>

#include <Abstract/DataSet.h>
#include <Concrete/EarthRenderer.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class DataSet;
class CoordinateTransformer;
}
namespace Concrete {
class SphericalCoordinateTransformer;
class PointSet;
}
}

namespace Visualization {

namespace Concrete {

template <class DataSetBaseParam>
class EarthDataSet:public DataSetBaseParam // Data set class
	{
	/* Embedded classes: */
	public:
	typedef DataSetBaseParam DataSetBase; // Base class for data set wrapper class
	
	/* Elements: */
	private:
	double flatteningFactor; // Flattening factor to be used by the Earth renderer
	SphericalCoordinateTransformer* coordinateTransformer; // Coordinate transformer object for this Earth data set
	std::vector<std::string> pointSetFileNames; // List of point set files to load for the Earth data set renderer
	
	/* Constructors and destructors: */
	public:
	EarthDataSet(const std::vector<std::string>& args); // Creates a data set by parsing the given argument list
	virtual ~EarthDataSet(void);
	
	/* Methods from Abstract::DataSet: */
	virtual Visualization::Abstract::CoordinateTransformer* getCoordinateTransformer(void) const;
	virtual Abstract::DataSet::Unit getUnit(void) const;
	
	/* New methods: */
	void setFlatteningFactor(double newFlatteningFactor); // Sets the flattening factor to use for the Earth renderer
	double getFlatteningFactor(void) const // Returns the flattening factor to use for the Earth renderer
		{
		return flatteningFactor;
		}
	const SphericalCoordinateTransformer* getSphericalCoordinateTransformer(void) const // Returns the spherical coordinate transformer for this data set
		{
		return coordinateTransformer;
		}
	SphericalCoordinateTransformer* getSphericalCoordinateTransformer(void) // Ditto
		{
		return coordinateTransformer;
		}
	const std::vector<std::string>& getPointSetFileNames(void) const // Returns the list of point set file names
		{
		return pointSetFileNames;
		}
	};

template <class DataSetBaseParam,class DataSetRendererBaseParam>
class EarthDataSetRenderer:public DataSetRendererBaseParam // Data set renderer class
	{
	/* Embedded classes: */
	public:
	typedef DataSetBaseParam DataSetBase; // Base class for data set wrapper class
	typedef Visualization::Concrete::EarthDataSet<DataSetBase> EarthDataSet;  // Type of compatible data set class
	typedef DataSetRendererBaseParam DataSetRendererBase; // Base class type
	
	/* Elements: */
	private:
	static const int numPointSetColors=6;
	static const GLColor<GLfloat,3> pointSetColors[numPointSetColors]; // Colors to render the loaded point sets
	EarthRenderer er; // An Earth renderer object
	bool drawEarthModel; // Flag whether to draw the Earth model
	std::vector<PointSet*> pointSets; // List of point sets to render with the Earth model
	
	/* Constructors and destructors: */
	public:
	EarthDataSetRenderer(const Visualization::Abstract::DataSet* sDataSet);
	private:
	EarthDataSetRenderer(const EarthDataSetRenderer& source); // Prohibit copy constructor
	EarthDataSetRenderer& operator=(const EarthDataSetRenderer& source); // Prohibit assignment operator
	public:
	virtual ~EarthDataSetRenderer(void);
	
	/* Methods: */
	virtual typename DataSetRendererBase::Base* clone(void) const
		{
		return new EarthDataSetRenderer(*this);
		}
	virtual int getNumRenderingModes(void) const
		{
		return DataSetRendererBase::getNumRenderingModes()+1;
		}
	virtual const char* getRenderingModeName(int renderingModeIndex) const
		{
		if(renderingModeIndex<DataSetRendererBase::getNumRenderingModes())
			return DataSetRendererBase::getRenderingModeName(renderingModeIndex);
		else
			return "Draw Earth Model";
		}
	virtual int getRenderingMode(void) const
		{
		if(drawEarthModel)
			return DataSetRendererBase::getNumRenderingModes();
		else
			return DataSetRendererBase::getRenderingMode();
		}
	virtual void setRenderingMode(int renderingModeIndex)
		{
		if(renderingModeIndex<DataSetRendererBase::getNumRenderingModes())
			{
			drawEarthModel=false;
			DataSetRendererBase::setRenderingMode(renderingModeIndex);
			}
		else
			drawEarthModel=true;
		}
	virtual void glRenderAction(GLRenderState& renderState) const;
	};

}

}

#ifndef VISUALIZATION_CONCRETE_EARTHDATASET_IMPLEMENTATION
#include <Concrete/EarthDataSet.icpp>
#endif

#endif
