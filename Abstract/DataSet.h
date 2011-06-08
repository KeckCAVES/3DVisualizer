/***********************************************************************
DataSet - Abstract base class to represent data sets.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2010 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_DATASET_INCLUDED
#define VISUALIZATION_ABSTRACT_DATASET_INCLUDED

#include <utility>
#include <Geometry/Point.h>
#include <Geometry/Rotation.h>
#include <Geometry/Box.h>
#include <Geometry/LinearUnit.h>
#include <Abstract/ScalarExtractor.h>
#include <Abstract/VectorExtractor.h>

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class DataValue;
class CoordinateTransformer;
}
}

namespace Visualization {

namespace Abstract {

class DataSet
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type for data set's domain
	typedef Geometry::Point<Scalar,3> Point; // Point type in data set's domain
	typedef Geometry::Rotation<Scalar,3> Orientation; // Orientation type in data set's domain
	typedef Geometry::Box<Scalar,3> Box; // Axis-aligned box type in data set's domain
	typedef Geometry::LinearUnit Unit; // Type for linear coordinate units
	typedef ScalarExtractor::Scalar VScalar; // Scalar value type
	typedef VectorExtractor::Vector VVector; // Vector value type
	typedef std::pair<VScalar,VScalar> VScalarRange; // Type for scalar value ranges
	
	class Locator // Class to encapsulate probes to evaluate data sets at arbitrary positions
		{
		/* Elements: */
		private:
		Point position; // Current position of the probe
		Orientation orientation; // Current orientation of the probe
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Constructs locator for the given data set
		protected:
		Locator(const Locator& source); // Protect copy constructor
		private:
		Locator& operator=(const Locator& source); // Prohibit assignment operator
		public:
		virtual ~Locator(void); // Destructor
		
		/* Methods: */
		virtual Locator* clone(void) const =0; // Returns identical copy of locator object
		const Point& getPosition(void) const // Returns locator's position
			{
			return position;
			}
		const Orientation& getOrientation(void) const // Returns locator's orientation
			{
			return orientation;
			}
		virtual bool setPosition(const Point& newPosition); // Sets the locator's position; returns true if position changed
		virtual bool setOrientation(const Orientation& newOrientation); // Sets the locator's orientation; returns true if position changed
		virtual bool isValid(void) const =0; // Returns true if the locator is inside the data set's domain
		virtual VScalar calcScalar(const ScalarExtractor* scalarExtractor) const =0; // Calculates scalar value at current locator position (locator must be valid)
		virtual VVector calcVector(const VectorExtractor* vectorExtractor) const =0; // Calculates vector value at current locator position (locator must be valid)
		};
	
	/* Constructors and destructors: */
	public:
	DataSet(void) // Default constructor
		{
		}
	private:
	DataSet(const DataSet& source); // Prohibit copy constructor
	DataSet& operator=(const DataSet& source); // Prohibit assignment operator
	public:
	virtual ~DataSet(void) // Destructor
		{
		}
	
	/* Methods: */
	virtual CoordinateTransformer* getCoordinateTransformer(void) const =0; // Returns a new coordinate transformer from the data set's Cartesian coordinates back to its source coordinates
	virtual Unit getUnit(void) const; // Returns the unit used by the data set's Cartesian coordinate space
	virtual Box getDomainBox(void) const =0; // Returns an axis-aligned box enclosing the data set's domain
	virtual Scalar calcAverageCellSize(void) const =0; // Returns an estimate of the data set's average cell size
	virtual int getNumScalarVariables(void) const; // Returns number of scalar variables contained in the data set
	virtual const char* getScalarVariableName(int scalarVariableIndex) const; // Returns descriptive name of a scalar variable
	virtual ScalarExtractor* getScalarExtractor(int scalarVariableIndex) const; // Returns scalar extractor for a scalar variable
	virtual VScalarRange calcScalarValueRange(const ScalarExtractor* scalarExtractor) const =0; // Calculates the range of scalar values extracted by the given extractor
	virtual int getNumVectorVariables(void) const; // Returns number of vector variables contained in the data set
	virtual const char* getVectorVariableName(int vectorVariableIndex) const; // Returns descriptive name of a vector variable
	virtual VectorExtractor* getVectorExtractor(int vectorVariableIndex) const; // Returns vector extractor for a vector variable
	virtual VScalarRange calcVectorValueMagnitudeRange(const VectorExtractor* vectorExtractor) const =0; // Calculates the magnitude range of vector values extracted by the given extractor
	virtual Locator* getLocator(void) const =0; // Returns an invalid locator for the data set
	};

}

}

#endif
