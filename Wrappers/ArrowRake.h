/***********************************************************************
ArrowRake - Class to represent rakes of arrow glyphs as visualization
elements.
Copyright (c) 2008-2012 Oliver Kreylos

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

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKE_INCLUDED
#define VISUALIZATION_WRAPPERS_ARROWRAKE_INCLUDED

#include <Misc/ArrayIndex.h>
#include <Misc/Array.h>
#include <GL/gl.h>
#include <GL/GLObject.h>

#include <Abstract/Element.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class ArrowRake:public Visualization::Abstract::Element,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef typename DS::Point Point; // Point type in data set's domain
	typedef typename DS::Vector Vector; // Vector type in data set's domain
	
	struct Arrow // Structure representing data for an arrow glyph
		{
		/* Elements: */
		public:
		Point base; // Arrow base point
		bool valid; // Flag if the arrow is valid
		Vector direction; // Vector from arrow base point to arrow tip, before length scaling is applied
		VScalar scalarValue; // Scalar value used to color arrow glyph
		};
	
	typedef Misc::ArrayIndex<2> Index; // Type for rake array indices
	typedef Misc::Array<Arrow,2> Rake; // 2D array of arrows forming a rake
	typedef GLVertex<void,0,void,0,Scalar,Scalar,dimension> Vertex; // Data type for arrow glyph vertices
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of buffer object for vertex data
		GLuint indexBufferId; // ID of buffer object for index data
		unsigned int version; // Version number of the arrow glyphs in the buffer objects
		Scalar scaledArrowShaftRadius; // Scaled shaft radius of arrow glyphs in the buffer objects
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	int scalarVariableIndex; // Index of the scalar variable used to color the arrow rake
	Cluster::MulticastPipe* pipe; // Pipe to stream arrow rake data in a cluster environment (owned by caller)
	Rake rake; // Array containing the arrow definitions
	Scalar lengthScale; // Arrow length scale
	Scalar shaftRadius; // Radius of the shafts of the arrow glyphs
	unsigned int numArrowVertices; // Number of vertices per arrow for arrow glyph creation
	unsigned int version; // Version number of the arrow rake
	
	/* Constructors and destructors: */
	public:
	ArrowRake(Visualization::Abstract::VariableManager* sVariableManager,Visualization::Abstract::Parameters* sParameters,int sScalarVariableIndex,const Index& sRakeSize,Scalar sLengthScale,Scalar sShaftRadius,unsigned int sNumArrowVertices,Cluster::MulticastPipe* pipe); // Creates an empty arrow rake for the given parameters
	private:
	ArrowRake(const ArrowRake& source); // Prohibit copy constructor
	ArrowRake& operator=(const ArrowRake& source); // Prohibit assignment operator
	public:
	virtual ~ArrowRake(void);
	
	/* Methods from Visualization::Abstract::Element: */
	virtual std::string getName(void) const;
	virtual size_t getSize(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods: */
	const Rake& getRake(void) const // Returns the array of arrows
		{
		return rake;
		}
	Rake& getRake(void) // Ditto
		{
		return rake;
		}
	void update(void); // Updates the rake array and synchronizes across a cluster
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKE_IMPLEMENTATION
#include <Wrappers/ArrowRake.icpp>
#endif

#endif
