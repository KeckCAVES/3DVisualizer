/***********************************************************************
Element - Abstract base class for visualization elements extracted from
data sets. Elements use thread-safe reference counting for automatic
garbage collection.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_ELEMENT_INCLUDED
#define VISUALIZATION_ABSTRACT_ELEMENT_INCLUDED

#include <string>
#include <Threads/RefCounted.h>

/* Forward declarations: */
namespace Misc {
class File;
}
namespace GLMotif {
class WidgetManager;
class Widget;
}
namespace Visualization {
namespace Abstract {
class VariableManager;
class Parameters;
}
}
class GLRenderState;

namespace Visualization {

namespace Abstract {

class Element:public Threads::RefCounted
	{
	/* Elements: */
	protected:
	VariableManager* variableManager; // Pointer to the variable manager
	Parameters* parameters; // Pointer to the parameters that were used to create this visualization element
	
	/* Constructors and destructors: */
	public:
	Element(VariableManager* sVariableManager,Parameters* sParameters) // Creates an "empty" visualization element that will inherit the given parameter object
		:variableManager(sVariableManager),parameters(sParameters)
		{
		}
	private:
	Element(const Element& source); // Prohibit copy constructor
	Element& operator=(const Element& source); // Prohibit assignment operator
	public:
	virtual ~Element(void); // Destroys the visualization element
	
	/* Methods: */
	const Parameters* getParameters(void) const // Returns pointer to parameter object
		{
		return parameters;
		}
	Parameters* getParameters(void) // Ditto
		{
		return parameters;
		}
	virtual std::string getName(void) const =0; // Returns a descriptive name for the visualization element
	virtual size_t getSize(void) const =0; // Returns some size value for the visualization element to compare it to other elements of the same type (number of triangles, points, etc.)
	virtual bool usesTransparency(void) const; // Returns true if the visualization element uses transparency (and needs to be rendered last)
	virtual GLMotif::Widget* createSettingsDialog(GLMotif::WidgetManager* widgetManager); // Returns a new UI widget to change internal settings of the element
	virtual void glRenderAction(GLRenderState& renderState) const =0; // Renders a visualization element into the given OpenGL context
	};

}

}

#endif
