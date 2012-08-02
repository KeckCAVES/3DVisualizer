/***********************************************************************
PointSet - Class to represent and render sets of scattered 3D points.
Copyright (c) 2005-2012 Oliver Kreylos

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

#include <Concrete/PointSet.h>

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

namespace Visualization {

namespace Concrete {

namespace {

/*********************************************************
Helper function to parse spreadsheet files in text format:
*********************************************************/

int
getNextValue(
	Misc::File& file,
	char* valueBuffer,
	size_t valueBufferSize)
	{
	/* Skip all whitespace: */
	int nextChar;
	do
		{
		nextChar=file.getc();
		}
	while(isspace(nextChar)&&nextChar!='\n'&&nextChar!=EOF);
	
	/* Check for end-of-line or end-of-file condition: */
	if(nextChar=='\n'||nextChar==EOF)
		return nextChar;
	
	/* Read the value: */
	char* vPtr=valueBuffer;
	if(nextChar=='"')
		{
		/* Read characters until next quotation mark: */
		while(true)
			{
			nextChar=file.getc();
			if(nextChar!='"')
				{
				if(valueBufferSize>1)
					{
					*vPtr=char(nextChar);
					++vPtr;
					--valueBufferSize;
					}
				}
			else
				break;
			}
		}
	else
		{
		/* Append the first character to the result string: */
		if(valueBufferSize>1)
			{
			*vPtr=char(nextChar);
			++vPtr;
			--valueBufferSize;
			}
		
		/* Read characters until next whitespace: */
		while(true)
			{
			nextChar=file.getc();
			if(!isspace(nextChar))
				{
				if(valueBufferSize>1)
					{
					*vPtr=char(nextChar);
					++vPtr;
					--valueBufferSize;
					}
				}
			else
				{
				file.ungetc(nextChar);
				break;
				}
			}
		}
	*vPtr='\0';
	
	return 0;
	}

/**********************************************************************
Helper functions to convert spherical (geoid) to Cartesian coordinates:
**********************************************************************/

inline
void
calcRadiusPos(
	float latitude,
	float longitude,
	float radius,
	double scaleFactor,
	float pos[3])
	{
	double s0=Math::sin(double(latitude));
	double c0=Math::cos(double(latitude));
	double r=radius*scaleFactor;
	double xy=r*c0;
	double s1=Math::sin(double(longitude));
	double c1=Math::cos(double(longitude));
	pos[0]=float(xy*c1);
	pos[1]=float(xy*s1);
	pos[2]=float(r*s0);
	}

inline
void
calcDepthPos(
	float latitude,
	float longitude,
	float depth,
	double flatteningFactor,
	double scaleFactor,
	float pos[3])
	{
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	
	double s0=Math::sin(double(latitude));
	double c0=Math::cos(double(latitude));
	double r=(a*(1.0-flatteningFactor*Math::sqr(s0))-depth)*scaleFactor;
	double xy=r*c0;
	double s1=Math::sin(double(longitude));
	double c1=Math::cos(double(longitude));
	pos[0]=float(xy*c1);
	pos[1]=float(xy*s1);
	pos[2]=float(r*s0);
	}

/**********************************************
Helper classes to upload and render point sets:
**********************************************/

template <class VertexParam>
class PointChunkUploader
	{
	/* Elements: */
	private:
	GLintptrARB offset; // Current offset into the vertex buffer object
	
	/* Constructors and destructors: */
	public:
	PointChunkUploader(void)
		:offset(0)
		{
		}
	
	/* Methods: */
	void operator()(const VertexParam* chunkVertices,size_t numChunkVertices)
		{
		/* Upload the current chunk of vertices: */
		GLsizeiptrARB size=numChunkVertices*sizeof(VertexParam);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,offset,size,chunkVertices);
		
		/* Prepare to upload the next chunk: */
		offset+=size;
		}
	};

template <class VertexParam>
class PointChunkRenderer
	{
	/* Methods: */
	public:
	void operator()(const VertexParam* chunkVertices,size_t numChunkVertices)
		{
		/* Render the current chunk of vertices: */
		glVertexPointer(chunkVertices);
		glDrawArrays(GL_POINTS,0,numChunkVertices);
		}
	};

}

/***********************************
Methods of class PointSet::DataItem:
***********************************/

PointSet::DataItem::DataItem(void)
	:vertexBufferObjectId(0)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		}
	}

PointSet::DataItem::~DataItem(void)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(vertexBufferObjectId!=0)
		{
		/* Destroy the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		}
	}

/*************************
Methods of class PointSet:
*************************/

PointSet::PointSet(const char* pointFileName,double flatteningFactor,double scaleFactor)
	{
	/* Open the point file: */
	Misc::File pointFile(pointFileName,"rt");
	
	/* Read the header line from the point file: */
	int latIndex=-1;
	int lngIndex=-1;
	int radiusIndex=-1;
	enum RadiusMode
		{
		RADIUS,DEPTH,NEGDEPTH
		} radiusMode=RADIUS;
	int index=0;
	while(true)
		{
		char valueBuffer[40];
		int terminator=getNextValue(pointFile,valueBuffer,sizeof(valueBuffer));
		if(terminator=='\n')
			break;
		else if(terminator==EOF)
			Misc::throwStdErr("PointSet::PointSet: Early end of file in input file \"%s\"",pointFileName);
		else if(strcasecmp(valueBuffer,"Latitude")==0||strcasecmp(valueBuffer,"Lat")==0)
			latIndex=index;
		else if(strcasecmp(valueBuffer,"Longitude")==0||strcasecmp(valueBuffer,"Long")==0||strcasecmp(valueBuffer,"Lon")==0)
			lngIndex=index;
		else if(strcasecmp(valueBuffer,"Radius")==0)
			{
			radiusIndex=index;
			radiusMode=RADIUS;
			}
		else if(strcasecmp(valueBuffer,"Depth")==0)
			{
			radiusIndex=index;
			radiusMode=DEPTH;
			}
		else if(strcasecmp(valueBuffer,"Negative Depth")==0||strcasecmp(valueBuffer,"Neg Depth")==0||strcasecmp(valueBuffer,"NegDepth")==0)
			{
			radiusIndex=index;
			radiusMode=NEGDEPTH;
			}
		++index;
		}
	
	/* Check if all required portions have been detected: */
	if(latIndex<0||lngIndex<0||radiusIndex<0)
		Misc::throwStdErr("PointSet::PointSet: Missing point components in input file \"%s\"",pointFileName);
	
	/* Read all point positions from the point file: */
	bool finished=false;
	while(!finished)
		{
		/* Read the next line from the input file: */
		int index=0;
		float sphericalCoordinates[3]={0.0f,0.0f,0.0f}; // Initialization just to shut up gcc
		int parsedComponentsMask=0x0;
		while(true)
			{
			char valueBuffer[40];
			int terminator=getNextValue(pointFile,valueBuffer,sizeof(valueBuffer));
			if(terminator=='\n')
				break;
			else if(terminator==EOF)
				{
				finished=true;
				break;
				}
			else if(index==latIndex)
				{
				sphericalCoordinates[0]=Math::rad(float(atof(valueBuffer)));
				parsedComponentsMask|=0x1;
				}
			else if(index==lngIndex)
				{
				sphericalCoordinates[1]=Math::rad(float(atof(valueBuffer)));
				parsedComponentsMask|=0x2;
				}
			else if(index==radiusIndex)
				{
				sphericalCoordinates[2]=float(atof(valueBuffer));
				parsedComponentsMask|=0x4;
				}
			++index;
			}
		
		/* Check if a complete set of coordinates has been parsed: */
		if(parsedComponentsMask==0x7&&!isnan(sphericalCoordinates[2]))
			{
			/* Convert the read spherical coordinates to Cartesian coordinates: */
			Vertex p;
			p.position=Vertex::Position(0,0,0); // To shut up gcc
			switch(radiusMode)
				{
				case RADIUS:
					calcRadiusPos(sphericalCoordinates[0],sphericalCoordinates[1],sphericalCoordinates[2]*1000.0f,scaleFactor,p.position.getXyzw());
					break;
				
				case DEPTH:
					calcDepthPos(sphericalCoordinates[0],sphericalCoordinates[1],sphericalCoordinates[2]*1000.0f,flatteningFactor,scaleFactor,p.position.getXyzw());
					break;
				
				case NEGDEPTH:
					calcDepthPos(sphericalCoordinates[0],sphericalCoordinates[1],-sphericalCoordinates[2]*1000.0f,flatteningFactor,scaleFactor,p.position.getXyzw());
					break;
				}
			
			/* Append the point to the point set: */
			points.push_back(p);
			}
		}
	std::cout<<points.size()<<" points parsed from "<<pointFileName<<std::endl;
	}

PointSet::~PointSet(void)
	{
	/* Nothing to do, incidentally */
	}

void PointSet::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Check if the vertex buffer object extension is supported: */
	if(dataItem->vertexBufferObjectId>0)
		{
		/* Create a vertex buffer object to store the points' coordinates: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,points.size()*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		
		/* Copy all points: */
		PointChunkUploader<Vertex> pcu;
		points.forEachChunk(pcu);
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	}

void PointSet::glRenderAction(GLContextData& contextData) const
	{
	/* Get a pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	
	/* Check if the vertex buffer object extension is supported: */
	if(dataItem->vertexBufferObjectId!=0)
		{
		/* Bind the point set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		
		/* Render the point set as a vertex buffer of points: */
		glVertexPointer(static_cast<const Vertex*>(0));
		glDrawArrays(GL_POINTS,0,points.size());
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* Render the point set as a set of regular vertex arrays of points: */
		PointChunkRenderer<Vertex> pcr;
		points.forEachChunk(pcr);
		}
	
	/* Restore OpenGL state: */
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
