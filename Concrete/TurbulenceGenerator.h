/***********************************************************************
TurbulenceGenerator - Class to generate artificial volumetric turbulence
data sets to test visualization algorithms.
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

#ifndef VISUALIZATION_CONCRETE_TURBULENCEGENERATOR_INCLUDED
#define VISUALIZATION_CONCRETE_TURBULENCEGENERATOR_INCLUDED

#include <Abstract/Module.h>

namespace Visualization {

namespace Concrete {

class TurbulenceGenerator:public Visualization::Abstract::Module
	{
	/* Methods: */
	public:
	virtual Visualization::Abstract::DataSet* load(const char* filename) const;
	virtual Visualization::Abstract::DataSetRenderer* getRenderer(const Visualization::Abstract::DataSet* dataSet) const;
	virtual int getNumScalarAlgorithms(void) const;
	virtual const char* getScalarAlgorithmName(int scalarAlgorithmIndex) const;
	virtual Visualization::Abstract::Algorithm* getScalarAlgorithm(int scalarAlgorithmIndex,const GLColorMap* colorMap,const Visualization::Abstract::DataSet* dataSet,const Visualization::Abstract::ScalarExtractor* scalarExtractor) const;
	};

}

}

#endif
