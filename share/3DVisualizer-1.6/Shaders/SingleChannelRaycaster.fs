// Fragment shader for GPU-based single-channel raycasting
// Copyright (c) 2009 Oliver Kreylos

uniform vec3 mcScale;
uniform sampler2D depthSampler;
uniform mat4 depthMatrix;
uniform vec2 depthSize;
uniform vec3 eyePosition;
uniform float stepSize;
uniform sampler3D volumeSampler;
uniform sampler1D colorMapSampler;

varying vec3 mcPosition;
varying vec3 dcPosition;

void main()
	{
	/* Calculate the ray direction in model coordinates: */
	vec3 mcDir=mcPosition-eyePosition;
	
	/* Get the distance from the eye to the ray starting point: */
	float eyeDist=length(mcDir);
	
	/* Normalize and multiply the ray direction with the current step size: */
	mcDir=normalize(mcDir);
	mcDir*=stepSize;
	eyeDist/=stepSize;
	
	/* Get the fragment's ray termination depth from the depth texture: */
	float termDepth=2.0*texture2D(depthSampler,gl_FragCoord.xy/depthSize).x-1.0;
	
	/* Calculate the maximum number of steps based on the termination depth: */
	vec4 cc1=depthMatrix*vec4(mcPosition,1.0);
	vec4 cc2=depthMatrix*vec4(mcDir,0.0);
	float lambdaMax=-(termDepth*cc1.w-cc1.z)/(termDepth*cc2.w-cc2.z);
	
	/* Convert the ray direction to data coordinates: */
	vec3 dcDir=mcDir*mcScale;
	
	/* Cast the ray and accumulate opacities and colors: */
	vec4 accum=vec4(0.0,0.0,0.0,0.0);
	
	/* Move the ray starting position forward to an integer multiple of the step size: */
	vec3 samplePos=dcPosition;
	float lambda=ceil(eyeDist)-eyeDist;
	if(lambda<lambdaMax)
		{
		samplePos+=dcDir*lambda;
		for(int i=0;i<1500;++i)
			{
			/* Get the volume data value at the current sample position: */
			vec4 vol=texture1D(colorMapSampler,texture3D(volumeSampler,samplePos).a);
			
			/* Accumulate color and opacity: */
			accum+=vol*(1.0-accum.a);
			
			/* Bail out when opacity hits 1.0: */
			if(accum.a>=1.0-1.0/256.0||lambda>=lambdaMax)
				break;
			
			/* Advance the sample position: */
			samplePos+=dcDir;
			lambda+=1.0;
			}
		}
	
	/* Assign the final color value: */
	gl_FragColor=accum;
	}
