/***********************************************************************
MargareteSubductionFile - Class to encapsulate operations on Margarete
Jadamec's subduction simulation data sets.
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

#include <stdio.h>
#include <Misc/File.h>
#include <Misc/LargeFile.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Endianness.h>

#include <Concrete/MargareteSubductionFile.h>

namespace Visualization {

namespace Concrete {

/****************************************
Methods of class MargareteSubductionFile:
****************************************/

MargareteSubductionFile::MargareteSubductionFile(void)
	:BaseModule("MargareteSubductionFile")
	{
	}

Visualization::Abstract::DataSet* MargareteSubductionFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the data file: */
	Misc::File dataFile(args[0].c_str(),"rt");
	
	/* Skip any header lines in the data set: */
	char line[256];
	do
		{
		dataFile.gets(line,sizeof(line));
		}
	while(line[0]=='#');
	
	/* Grid order in file is latitude, longitude, depth (from slowest to fastest) */
	
	/* Create result data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(DS::Index(369,385,145));
	
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
	
	/* Read all vertex positions and values: */
	DS::Array& vertices=result->getDs().getVertices();
	for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		{
		/* Parse the grid vertex' components from the line: */
		int nodeIndex;
		double latitude,longitude,radius,temperature,viscosity;
		sscanf(line,"%d %lf %lf %lf %lf %lf",&nodeIndex,&longitude,&radius,&latitude,&temperature,&viscosity);
		
		/* Convert from spherical to Cartesian coordinates: */
		double s0=Math::sin(latitude);
		double c0=Math::cos(latitude);
		double r=(a*(1.0-f*Math::sqr(s0))*radius)*scaleFactor;
		double xy=r*c0;
		double s1=Math::sin(longitude);
		double c1=Math::cos(longitude);
		vIt->pos[0]=float(xy*c1);
		vIt->pos[1]=float(xy*s1);
		vIt->pos[2]=float(r*s0);
		vIt->value.temperature=float(temperature*1400.0);
		vIt->value.viscosity=viscosity; // Math::log(float(viscosity));
		
		/* Read the next line from the file: */
		dataFile.gets(line,sizeof(line));
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::MargareteSubductionFile* module=new Visualization::Concrete::MargareteSubductionFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
