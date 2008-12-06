/***********************************************************************
PointerID - Helper class to use pointers as IDs for data set objects
such as vertices, edges, and cells.
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

#ifndef VISUALIZATION_TEMPLATIZED_POINTERID_INCLUDED
#define VISUALIZATION_TEMPLATIZED_POINTERID_INCLUDED

namespace Visualization {

namespace Templatized {

template <class ObjectParam>
class PointerID
	{
	/* Embedded classes: */
	public:
	typedef ObjectParam Object; // Type of identified objects
	
	/* Elements: */
	private:
	const Object* object; // Pointer to identified object
	
	/* Constructors and destructors: */
	public:
	PointerID(void) // Constructs invalid ID
		:object(0)
		{
		}
	PointerID(const Object* sObject)
		:object(sObject)
		{
		}

	/* Methods: */
	bool isValid(void) const // Returns true if the ID identifies a valid object
		{
		return object!=0;
		}
	const Object* getObject(void) const
		{
		return object;
		}
	friend bool operator==(const PointerID& pi1,const PointerID& pi2)
		{
		return pi1.object==pi2.object;
		}
	friend bool operator!=(const PointerID& pi1,const PointerID& pi2)
		{
		return pi1.object!=pi2.object;
		}
	static size_t hash(const PointerID& pi,size_t tableSize)
		{
		return reinterpret_cast<size_t>(pi.object)%tableSize;
		}
	};

}

}

#endif
