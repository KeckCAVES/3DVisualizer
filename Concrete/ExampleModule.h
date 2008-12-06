/***********************************************************************
ExampleModule - Example class demonstrating how to write new modules for
3D Visualizer. Reads single-valued data in Cartesian coordinates from
simple ASCII files.
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

#ifndef VISUALIZATION_CONCRETE_EXAMPLEMODULE_INCLUDED
#define VISUALIZATION_CONCRETE_EXAMPLEMODULE_INCLUDED

/***********************************************************************
Header files defining basic grid data structures and matching
visualization algorithms:
***********************************************************************/

#include <Wrappers/SlicedCurvilinearIncludes.h>
#include <Wrappers/SlicedScalarVectorDataValue.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Concrete {

namespace {

/***********************************************************************
Basic type declarations, can be adapted according to requirements:
***********************************************************************/

typedef float Scalar; // Data set uses 32-bit floats to store vertex positions
typedef float VScalar; // Data set uses 32-bit floats to store vertex values

/***********************************************************************
The following type declarations define how data is represented
internally, and do not have to be changed:
***********************************************************************/

typedef Visualization::Templatized::SlicedCurvilinear<Scalar,3,VScalar> DS; // Templatized data set type
typedef Visualization::Wrappers::SlicedScalarVectorDataValue<DS,VScalar> DataValue; // Type of data value descriptor
typedef Visualization::Wrappers::Module<DS,DataValue> BaseModule; // Module base class type

}

/***********************************************************************
Declaration of the module class:
***********************************************************************/

class ExampleModule:public BaseModule
	{
	/* Constructors and destructors: */
	public:
	ExampleModule(void); // Default constructor
	
	/*********************************************************************
	Method to load a data set from a file, given a particular command
	line. This method defines the format of the data files read by this
	module class and has to be written.
	*********************************************************************/
	virtual Visualization::Abstract::DataSet* load(const std::vector<std::string>& args) const; // Method to load a data set from a file
	};

}

}

#endif
