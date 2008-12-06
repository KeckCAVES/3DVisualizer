/***********************************************************************
TurbulenceGenerator - Class to generate artificial volumetric turbulence
data sets to test visualization algorithms.
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

#include <Misc/File.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Templatized/Curvilinear.h>
#include <Templatized/CurvilinearRenderer.h>
#include <Concrete/DensityValue.h>
#include <Wrappers/DataSet.h>
#include <Wrappers/DataSetRenderer.h>
#include <Templatized/SliceCaseTableTesseract.h>
#include <Templatized/SliceExtractor.h>
#include <Wrappers/Slice.h>
#include <Wrappers/SeededSliceExtractor.h>
#include <Templatized/IsosurfaceCaseTableTesseract.h>
#include <Templatized/IsosurfaceExtractor.h>
#include <Wrappers/Isosurface.h>
#include <Wrappers/SeededIsosurfaceExtractor.h>
#include <Concrete/Noise.h>

#include <Concrete/TurbulenceGenerator.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper structure:
****************/

struct TurbulenceGeneratorTypes // Structure containing the data types required to visualize artifical volumetric turbulence
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Templatized::Curvilinear<float,3,float> DS; // Templatized data set type
	typedef DensityValue<DS,float> DataValue; // Data value descriptor type
	typedef Visualization::Wrappers::DataSet<DS,float,DataValue> DataSet; // Data set class
	typedef Visualization::Wrappers::DataSetRenderer<DataSet> DataSetRenderer; // Data set renderer class
	typedef Visualization::Wrappers::SeededSliceExtractor<DataSet> SeededSliceExtractor; // Slice extractor class
	typedef Visualization::Wrappers::SeededIsosurfaceExtractor<DataSet> SeededIsosurfaceExtractor; // Isosurface extractor class
	};

}

/************************************
Methods of class TurbulenceGenerator:
************************************/

Visualization::Abstract::DataSet* TurbulenceGenerator::load(const char* filename) const
	{
	/* Create the data set: */
	TurbulenceGeneratorTypes::DS::Index numVertices(128,128,128);
	TurbulenceGeneratorTypes::DataSet* result=new TurbulenceGeneratorTypes::DataSet;
	result->getDs().setData(numVertices);
	
	/* Create the vertex values: */
	Noise ns(5,3);
	TurbulenceGeneratorTypes::DS::Array& vertices=result->getDs().getVertices();
	float cellSize[3]={0.05f,0.05f,0.05f};
	float minLat=Math::rad(20.0f);
	float maxLat=Math::rad(60.0f);
	float minLng=Math::rad(60.0f);
	float maxLng=Math::rad(120.0f);
	float minR=3000.0f;
	float maxR=6371.0f;
	for(TurbulenceGeneratorTypes::DS::Index index(0);index[0]<vertices.getSize(0);vertices.preInc(index))
		{
		TurbulenceGeneratorTypes::DS::GridVertex& vertex=vertices(index);
		
		/* Calculate the vertex position: */
		float lat=float(index[0])*(maxLat-minLat)/float(vertices.getSize(0)-1)+minLat;
		float lng=float(index[1])*(maxLng-minLng)/float(vertices.getSize(1)-1)+minLng;
		float r=float(index[2])*(maxR-minR)/float(vertices.getSize(2)-1)+minR;
		float xy=Math::cos(lat)*r;
		float z=Math::sin(lat)*r;
		float y=Math::sin(lng)*xy;
		float x=Math::cos(lng)*xy;
		vertex.pos=TurbulenceGeneratorTypes::DS::Point(x,y,z);
		
		/* Calculate the vertex value: */
		Noise::Point p;
		for(int i=0;i<3;++i)
			p[i]=float(index[i])*cellSize[i];
		vertex.value=ns.calcTurbulence(p,4);
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

Visualization::Abstract::DataSetRenderer* TurbulenceGenerator::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new TurbulenceGeneratorTypes::DataSetRenderer(dataSet);
	}

int TurbulenceGenerator::getNumScalarAlgorithms(void) const
	{
	return 3;
	}

const char* TurbulenceGenerator::getScalarAlgorithmName(int scalarAlgorithmIndex) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=3)
		Misc::throwStdErr("TurbulenceGenerator::getAlgorithmName: invalid algorithm index %d",scalarAlgorithmIndex);
	
	static const char* algorithmNames[3]=
		{
		"Seeded Slice","Seeded Isosurface (Flat Shaded)","Seeded Isosurface (Smooth Shaded)"
		};
	
	return algorithmNames[scalarAlgorithmIndex];
	}

Visualization::Abstract::Algorithm* TurbulenceGenerator::getScalarAlgorithm(int scalarAlgorithmIndex,const GLColorMap* colorMap,const Visualization::Abstract::DataSet* dataSet,const Visualization::Abstract::ScalarExtractor* scalarExtractor) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=3)
		Misc::throwStdErr("TurbulenceGenerator::getAlgorithm: invalid algorithm index %d",scalarAlgorithmIndex);
	
	Visualization::Abstract::Algorithm* result=0;
	switch(scalarAlgorithmIndex)
		{
		case 0:
			result=new TurbulenceGeneratorTypes::SeededSliceExtractor(colorMap,dataSet,scalarExtractor);
			break;
		
		case 1:
			{
			TurbulenceGeneratorTypes::SeededIsosurfaceExtractor* ise=new TurbulenceGeneratorTypes::SeededIsosurfaceExtractor(colorMap,dataSet,scalarExtractor);
			ise->getIse().setExtractionMode(TurbulenceGeneratorTypes::SeededIsosurfaceExtractor::ISE::FLAT);
			result=ise;
			break;
			}
		
		case 2:
			{
			TurbulenceGeneratorTypes::SeededIsosurfaceExtractor* ise=new TurbulenceGeneratorTypes::SeededIsosurfaceExtractor(colorMap,dataSet,scalarExtractor);
			ise->getIse().setExtractionMode(TurbulenceGeneratorTypes::SeededIsosurfaceExtractor::ISE::SMOOTH);
			result=ise;
			break;
			}
		}
	return result;
	}

}

}
