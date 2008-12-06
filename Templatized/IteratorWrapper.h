/***********************************************************************
IteratorWrapper - Helper class to wrap classes that already provide
iterator functionality with a proper iterator interface.
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

#ifndef VISUALIZATION_TEMPLATIZED_ITERATORWRAPPER_INCLUDED
#define VISUALIZATION_TEMPLATIZED_ITERATORWRAPPER_INCLUDED

namespace Visualization {

namespace Templatized {

template <class BaseClassParam>
class IteratorWrapper:public BaseClassParam
	{
	/* Constructors and destructors: */
	public:
	IteratorWrapper(void)
		{
		}
	IteratorWrapper(const BaseClassParam& source)
		:BaseClassParam(source)
		{
		}
	
	/* Iterator methods: */
	const BaseClassParam& operator*(void) const
		{
		return *this;
		}
	BaseClassParam& operator*(void)
		{
		return *this;
		}
	const BaseClassParam* operator->(void) const
		{
		return this;
		}
	BaseClassParam* operator->(void)
		{
		return this;
		}
	};

}

}

#endif
