/***********************************************************************
GlobalIsosurfaceExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized isosurface extractor
implementation.
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

#define VISUALIZATION_WRAPPERS_GLOBALISOSURFACEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <GL/GLColorMap.h>

#include <Templatized/IsosurfaceExtractorIndexedTriangleSet.h>
#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/Isosurface.h>
#include <Wrappers/AlarmTimer.h>

#include <Wrappers/GlobalIsosurfaceExtractor.h>

namespace Visualization {

namespace Wrappers {

/******************************************
Methods of class GlobalIsosurfaceExtractor:
******************************************/

template <class DataSetWrapperParam>
inline
const typename GlobalIsosurfaceExtractor<DataSetWrapperParam>::DS*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::GlobalIsosurfaceExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename GlobalIsosurfaceExtractor<DataSetWrapperParam>::SE&
GlobalIsosurfaceExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("GlobalIsosurfaceExtractor::GlobalIsosurfaceExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
GlobalIsosurfaceExtractor<DataSetWrapperParam>::GlobalIsosurfaceExtractor(
	const GLColorMap* sColorMap,
	const Visualization::Abstract::DataSet* sDataSet,
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	:colorMap(sColorMap),
	 ise(getDs(sDataSet),getSe(sScalarExtractor)),
	 isovalue(0)
	{
	}

template <class DataSetWrapperParam>
inline
GlobalIsosurfaceExtractor<DataSetWrapperParam>::~GlobalIsosurfaceExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
GlobalIsosurfaceExtractor<DataSetWrapperParam>::setIsovalue(
	typename GlobalIsosurfaceExtractor<DataSetWrapperParam>::VScalar newIsovalue) const
	{
	isovalue=newIsovalue;
	}

template <class DataSetWrapperParam>
inline
bool
GlobalIsosurfaceExtractor<DataSetWrapperParam>::hasGlobalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
GlobalIsosurfaceExtractor<DataSetWrapperParam>::createElement(
	void)
	{
	/* Create a new isosurface visualization element: */
	Isosurface* result=new Isosurface(colorMap,isovalue);
	
	/* Extract the isosurface into the visualization element: */
	ise.extractIsosurface(isovalue,result->getSurface());
	
	/* Return the result: */
	return result;
	}

}

}
