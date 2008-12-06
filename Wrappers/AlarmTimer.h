/***********************************************************************
AlarmTimer - Wrapper class to use an alarm timer as a continuation
functor for incremental visualization algorithms.
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

#ifndef VISUALIZATION_WRAPPERS_ALARMTIMER_INCLUDED
#define VISUALIZATION_WRAPPERS_ALARMTIMER_INCLUDED

#include <Realtime/AlarmTimer.h>

namespace Visualization {

namespace Wrappers {

class AlarmTimer
	{
	/* Elements: */
	private:
	const Realtime::AlarmTimer& alarm; // The queried alarm timer
	
	/* Constructors and destructors: */
	public:
	AlarmTimer(const Realtime::AlarmTimer& sAlarm)
		:alarm(sAlarm)
		{
		}
	
	/* Methods: */
	bool operator()(void) const
		{
		return !alarm.isExpired();
		}
	};

}

}

#endif
