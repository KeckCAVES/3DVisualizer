/***********************************************************************
ScalarExtractor - Wrapper class to map from the abstract scalar
extractor interface to its templatized implementation.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_SCALAREXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_SCALAREXTRACTOR_INCLUDED

#include <Abstract/ScalarExtractor.h>

namespace Visualization {

namespace Wrappers {

template <class SEParam>
class ScalarExtractor:public Visualization::Abstract::ScalarExtractor
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::ScalarExtractor Base; // Base class type
	typedef SEParam SE; // Type of templatized scalar extractor
	
	/* Elements: */
	private:
	SE se; // Templatized scalar extractor
	
	/* Constructors and destructors: */
	public:
	ScalarExtractor(const SE& sSe)
		:se(sSe)
		{
		}
	protected:
	ScalarExtractor(const ScalarExtractor& source) // Protect copy constructor
		:Base(source),
		 se(source.se)
		{
		}
	private:
	ScalarExtractor& operator=(const ScalarExtractor& source); // Prohibit assignment operator
	
	/* Methods: */
	public:
	virtual Base* clone(void) const
		{
		return new ScalarExtractor(*this);
		}
	const SE& getSe(void) const // Returns the templatized scalar extractor
		{
		return se;
		}
	};

}

}

#endif
