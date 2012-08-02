/***********************************************************************
EarthRenderer - Class to render a configurable model of Earth using
transparent surfaces and several interior components.
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

#include <Concrete/EarthRenderer.h>

#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <Images/Config.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>

#include <GLRenderState.h>

namespace Visualization {

namespace Concrete {

/****************************************
Methods of class EarthRenderer::DataItem:
****************************************/

EarthRenderer::DataItem::DataItem(void)
	:surfaceTextureObjectId(0),
	 displayListIdBase(0),
	 surfaceVersion(0),gridVersion(0),outerCoreVersion(0),innerCoreVersion(0)
	{
	/* Generate a texture object for the Earth's surface texture: */
	glGenTextures(1,&surfaceTextureObjectId);
	
	/* Generate display lists for the Earth model components: */
	displayListIdBase=glGenLists(4);
	}

EarthRenderer::DataItem::~DataItem(void)
	{
	/* Delete the Earth surface texture object: */
	glDeleteTextures(1,&surfaceTextureObjectId);
	
	/* Delete the Earth model components display lists: */
	glDeleteLists(displayListIdBase,4);
	}

/**************************************
Static elements of class EarthRenderer:
**************************************/

const double EarthRenderer::a=6378.14e3;
const double EarthRenderer::flatteningFactor=1.0/298.257;

/******************************
Methods of class EarthRenderer:
******************************/

void EarthRenderer::renderSurface(EarthRenderer::DataItem* dataItem) const
	{
	/* Check if the Earth surface display list is up-to-date: */
	if(dataItem->surfaceVersion==surfaceVersion)
		{
		/* Call the Earth surface display list: */
		glCallList(dataItem->displayListIdBase+0);
		}
	else
		{
		/* Update the Earth surface display list: */
		glNewList(dataItem->displayListIdBase+0,GL_COMPILE_AND_EXECUTE);
		
		const double pi=Math::Constants<double>::pi;
		const int baseNumStrips=18; // Number of circles of constant latitude for lowest-detail model
		const int baseNumQuads=36; // Number of meridians for lowest-detail model
		
		int numStrips=baseNumStrips*surfaceDetail;
		int numQuads=baseNumQuads*surfaceDetail;
		
		float texY1=float(0)/float(numStrips);
		double lat1=(pi*double(0))/double(numStrips)-0.5*pi;
		double s1=Math::sin(lat1);
		double c1=Math::cos(lat1);
		double r1=a*(1.0-f*s1*s1)*scaleFactor;
		double xy1=r1*c1;
		double z1=r1*s1;
		
		/* Draw latitude quad strips: */
		for(int i=1;i<numStrips+1;++i)
			{
			float texY0=texY1;
			double s0=s1;
			double c0=c1;
			double xy0=xy1;
			double z0=z1;
			texY1=float(i)/float(numStrips);
			lat1=(pi*double(i))/double(numStrips)-0.5*pi;
			s1=Math::sin(lat1);
			c1=Math::cos(lat1);
			r1=a*(1.0-f*s1*s1)*scaleFactor;
			xy1=r1*c1;
			z1=r1*s1;
			
			glBegin(GL_QUAD_STRIP);
			for(int j=0;j<=numQuads;++j)
				{
				float texX=float(j)/float(numQuads)+0.5f;
				glTexCoord2f(texX,texY1);
				double lng=(2.0*pi*double(j))/double(numQuads);
				double cl=Math::cos(lng);
				double sl=Math::sin(lng);
				double nx1=(1.0-3.0*f*s1*s1)*c1*cl;
				double ny1=(1.0-3.0*f*s1*s1)*c1*sl;
				double nz1=(1.0+3.0*f*c1*c1-f)*s1;
				double nl1=Math::sqrt(nx1*nx1+ny1*ny1+nz1*nz1);
				glNormal3f(float(nx1/nl1),float(ny1/nl1),float(nz1/nl1));
				double x1=xy1*cl;
				double y1=xy1*sl;
				glVertex3f(float(x1),float(y1),float(z1));
				glTexCoord2f(texX,texY0);
				double nx0=(1.0-3.0*f*s0*s0)*c0*cl;
				double ny0=(1.0-3.0*f*s0*s0)*c0*sl;
				double nz0=(1.0+3.0*f*c0*c0-f)*s0;
				double nl0=Math::sqrt(nx0*nx0+ny0*ny0+nz0*nz0);
				glNormal3f(float(nx0/nl0),float(ny0/nl0),float(nz0/nl0));
				double x0=xy0*cl;
				double y0=xy0*sl;
				glVertex3f(float(x0),float(y0),float(z0));
				}
			glEnd();
			}
		
		glEndList();
		dataItem->surfaceVersion=surfaceVersion;
		}
	}

void EarthRenderer::renderGrid(EarthRenderer::DataItem* dataItem) const
	{
	/* Check if the latitude/longitude grid display list is up-to-date: */
	if(dataItem->gridVersion==gridVersion)
		{
		/* Call the latitude/longitude grid display list: */
		glCallList(dataItem->displayListIdBase+1);
		}
	else
		{
		/* Create the latitude/longitude grid display list: */
		glNewList(dataItem->displayListIdBase+1,GL_COMPILE_AND_EXECUTE);
		
		const double pi=Math::Constants<double>::pi;
		const int baseNumStrips=18; // Number of circles of constant latitude for lowest-detail model
		const int baseNumQuads=36; // Number of meridians for lowest-detail model
		
		int numStrips=baseNumStrips*gridDetail;
		int numQuads=baseNumQuads*gridDetail;
		
		/* Draw circles of constant latitude (what are they called?): */
		for(int i=1;i<baseNumStrips;++i)
			{
			double lat=(pi*double(i))/double(baseNumStrips)-0.5*pi;
			double s=Math::sin(lat);
			double c=Math::cos(lat);
			double r=a*(1.0-f*s*s)*scaleFactor;
			double xy=r*c;
			double z=r*s;
			
			glBegin(GL_LINE_LOOP);
			for(int j=0;j<numQuads;++j)
				{
				double lng=(2.0*pi*double(j))/double(numQuads);
				double cl=Math::cos(lng);
				double sl=Math::sin(lng);
				double x=xy*cl;
				double y=xy*sl;
				glVertex3f(float(x),float(y),float(z));
				}
			glEnd();
			}
		
		/* Draw meridians: */
		for(int i=0;i<baseNumQuads;++i)
			{
			double lng=(2.0*pi*double(i))/double(baseNumQuads);
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			
			glBegin(GL_LINE_STRIP);
			glVertex3f(0.0f,0.0f,-a*(1.0-f)*scaleFactor);
			for(int j=1;j<numStrips;++j)
				{
				double lat=(pi*double(j))/double(numStrips)-0.5*pi;
				double s=Math::sin(lat);
				double c=Math::cos(lat);
				double r=a*(1.0-f*s*s)*scaleFactor;
				double xy=r*c;
				double x=xy*cl;
				double y=xy*sl;
				double z=r*s;
				glVertex3f(float(x),float(y),float(z));
				}
			glVertex3f(0.0f,0.0f,a*(1.0-f)*scaleFactor);
			glEnd();
			}
		
		glEndList();
		dataItem->gridVersion=gridVersion;
		}
	}

void EarthRenderer::renderOuterCore(EarthRenderer::DataItem* dataItem) const
	{
	/* Check if the outer core display list is up-to-date: */
	if(dataItem->outerCoreVersion==outerCoreVersion)
		{
		/* Call the outer core display list: */
		glCallList(dataItem->displayListIdBase+2);
		}
	else
		{
		/* Create the outer core display list: */
		glNewList(dataItem->displayListIdBase+2,GL_COMPILE_AND_EXECUTE);
		glDrawSphereIcosahedron(float(3480.0e3*scaleFactor),outerCoreDetail);
		glEndList();
		dataItem->outerCoreVersion=outerCoreVersion;
		}
	}

void EarthRenderer::renderInnerCore(EarthRenderer::DataItem* dataItem) const
	{
	/* Check if the inner core display list is up-to-date: */
	if(dataItem->innerCoreVersion==innerCoreVersion)
		{
		/* Call the inner core display list: */
		glCallList(dataItem->displayListIdBase+3);
		}
	else
		{
		/* Create the inner core display list: */
		glNewList(dataItem->displayListIdBase+3,GL_COMPILE_AND_EXECUTE);
		glDrawSphereIcosahedron(float(1221.0e3*scaleFactor),innerCoreDetail);
		glEndList();
		dataItem->innerCoreVersion=innerCoreVersion;
		}
	}

EarthRenderer::EarthRenderer(double sScaleFactor)
	:scaleFactor(sScaleFactor),
	 f(flatteningFactor),
	 surfaceDetail(2),
	 surfaceMaterial(GLMaterial::Color(1.0f,1.0f,1.0f,0.333f),GLMaterial::Color(0.333f,0.333f,0.333f),10.0f),
	 surfaceOpacity(0.333f),
	 surfaceVersion(1),
	 gridDetail(10),
	 gridLineWidth(1.0f),
	 gridColor(0.0f,1.0f,0.0f,0.1f),
	 gridOpacity(0.1f),
	 gridVersion(1),
	 outerCoreDetail(8),
	 outerCoreMaterial(GLMaterial::Color(1.0f,0.5f,0.0f,0.333f),GLMaterial::Color(1.0f,1.0f,1.0f),50.0f),
	 outerCoreOpacity(0.333f),
	 outerCoreVersion(1),
	 innerCoreDetail(8),
	 innerCoreMaterial(GLMaterial::Color(1.0f,0.0f,0.0f,0.333f),GLMaterial::Color(1.0f,1.0f,1.0f),50.0f),
	 innerCoreOpacity(0.333f),
	 innerCoreVersion(1)
	{
	}

void EarthRenderer::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem();
	contextData.addDataItem(this,dataItem);
	
	/* Create the default topography file name: */
	std::string topographyFileName=EARTHRENDERER_IMAGEDIR;
	#if IMAGES_CONFIG_HAVE_PNG
	topographyFileName.append("/EarthTopography.png");
	#else
	topographyFileName.append("/EarthTopography.ppm");
	#endif
	
	/* Load the Earth surface texture image from an image file: */
	Images::RGBImage earthTexture=Images::readImageFile(topographyFileName.c_str());
	
	/* Select the Earth surface texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->surfaceTextureObjectId);
	
	/* Upload the Earth surface texture image: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	earthTexture.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB);
	
	/* Protect the Earth surface texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

void EarthRenderer::setScaleFactor(double newScaleFactor)
	{
	scaleFactor=newScaleFactor;
	++surfaceVersion;
	++gridVersion;
	++outerCoreVersion;
	++innerCoreVersion;
	}

void EarthRenderer::setFlatteningFactor(double newF)
	{
	f=newF;
	++surfaceVersion;
	++gridVersion;
	}

void EarthRenderer::setSurfaceDetail(int newSurfaceDetail)
	{
	surfaceDetail=newSurfaceDetail;
	++surfaceVersion;
	}

void EarthRenderer::setSurfaceMaterial(const GLMaterial& newSurfaceMaterial)
	{
	surfaceMaterial=newSurfaceMaterial;
	surfaceMaterial.diffuse[3]=surfaceOpacity;
	}

void EarthRenderer::setSurfaceOpacity(float newSurfaceOpacity)
	{
	surfaceOpacity=newSurfaceOpacity;
	surfaceMaterial.diffuse[3]=surfaceOpacity;
	}

void EarthRenderer::setGridDetail(int newGridDetail)
	{
	gridDetail=newGridDetail;
	++gridVersion;
	}

void EarthRenderer::setGridLineWidth(float newGridLineWidth)
	{
	gridLineWidth=newGridLineWidth;
	}

void EarthRenderer::setGridColor(const EarthRenderer::Color& newGridColor)
	{
	gridColor=newGridColor;
	gridColor[3]=gridOpacity;
	}

void EarthRenderer::setGridOpacity(float newGridOpacity)
	{
	gridOpacity=newGridOpacity;
	gridColor[3]=gridOpacity;
	}

void EarthRenderer::setOuterCoreDetail(int newOuterCoreDetail)
	{
	outerCoreDetail=newOuterCoreDetail;
	++outerCoreVersion;
	}

void EarthRenderer::setOuterCoreMaterial(const GLMaterial& newOuterCoreMaterial)
	{
	outerCoreMaterial=newOuterCoreMaterial;
	outerCoreMaterial.diffuse[3]=outerCoreOpacity;
	}

void EarthRenderer::setOuterCoreOpacity(float newOuterCoreOpacity)
	{
	outerCoreOpacity=newOuterCoreOpacity;
	outerCoreMaterial.diffuse[3]=outerCoreOpacity;
	}

void EarthRenderer::setInnerCoreDetail(int newInnerCoreDetail)
	{
	innerCoreDetail=newInnerCoreDetail;
	++innerCoreVersion;
	}

void EarthRenderer::setInnerCoreMaterial(const GLMaterial& newInnerCoreMaterial)
	{
	innerCoreMaterial=newInnerCoreMaterial;
	innerCoreMaterial.diffuse[3]=innerCoreOpacity;
	}

void EarthRenderer::setInnerCoreOpacity(float newInnerCoreOpacity)
	{
	innerCoreOpacity=newInnerCoreOpacity;
	innerCoreMaterial.diffuse[3]=innerCoreOpacity;
	}

void EarthRenderer::glRenderAction(GLRenderState& renderState) const
	{
	/* Get context data item: */
	DataItem* dataItem=renderState.getContextData().retrieveDataItem<DataItem>(this);
	
	/* Save OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	/* Render all opaque surfaces: */
	renderState.disableCulling();
	if(surfaceOpacity>0.0f)
		{
		/* Reset the texture matrix: */
		renderState.setMatrixMode(2);
		glLoadIdentity();
		renderState.updateMatrix();
		}
	if(surfaceOpacity==1.0f)
		{
		/* Set up OpenGL to render the Earth's surface: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,surfaceMaterial);
		renderState.setTextureLevel(2);
		renderState.bindTexture(dataItem->surfaceTextureObjectId);
		renderState.setTextureMode(GL_MODULATE);
		renderState.setSeparateSpecularColor(true);
		
		/* Render the Earth's surface: */
		renderSurface(dataItem);
		}
	if(outerCoreOpacity==1.0f)
		{
		/* Set up OpenGL to render the outer core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,outerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		
		/* Render the outer core: */
		renderOuterCore(dataItem);
		}
	if(innerCoreOpacity==1.0f)
		{
		/* Set up OpenGL to render the inner core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,innerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		
		/* Render the inner core: */
		renderInnerCore(dataItem);
		}
	
	/* Render transparent surfaces in back-to-front order: */
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	
	/* Render back parts of surfaces: */
	renderState.enableCulling(GL_FRONT);
	if(surfaceOpacity>0.0f&&surfaceOpacity<1.0f)
		{
		/* Set up OpenGL to render the Earth's surface: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,surfaceMaterial);
		renderState.setTextureLevel(2);
		renderState.bindTexture(dataItem->surfaceTextureObjectId);
		renderState.setTextureMode(GL_MODULATE);
		renderState.setSeparateSpecularColor(true);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the Earth's surface: */
		renderSurface(dataItem);
		}
	if(gridOpacity>0.0f&&gridOpacity<1.0f)
		{
		renderState.setLineWidth(1.0f);
		renderState.setLighting(false);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glColor(gridColor);
		
		/* Render the latitude/longitude grid: */
		renderGrid(dataItem);
		}
	if(outerCoreOpacity>0.0f&&outerCoreOpacity<1.0f)
		{
		/* Set up OpenGL to render the outer core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,outerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the outer core: */
		renderOuterCore(dataItem);
		}
	if(innerCoreOpacity>0.0f&&innerCoreOpacity<1.0f)
		{
		/* Set up OpenGL to render the inner core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,innerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the inner core: */
		renderInnerCore(dataItem);
		}
	
	/* Render front parts of surfaces: */
	renderState.enableCulling(GL_BACK);
	if(innerCoreOpacity>0.0f&&innerCoreOpacity<1.0f)
		{
		/* Set up OpenGL to render the inner core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,innerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the inner core: */
		renderInnerCore(dataItem);
		}
	if(outerCoreOpacity>0.0f&&outerCoreOpacity<1.0f)
		{
		/* Set up OpenGL to render the outer core: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,outerCoreMaterial);
		renderState.setTextureLevel(0);
		renderState.setSeparateSpecularColor(false);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the outer core: */
		renderOuterCore(dataItem);
		}
	if(surfaceOpacity>0.0f&&surfaceOpacity<1.0f)
		{
		/* Set up OpenGL to render the Earth's surface: */
		renderState.setLighting(true);
		renderState.setTwoSidedLighting(true);
		renderState.disableColorMaterial();
		glMaterial(GLMaterialEnums::FRONT_AND_BACK,surfaceMaterial);
		renderState.setTextureLevel(2);
		renderState.bindTexture(dataItem->surfaceTextureObjectId);
		renderState.setTextureMode(GL_MODULATE);
		renderState.setSeparateSpecularColor(true);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		/* Render the Earth's surface: */
		renderSurface(dataItem);
		}
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

}

}
