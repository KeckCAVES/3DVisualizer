/***********************************************************************
ColorBar - A widget to display color bars with tick marks and numerical
values.
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

#ifndef COLORBAR_INCLUDED
#define COLORBAR_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLFont.h>
#include <GLMotif/Widget.h>

/* Forward declarations: */
class GLColorMap;

namespace GLMotif {

class ColorBar:public Widget,public GLObject
	{
	/* Embedded classes: */
	private:
	struct TickMark // Structure representing a tick mark
		{
		/* Elements: */
		public:
		char* label; // Label text for tick mark
		Box labelBox; // Position and natural size of label text
		GLFont::TBox labelTexCoords; // Texture coordinates of label texture
		
		/* Constructors and destructors: */
		public:
		TickMark(void)
			:label(0)
			{
			}
		~TickMark(void)
			{
			delete[] label;
			}
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		int numTickMarks; // Number of tick mark label texture objects
		GLuint* textureObjectIds; // Array of tick mark label texture objects
		GLuint tickMarksVersion; // Version number of all tick marks

		/* Constructors and destructors: */
		DataItem(int sNumTickMarks);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	GLfloat marginWidth; // Width of margin around color bar widget
	GLfloat colorBarHeight; // Preferred height of color bar
	Box colorBarBox; // Position and size of color bar
	double valueMin,valueMax; // Value range of color bar
	GLColorMap* colorMap; // Pointer to used color map
	GLFont* font; // Pointer to the font used to render tick marks
	GLfloat tickMarkHeight; // Height of tick marks themselves
	GLfloat tickMarkWidth; // Width of tick mark at the base
	Box tickMarkLabelBox; // Box containing all tick mark labels
	int tickMarkLabelPrecision; // Number of digits of precision to print for each tick mark
	GLfloat tickMarkLabelSeparation; // Minimum separation between adjacent tick mark labels
	GLfloat tickMarkLabelHeight; // Maximum height of any tick mark label
	int numTickMarks; // Number of tick marks to place underneath color bar
	TickMark* tickMarks; // Array of tick mark labels
	GLuint tickMarksVersion; // Version number of all tick marks
	
	/* Private methods: */
	void updateTickMarks(void); // Updates tick marks after any changes
	void layout(void); // Lays out the widget after a resize or change
	
	/* Constructors and destructors: */
	public:
	ColorBar(const char* sName,Container* sParent,GLfloat sColorBarHeight,int sTickMarkLabelPrecision,int sNumTickMarks,bool sManageChild =true);
	virtual ~ColorBar(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	
	/* Methods inherited from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	void setColorMap(GLColorMap* newColorMap); // Sets a new color map
	void setValueRange(double newValueMin,double newValueMax); // Sets a new value range
	};

}

#endif
