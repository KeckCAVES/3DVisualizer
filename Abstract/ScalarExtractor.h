/***********************************************************************
ScalarExtractor - Abstract base class encapsulating how to extract
scalar values from (compound) data values.
Part of the abstract interface to the templatized visualization
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

#ifndef VISUALIZATION_ABSTRACT_SCALAREXTRACTOR_INCLUDED
#define VISUALIZATION_ABSTRACT_SCALAREXTRACTOR_INCLUDED

namespace Visualization {

namespace Abstract {

class ScalarExtractor
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Type of reported scalars
	
	/* Constructors and destructors: */
	ScalarExtractor(void) // Default constructor
		{
		}
	protected:
	ScalarExtractor(const ScalarExtractor& source) // Protect copy constructor
		{
		}
	private:
	ScalarExtractor& operator=(const ScalarExtractor& source); // Prohibit assignment operator
	public:
	virtual ~ScalarExtractor(void) // Destructor
		{
		}
	
	/* Methods: */
	virtual ScalarExtractor* clone(void) const =0; // Returns an identical copy of the scalar extractor object
	};

}

}

#endif
