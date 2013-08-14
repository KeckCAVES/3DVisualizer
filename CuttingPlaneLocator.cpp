/***********************************************************************
CuttingPlaneLocator - Class for locators rendering cutting planes.
Copyright (c) 2006-2010 Oliver Kreylos

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

#include "CuttingPlaneLocator.h"

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>

#include "CuttingPlane.h"
#include "Visualizer.h"

/************************************
Methods of class CuttingPlaneLocator:
************************************/

CuttingPlaneLocator::CuttingPlaneLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication,const Misc::ConfigurationFileSection* cfg)
	:BaseLocator(sLocatorTool,sApplication),
	 cuttingPlane(0)
	{
	/* Find a cutting plane index for this locator: */
	for(size_t i=0;i<application->numCuttingPlanes;++i)
		if(!application->cuttingPlanes[i].allocated)
			{
			cuttingPlane=&application->cuttingPlanes[i];
			break;
			}
	
	/* Allocate the cutting plane: */
	if(cuttingPlane!=0)
		{
		cuttingPlane->allocated=true;
		cuttingPlane->active=false;
		}
	}

CuttingPlaneLocator::~CuttingPlaneLocator(void)
	{
	/* De-allocate the cutting plane: */
	if(cuttingPlane!=0)
		{
		cuttingPlane->active=false;
		cuttingPlane->allocated=false;
		}
	}

void CuttingPlaneLocator::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write the algorithm name: */
	configFileSection.storeString("./algorithm","Cutting Plane");
	}

void CuttingPlaneLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	if(cuttingPlane!=0&&cuttingPlane->active)
		{
		/* Update the cutting plane equation: */
		Vrui::Vector planeNormal=cbData->currentTransformation.transform(Vrui::Vector(0,1,0));
		Vrui::Point planePoint=cbData->currentTransformation.getOrigin();
		cuttingPlane->plane=CuttingPlane::Plane(planeNormal,planePoint);
		}
	}

void CuttingPlaneLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Activate the cutting plane: */
	if(cuttingPlane!=0)
		cuttingPlane->active=true;
	}

void CuttingPlaneLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Deactivate the cutting plane: */
	if(cuttingPlane!=0)
		cuttingPlane->active=false;
	}
