/***********************************************************************
LinearIndexID - Helper class to use unsigned integers as IDs for data
set objects such as vertices, edges, and cells.
Copyright (c) 2006-2007 Oliver Kreylos

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

#ifndef VISUALIZATION_TEMPLATIZED_LINEARINDEXID_INCLUDED
#define VISUALIZATION_TEMPLATIZED_LINEARINDEXID_INCLUDED

namespace Visualization {

namespace Templatized {

class LinearIndexID
	{
	/* Embedded classes: */
	public:
	typedef unsigned int Index; // Type for linear indices
	
	/* Elements: */
	private:
	Index index; // Linear index
	
	/* Constructors and destructors: */
	public:
	LinearIndexID(void) // Constructs invalid ID
		:index(~Index(0))
		{
		}
	LinearIndexID(Index sIndex)
		:index(sIndex)
		{
		}

	/* Methods: */
	bool isValid(void) const // Returns true if the ID identifies a valid object
		{
		return index!=~Index(0);
		}
	Index getIndex(void) const
		{
		return index;
		}
	friend bool operator==(const LinearIndexID& li1,const LinearIndexID& li2)
		{
		return li1.index==li2.index;
		}
	friend bool operator!=(const LinearIndexID& li1,const LinearIndexID& li2)
		{
		return li1.index!=li2.index;
		}
	static size_t hash(const LinearIndexID& li,size_t tableSize)
		{
		return size_t(li.index)%tableSize;
		}
	};

}

}

#endif
