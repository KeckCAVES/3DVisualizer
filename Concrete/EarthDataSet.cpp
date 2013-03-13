/***********************************************************************
EarthDataSet - Wrapper class to add an Earth renderer to an arbitrary
visualization module working on whole-Earth grids.
Copyright (c) 2007 Oliver Kreylos

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

#define VISUALIZATION_CONCRETE_EARTHDATASET_IMPLEMENTATION

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <GL/GLColorTemplates.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/PointSet.h>

#include <Concrete/EarthDataSet.h>

namespace Visualization {

namespace Concrete {

/*****************************
Methods of class EarthDataSet:
*****************************/

template <class DataSetBaseParam>
inline
EarthDataSet<DataSetBaseParam>::EarthDataSet(
	const std::vector<std::string>& args)
	:flatteningFactor(EarthRenderer::getFlatteningFactor()),
	 coordinateTransformer(new SphericalCoordinateTransformer)
	{
	/* Parse the arguments: */
	bool havePoints=false;
	for(std::vector<std::string>::const_iterator aIt=args.begin();aIt!=args.end();++aIt)
		{
		if(strcasecmp(aIt->c_str(),"-points")==0)
			havePoints=true;
		else if(havePoints)
			pointSetFileNames.push_back(*aIt);
		}
	
	/* Initialize the coordinate transformer: */
	coordinateTransformer->setRadius(EarthRenderer::getRadius()*1.0e-3);
	coordinateTransformer->setFlatteningFactor(flatteningFactor);
	}

template <class DataSetBaseParam>
inline
EarthDataSet<DataSetBaseParam>::~EarthDataSet(
	void)
	{
	delete coordinateTransformer;
	}

template <class DataSetBaseParam>
inline
void
EarthDataSet<DataSetBaseParam>::setFlatteningFactor(
	double newFlatteningFactor)
		{
		flatteningFactor=newFlatteningFactor;
		coordinateTransformer->setFlatteningFactor(flatteningFactor);
		}

template <class DataSetBaseParam>
inline
Visualization::Abstract::CoordinateTransformer*
EarthDataSet<DataSetBaseParam>::getCoordinateTransformer(
	void) const
	{
	return coordinateTransformer->clone();
	}

/*********************************************
Static elements of class EarthDataSetRenderer:
*********************************************/

template <class DataSetBaseParam,class DataSetRendererBaseParam>
const GLColor<GLfloat,3>
EarthDataSetRenderer<DataSetBaseParam,DataSetRendererBaseParam>::pointSetColors[]=
	{
	GLColor<GLfloat,3>(1.0f,0.0f,0.0f),GLColor<GLfloat,3>(1.0f,1.0f,0.0f),
	GLColor<GLfloat,3>(0.0f,1.0f,0.0f),GLColor<GLfloat,3>(0.0f,1.0f,1.0f),
	GLColor<GLfloat,3>(0.0f,0.0f,1.0f),GLColor<GLfloat,3>(1.0f,0.0f,1.0f)
	};

/*************************************
Methods of class EarthDataSetRenderer:
*************************************/

template <class DataSetBaseParam,class DataSetRendererBaseParam>
inline
EarthDataSetRenderer<DataSetBaseParam,DataSetRendererBaseParam>::EarthDataSetRenderer(
	const Visualization::Abstract::DataSet* sDataSet)
	:DataSetRendererBase(sDataSet),
	 er(1.0e-3),
	 drawEarthModel(false)
	{
	/* Initialize the Earth renderer: */
	const EarthDataSet* eds=dynamic_cast<const EarthDataSet*>(sDataSet);
	er.setFlatteningFactor(eds->getFlatteningFactor());
	er.setSurfaceOpacity(0.5f);
	er.setOuterCoreOpacity(0.5f);
	er.setInnerCoreOpacity(0.0f);
	
	/* Load all point sets listed in the Earth data set: */
	for(std::vector<std::string>::const_iterator psfnIt=eds->getPointSetFileNames().begin();psfnIt!=eds->getPointSetFileNames().end();++psfnIt)
		{
		PointSet* ps=new PointSet(psfnIt->c_str(),eds->getFlatteningFactor(),1.0e-3);
		pointSets.push_back(ps);
		}
	}

template <class DataSetBaseParam,class DataSetRendererBaseParam>
inline
EarthDataSetRenderer<DataSetBaseParam,DataSetRendererBaseParam>::~EarthDataSetRenderer(
	void)
	{
	/* Delete all point sets: */
	for(std::vector<PointSet*>::iterator psIt=pointSets.begin();psIt!=pointSets.end();++psIt)
		delete *psIt;
	}

template <class DataSetBaseParam,class DataSetRendererBaseParam>
inline
void
EarthDataSetRenderer<DataSetBaseParam,DataSetRendererBaseParam>::glRenderAction(
	GLContextData& contextData) const
	{
	if(!pointSets.empty())
		{
		/* Save and set up OpenGL state: */
		GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
		if(lightingEnabled)
			glDisable(GL_LIGHTING);
		GLfloat pointSize;
		glGetFloatv(GL_POINT_SIZE,&pointSize);
		glPointSize(1.0f);
		GLfloat renderColor[4];
		glGetFloatv(GL_CURRENT_COLOR,renderColor);
		
		/* Draw all point sets: */
		int index=0;
		for(std::vector<PointSet*>::const_iterator psIt=pointSets.begin();psIt!=pointSets.end();++psIt,++index)
			{
			glColor(pointSetColors[index%numPointSetColors]);
			(*psIt)->glRenderAction(contextData);
			}
		
		/* Restore OpenGL state: */
		glColor4fv(renderColor);
		glPointSize(pointSize);
		if(lightingEnabled)
			glEnable(GL_LIGHTING);
		}
	
	/* Draw the model itself: */
	if(drawEarthModel)
		er.glRenderAction(contextData);
	else
		DataSetRendererBase::glRenderAction(contextData);
	}

}

}
