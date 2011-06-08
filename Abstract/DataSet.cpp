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

#include <Abstract/DataSet.h>

#include <Misc/ThrowStdErr.h>

namespace Visualization {

namespace Abstract {

/*********************************
Methods of class DataSet::Locator:
*********************************/

DataSet::Locator::Locator(void)
	:position(Point::origin),orientation(Orientation::identity)
	{
	}

DataSet::Locator::Locator(const DataSet::Locator& source)
	:position(source.position),orientation(source.orientation)
	{
	}

DataSet::Locator::~Locator(void)
	{
	}

bool DataSet::Locator::setPosition(const DataSet::Point& newPosition)
	{
	bool result=position!=newPosition;
	position=newPosition;
	return result;
	}

bool DataSet::Locator::setOrientation(const DataSet::Orientation& newOrientation)
	{
	bool result=orientation!=newOrientation;
	orientation=newOrientation;
	return result;
	}

/************************
Methods of class DataSet:
************************/

DataSet::Unit DataSet::getUnit(void) const
	{
	/* Return a default unit: */
	return Unit();
	}

int DataSet::getNumScalarVariables(void) const
	{
	return 0;
	}

const char* DataSet::getScalarVariableName(int scalarVariableIndex) const
	{
	Misc::throwStdErr("DataSet::getScalarVariableName: invalid variable index %d",scalarVariableIndex);
	return 0;
	}

ScalarExtractor* DataSet::getScalarExtractor(int scalarVariableIndex) const
	{
	Misc::throwStdErr("DataSet::getScalarExtractor: invalid variable index %d",scalarVariableIndex);
	return 0;
	}

int DataSet::getNumVectorVariables(void) const
	{
	return 0;
	}

const char* DataSet::getVectorVariableName(int vectorVariableIndex) const
	{
	Misc::throwStdErr("DataSet::getVectorVariableName: invalid variable index %d",vectorVariableIndex);
	return 0;
	}

VectorExtractor* DataSet::getVectorExtractor(int vectorVariableIndex) const
	{
	Misc::throwStdErr("DataSet::getVectorExtractor: invalid variable index %d",vectorVariableIndex);
	return 0;
	}

}

}
