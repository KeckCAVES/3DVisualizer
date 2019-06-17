/***********************************************************************
UnstructuredHexahedralVTK - Class reading unstructured hexahedral data
sets from files in legacy VTK format.
Copyright (c) 2016 Oliver Kreylos

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

#ifndef VISUALIZATION_CONCRETE_UNSTRUCTRUREDHEXAHEDRALVTK_INCLUDED
#define VISUALIZATION_CONCRETE_UNSTRUCTRUREDHEXAHEDRALVTK_INCLUDED

#include <Wrappers/SlicedHypercubicIncludes.h>
#include <Wrappers/SlicedScalarVectorDataValue.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Concrete {

namespace {

/* Basic type declarations: */
typedef float Scalar; // Scalar type of data set domain
typedef double VScalar; // Scalar type of data set value
typedef Visualization::Templatized::SlicedHypercubic<Scalar,3,VScalar> DS; // Templatized data set type
typedef Visualization::Wrappers::SlicedScalarVectorDataValue<DS,VScalar> DataValue; // Type of data value descriptor
typedef Visualization::Wrappers::Module<DS,DataValue> BaseModule; // Module base class type

}

class UnstructuredHexahedralVTK:public BaseModule
	{
	/* Constructors and destructors: */
	public:
	UnstructuredHexahedralVTK(void); // Default constructor
	
	/* Methods: */
	virtual Visualization::Abstract::DataSet* load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const;
	};

}

}

#endif
