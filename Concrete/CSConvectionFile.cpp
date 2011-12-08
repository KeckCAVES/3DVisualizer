/***********************************************************************
CSConvectionFile - Class to encapsulate operations on C.S. Natarajan's
convection simulation data sets.
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

#include <ctype.h>
#include <stdlib.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

#include <Concrete/CSConvectionFile.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
****************/

template <class VectorParam,class ScalarParam>
inline void sphericalToCartesian(VectorParam& vector,const ScalarParam spherical[3])
	{
	ScalarParam cc=Math::cos(spherical[0]);
	ScalarParam cs=Math::sin(spherical[0]);
	ScalarParam lc=Math::cos(spherical[1]);
	ScalarParam ls=Math::sin(spherical[1]);
	ScalarParam r=spherical[2];
	vector[0]=cs*ls*r;
	vector[1]=cs*lc*r;
	vector[2]=cc*r;
	}

CSConvectionFile::DS::Index parseZoneSize(const char* line)
	{
	CSConvectionFile::DS::Index result;
	int compMask=0x0;
	
	const char* lPtr=line;
	while(true)
		{
		/* Search for the next equal sign: */
		while(*lPtr!='\0'&&*lPtr!='=')
			++lPtr;
		if(*lPtr=='\0')
			break;
		
		if(lPtr[-1]>='i'&&lPtr[-1]<='k')
			{
			/* Parse the next number: */
			const char* start;
			for(start=lPtr+1;isspace(*start);++start)
				;
			int number=0;
			const char* end;
			for(end=start;isdigit(*end);++end)
				number=number*10+int(*end-'0');
			result[2-(lPtr[-1]-'i')]=number;
			compMask|=1<<(lPtr[-1]-'i');
			lPtr=end;
			}
		else
			++lPtr;
		}
	
	/* Check if all zone size components have been read: */
	if(compMask!=0x7)
		Misc::throwStdErr("CSConvectionFile::load: Wrong format in data file header");
	
	return result;
	}

}

/*********************************
Methods of class CSConvectionFile:
*********************************/

CSConvectionFile::CSConvectionFile(void)
	:BaseModule("CSConvectionFile")
	{
	}

Visualization::Abstract::DataSet* CSConvectionFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Open the data file: */
	Misc::File dataFile(args[0].c_str(),"rt");
	
	/* Read data file's header: */
	char line[256];
	dataFile.gets(line,sizeof(line)); // Skip title field
	if(strncmp(line,"TITLE=",6)!=0)
		Misc::throwStdErr("CSConvectionFile::load: Wrong format in data file header");
	dataFile.gets(line,sizeof(line)); // Skip variable field
	if(strncmp(line,"VARIABLES=",10)!=0)
		Misc::throwStdErr("CSConvectionFile::load: Wrong format in data file header");
	
	/* Determine which zone to read: */
	int zoneIndex=0;
	if(args.size()>1)
		zoneIndex=atoi(args[1].c_str());
	
	/* Read the first zone from the data file: */
	dataFile.gets(line,sizeof(line)); // Read size field
	if(strncmp(line,"ZONE ",5)!=0)
		Misc::throwStdErr("CSConvectionFile::load: Wrong format in data file header");
	DS::Index numZoneVertices=parseZoneSize(line);
	
	/* Skip zones until the correct one: */
	for(int i=0;i<zoneIndex;++i)
		{
		/* Skip all nodes of this zone: */
		size_t numNodes=size_t(numZoneVertices.calcIncrement(-1));
		for(size_t j=0;j<numNodes;++j)
			dataFile.gets(line,sizeof(line));
		
		/* Read the next zone header: */
		dataFile.gets(line,sizeof(line)); // Read size field
		if(strncmp(line,"ZONE ",5)!=0)
			Misc::throwStdErr("CSConvectionFile::load: Wrong format in data file header");
		numZoneVertices=parseZoneSize(line);
		}
	
	/* Create the result data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numZoneVertices);
	
	/* Read all vertex positions and values: */
	DS::Array& vertices=result->getDs().getVertices();
	for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		{
		dataFile.gets(line,sizeof(line));
		float pos[3];
		float temp,visc;
		float vel[3];
		if(sscanf(line,"%f %f %f %f %f %f %f %f",&pos[0],&pos[1],&pos[2],&temp,&visc,&vel[0],&vel[1],&vel[2])!=8)
			Misc::throwStdErr("CSConvectionFile::load: Error while reading grid data");
		
		sphericalToCartesian(vIt->pos,pos);
		vIt->value.temperature=temp;
		vIt->value.viscosity=Math::log(visc);
		sphericalToCartesian(vIt->value.velocity,vel);
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
	Visualization::Concrete::CSConvectionFile* module=new Visualization::Concrete::CSConvectionFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
