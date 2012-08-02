/***********************************************************************
EarthRenderer - Class to render a configurable model of Earth using
transparent surfaces and several interior components.
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

#ifndef VISUALIZATION_CONCRETE_EARTHRENDERER_INCLUDED
#define VISUALIZATION_CONCRETE_EARTHRENDERER_INCLUDED

#include <string>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>

/* Forward declarations: */
class GLRenderState;

namespace Visualization {

namespace Concrete {

class EarthRenderer:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef GLColor<GLfloat,4> Color;
	
	private:
	struct DataItem:public GLObject::DataItem // Class to store OpenGL-related data needed by the Earth renderer
		{
		/* Elements: */
		public:
		GLuint surfaceTextureObjectId; // Texture object ID for Earth surface texture
		GLuint displayListIdBase; // Base ID of set of display lists for Earth model components
		unsigned int surfaceVersion; // Version number of surface display list
		unsigned int gridVersion; // Version number of longitude/latitude grid display list
		unsigned int outerCoreVersion; // Version number of outer core display list
		unsigned int innerCoreVersion; // Version number of inner core display list
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	static const double a; // Earth's equatorial radius in m
	static const double flatteningFactor; // Earth's geoid flattening factor
	double scaleFactor; // Scale factor for Cartesian coordinates
	double f; // Effective flattening factor used for this Earth renderer
	int surfaceDetail; // Subdivision level of surface sphere
	GLMaterial surfaceMaterial; // Material property for surface
	float surfaceOpacity; // Transparenct for surface (0.0: invisible, 1.0: fully opaque)
	unsigned int surfaceVersion; // Version number of surface display list
	int gridDetail; // Subdivision level of longitude/latitude grid
	float gridLineWidth; // Line width of longitude/latitude grid
	Color gridColor; // Color of longitude/latitude grid
	float gridOpacity; // Opacity of longitude/latitude grid (0.0: invisible, 1.0: fully opaque)
	unsigned int gridVersion; // Version number of longitude/latitude grid display list
	int outerCoreDetail; // Subdivision level of outer core sphere
	GLMaterial outerCoreMaterial; // Material property for outer core
	float outerCoreOpacity; // Transparenct for outer core (0.0: invisible, 1.0: fully opaque)
	unsigned int outerCoreVersion; // Version number of outer core display list
	int innerCoreDetail; // Subdivision level of inner core sphere
	GLMaterial innerCoreMaterial; // Material property for inner core
	float innerCoreOpacity; // Transparenct for inner core (0.0: invisible, 1.0: fully opaque)
	unsigned int innerCoreVersion; // Version number of inner core display list
	
	/* Private methods: */
	void renderSurface(DataItem* dataItem) const;
	void renderGrid(DataItem* dataItem) const;
	void renderOuterCore(DataItem* dataItem) const;
	void renderInnerCore(DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	EarthRenderer(double sScaleFactor);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	static const double getRadius(void)
		{
		return a;
		}
	static const double getFlatteningFactor(void)
		{
		return flatteningFactor;
		}
	void setScaleFactor(double newScaleFactor);
	void setFlatteningFactor(double newF);
	void setSurfaceDetail(int newSurfaceDetail);
	void setSurfaceMaterial(const GLMaterial& newSurfaceMaterial);
	void setSurfaceOpacity(float newSurfaceOpacity);
	void setGridDetail(int newGridDetail);
	void setGridLineWidth(float newGridLineWidth);
	void setGridColor(const Color& newGridColor);
	void setGridOpacity(float newGridOpacity);
	void setOuterCoreDetail(int newOuterCoreDetail);
	void setOuterCoreMaterial(const GLMaterial& newOuterCoreMaterial);
	void setOuterCoreOpacity(float newOuterCoreOpacity);
	void setInnerCoreDetail(int newInnerCoreDetail);
	void setInnerCoreMaterial(const GLMaterial& newInnerCoreMaterial);
	void setInnerCoreOpacity(float newInnerCoreOpacity);
	void glRenderAction(GLRenderState& renderState) const;
	};

}

}

#endif
