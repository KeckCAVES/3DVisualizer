/***********************************************************************
SharedVisualizationPipe - Common interface between shared visualization
servers and clients.
Copyright (c) 2009 Oliver Kreylos

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

#include "SharedVisualizationPipe.h"

namespace Collaboration {

/************************************************
Static elements of class SharedVisualizationPipe:
************************************************/

const char* SharedVisualizationPipe::protocolName="SharedVisualizer";
const unsigned int SharedVisualizationPipe::protocolVersion=1*65536+0; // Version 1.0

}
