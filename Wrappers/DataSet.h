/***********************************************************************
DataSet - Wrapper class to map from the abstract data set interface to
its templatized data set implementation.
Copyright (c) 2005-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_DATASET_INCLUDED
#define VISUALIZATION_WRAPPERS_DATASET_INCLUDED

#include <Abstract/DataSet.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Vector;
}
namespace Visualization {
namespace Templatized {
template <class ScalarParam,class SourceValueParam>
class ScalarExtractor;
template <class VectorParam,class SourceValueParam>
class VectorExtractor;
}
namespace Wrappers {
template <class SEParam>
class ScalarExtractor;
template <class VEParam>
class VectorExtractor;
}
}

namespace Visualization {

namespace Wrappers {

template <class DSParam,class VScalarParam,class DataValueParam>
class DataSet:public Visualization::Abstract::DataSet
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::DataSet Base; // Base class
	typedef Base::Scalar Scalar; // Data set scalar type
	typedef Base::VScalar DestScalar; // Destination type for scalar extraction
	typedef Base::VVector DestVector; // Destination type for vector extraction
	typedef Base::VScalarRange DestScalarRange; // Destination type for scalar range extraction
	typedef Base::Locator BaseLocator; // Base class for locators
	typedef DSParam DS; // Type of templatized data set
	typedef typename DS::Value DSValue; // Value type of templatized data set
	typedef typename DS::Locator DSL; // Type of templatized locator
	typedef VScalarParam VScalar; // Scalar value type
	typedef Geometry::Vector<VScalar,DS::dimension> VVector; // Vector value type
	typedef Visualization::Templatized::ScalarExtractor<VScalar,DSValue> SE; // Type of templatized scalar extractor
	typedef Visualization::Wrappers::ScalarExtractor<SE> ScalarExtractor; // Compatible scalar extractor wrapper class
	typedef Visualization::Templatized::VectorExtractor<VVector,DSValue> VE; // Type of templatized vector extractor
	typedef Visualization::Wrappers::VectorExtractor<VE> VectorExtractor; // Compatible vector extractor wrapper class
	typedef DataValueParam DataValue; // Type of data value descriptor
	
	class Locator:public BaseLocator
		{
		/* Elements: */
		private:
		DSL dsl; // The templatized locator
		bool valid; // Flag if the locator is currently valid
		
		/* Constructors and destructors: */
		public:
		Locator(const DS& ds); // Creates a locator for the given data set
		protected:
		Locator(const Locator& source); // Protect copy constructor
		private:
		Locator& operator=(const Locator& source); // Prohibit assignment operator
		
		/* Methods: */
		public:
		const DSL& getDsl(void) const // Returns the templatized locator
			{
			return dsl;
			}
		virtual BaseLocator* clone(void) const;
		virtual bool setPosition(const Point& newPosition);
		virtual bool isValid(void) const
			{
			return valid;
			}
		virtual DestScalar calcScalar(const Visualization::Abstract::ScalarExtractor* scalarExtractor) const;
		virtual DestVector calcVector(const Visualization::Abstract::VectorExtractor* vectorExtractor) const;
		};
	
	/* Elements: */
	private:
	DataValue dataValue; // Descriptor for data values stored in the data set
	DS ds; // The templatized data set
	
	/* Constructors and destructors: */
	public:
	DataSet(void) // Default constructor
		{
		}
	private:
	DataSet(const DataSet& source); // Prohibit copy constructor
	DataSet& operator=(const DataSet& source); // Prohibit assignment operator
	public:
	virtual ~DataSet(void)
		{
		}
	
	/* Methods: */
	const DataValue& getDataValue(void) const // Returns the data value descriptor
		{
		return dataValue;
		}
	DataValue& getDataValue(void) // Ditto
		{
		return dataValue;
		}
	const DS& getDs(void) const // Returns templatized data set
		{
		return ds;
		}
	DS& getDs(void) // Ditto
		{
		return ds;
		}
	virtual Visualization::Abstract::CoordinateTransformer* getCoordinateTransformer(void) const;
	virtual Box getDomainBox(void) const
		{
		return Box(ds.getDomainBox());
		}
	virtual Scalar calcAverageCellSize(void) const
		{
		return Scalar(ds.calcAverageCellSize());
		}
	virtual int getNumScalarVariables(void) const;
	virtual const char* getScalarVariableName(int scalarVariableIndex) const;
	virtual Visualization::Abstract::ScalarExtractor* getScalarExtractor(int scalarVariableIndex) const;
	virtual DestScalarRange calcScalarValueRange(const Visualization::Abstract::ScalarExtractor* scalarExtractor) const;
	virtual int getNumVectorVariables(void) const;
	virtual const char* getVectorVariableName(int vectorVariableIndex) const;
	virtual Visualization::Abstract::VectorExtractor* getVectorExtractor(int vectorVariableIndex) const;
	virtual DestScalarRange calcVectorValueMagnitudeRange(const Visualization::Abstract::VectorExtractor* vectorExtractor) const;
	virtual BaseLocator* getLocator(void) const
		{
		return new Locator(ds);
		}
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_DATASET_IMPLEMENTATION
#include <Wrappers/DataSet.icpp>
#endif

#endif
