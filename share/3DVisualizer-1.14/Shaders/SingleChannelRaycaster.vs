/***********************************************************************
Vertex shader for GPU-based single-channel raycasting
Copyright (c) 2009-2010 Oliver Kreylos

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

uniform vec3 mcScale;
uniform vec3 mcOffset;

varying vec3 mcPosition;
varying vec3 dcPosition;

void main()
	{
	/* Store the model-coordinate vertex position: */
	mcPosition=gl_Vertex.xyz/gl_Vertex.w;
	
	/* Store the data-coordinate vertex position: */
	dcPosition=mcPosition*mcScale+mcOffset;
	
	/* Calculate the clip-coordinate vertex position: */
	gl_Position=ftransform();
	
	/* Ensure that no vertices are in front of the front plane, which can happen due to rounding: */
	if(gl_Position[2]<-gl_Position[3])
		gl_Position[2]=-gl_Position[3];
	}
