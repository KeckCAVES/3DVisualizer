/***********************************************************************
VectorExtractor - Wrapper class to map from the abstract vector
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

#ifndef VISUALIZATION_WRAPPERS_VECTOREXTRACTOR_INCLUDED
#define VISUALIZATION_WRAPPERS_VECTOREXTRACTOR_INCLUDED

#include <Abstract/VectorExtractor.h>

namespace Visualization {

namespace Wrappers {

template <class VEParam>
class VectorExtractor:public Visualization::Abstract::VectorExtractor
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::VectorExtractor Base; // Type of base class
	typedef VEParam VE; // Type of templatized vector extractor
	
	/* Elements: */
	private:
	VE ve; // Templatized vector extractor
	
	/* Constructors and destructors: */
	public:
	VectorExtractor(const VE& sVe)
		:ve(sVe)
		{
		}
	protected:
	VectorExtractor(const VectorExtractor& source) // Protect copy constructor
		:Base(source),
		 ve(source.ve)
		{
		}
	private:
	VectorExtractor& operator=(const VectorExtractor& source); // Prohibit assignment operator
	
	/* Methods: */
	public:
	virtual Base* clone(void) const
		{
		return new VectorExtractor(*this);
		}
	const VE& getVe(void) const // Returns the templatized vector extractor
		{
		return ve;
		}
	};

}

}

#endif
