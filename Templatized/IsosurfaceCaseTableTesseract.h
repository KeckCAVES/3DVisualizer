/***********************************************************************
IsosurfaceCaseTableTesseract - Specialized versions of
IsosurfaceCaseTable for two- and three-dimensional tesseracts.
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

#ifndef VISUALIZATION_TEMPLATIZED_ISOSURFACECASETABLETESSERACT_INCLUDED
#define VISUALIZATION_TEMPLATIZED_ISOSURFACECASETABLETESSERACT_INCLUDED

#include <Templatized/IsosurfaceCaseTable.h>
#include <Templatized/Tesseract.h>

namespace Visualization {

namespace Templatized {

template <>
class IsosurfaceCaseTable<Tesseract<3> >
	{
	/* Embedded classes: */
	public:
	typedef Tesseract<3> CellTopology;
	
	/* Elements: */
	static const int triangleEdgeIndices[256][16];
	static const int edgeMasks[256];
	static const int neighbourMasks[256];
	};

}

}

#endif
