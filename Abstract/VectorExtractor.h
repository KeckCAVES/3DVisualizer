/***********************************************************************
VectorExtractor - Abstract base class encapsulating how to extract
vector values from (compound) data values.
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

#ifndef VISUALIZATION_ABSTRACT_VECTOREXTRACTOR_INCLUDED
#define VISUALIZATION_ABSTRACT_VECTOREXTRACTOR_INCLUDED

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Vector;
}

namespace Visualization {

namespace Abstract {

class VectorExtractor
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Vector<double,3> Vector; // Type of reported vectors
	
	/* Constructors and destructors: */
	VectorExtractor(void) // Default constructor
		{
		}
	protected:
	VectorExtractor(const VectorExtractor& source) // Protect copy constructor
		{
		}
	private:
	VectorExtractor& operator=(const VectorExtractor& source); // Prohibit assignment operator
	public:
	virtual ~VectorExtractor(void) // Destructor
		{
		}
	
	/* Methods: */
	virtual VectorExtractor* clone(void) const =0; // Returns an identical copy of the vector extractor object
	};

}

}

#endif
