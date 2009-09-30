// Vertex shader for GPU-based triple-channel raycasting
// Copyright (c) 2009 Oliver Kreylos

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
	}
