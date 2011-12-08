/***********************************************************************
Parameters - Abstract base class for parameters that completely define
how to extract a visualization element from a data set using a given
visualization algorithm. Mostly used to read/write visualization
elements to files, and to transmit them over networks.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_PARAMETERS_INCLUDED
#define VISUALIZATION_ABSTRACT_PARAMETERS_INCLUDED

/* Forward declarations: */
namespace Visualization {
namespace Abstract {
class ParametersSink;
class ParametersSource;
}
}

namespace Visualization {

namespace Abstract {

class Parameters
	{
	/* Constructors and destructors: */
	public:
	virtual ~Parameters(void) // Destroys the parameter object
		{
		}
	
	/* Methods: */
	virtual bool isValid(void) const =0; // Returns true if the parameter object can be used to extract a valid visualization element
	virtual Parameters* clone(void) const =0; // Returns an exact copy of the parameter object
	virtual void write(ParametersSink& sink) const =0; // Writes parameters to a parameter sink
	virtual void read(ParametersSource& source) =0; // Reads parameters from a parameter source
	};

}

}

#endif
