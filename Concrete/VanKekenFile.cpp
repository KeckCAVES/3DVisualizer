/***********************************************************************
VanKekenFile - Class to encapsulate operations on Peter van Keken's old
mantle mixing simulations.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Misc/LargeFile.h>
#include <Cluster/MulticastPipe.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Endianness.h>

#include <Concrete/EarthDataSet.h>

#include <Concrete/VanKekenFile.h>

namespace Visualization {

namespace Concrete {

#if 0

namespace {

/*****************
Helper structures:
*****************/

struct GridFile
	{
	/* Elements: */
	public:
	std::string fileName; // Name of grid file
	int gridIndex; // Grid index of grid file
	
	/* Constructors and destructors: */
	GridFile(const char* sFileName,int sGridIndex)
		:fileName(sFileName),gridIndex(sGridIndex)
		{
		}
	
	/* Methods: */
	friend bool operator==(const GridFile& gf1,const GridFile& gf2)
		{
		return gf1.gridIndex==gf2.gridIndex;
		}
	friend bool operator<(const GridFile& gf1,const GridFile& gf2)
		{
		return gf1.gridIndex<gf2.gridIndex;
		}
	};

struct LongLat
	{
	/* Elements: */
	public:
	double longitude;
	double latitude;
	};

}

#endif

/*****************************
Methods of class VanKekenFile:
*****************************/

VanKekenFile::VanKekenFile(void)
	:BaseModule("VanKekenFile")
	{
	}

#if 1
Visualization::Abstract::DataSet* VanKekenFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	bool master=pipe==0||pipe->isMaster();
	
	EarthDataSet<DataSet>* result=0;
	if(master)
		{
		/* Open the grid and data files: */
		Misc::File gridFile(args[0].c_str(),"rb",Misc::File::LittleEndian);
		Misc::File dataFile(args[1].c_str(),"rb",Misc::File::LittleEndian);
		
		/* Read the grid/data file header: */
		int numGrids=gridFile.read<int>();
		int dataNumGrids=dataFile.read<int>();
		if(numGrids!=dataNumGrids)
			{
			if(pipe!=0)
				{
				pipe->write<int>(0);
				pipe->flush();
				}
			Misc::throwStdErr("VanKekenFile::load: Grid file %s and data file %s have mismatching sizes",args[0].c_str(),args[1].c_str());
			}
		else if(pipe!=0)
			{
			pipe->write<int>(1);
			pipe->write<int>(numGrids);
			}
			
		/* Create the result data set: */
		result=new EarthDataSet<DataSet>(args);
		result->setFlatteningFactor(0.0);
		result->getDs().setGrids(numGrids);
		
		/* Set the data value's name: */
		result->getDataValue().setScalarVariableName("Density");
		result->getDataValue().setVectorVariableName("Velocity");
		
		/* Read each grid: */
		std::cout<<"Reading grid vertex positions and values...   0%"<<std::flush;
		for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
			{
			/* Read the grid header: */
			DS::Index gridSize;
			gridFile.read<int>(gridSize.getComponents(),3);
			DS::Index dataGridSize;
			dataFile.read<int>(dataGridSize.getComponents(),3);
			if(gridSize!=dataGridSize)
				{
				if(pipe!=0)
					{
					pipe->write<int>(0);
					pipe->flush();
					}
				delete result;
				Misc::throwStdErr("VanKekenFile::load: Grid file %s and data file %s have mismatching sizes",args[0].c_str(),args[1].c_str());
				}
			else if(pipe!=0)
				{
				pipe->write<int>(1);
				pipe->write<int>(gridSize.getComponents(),3);
				}
			
			/* Add the grid to the data set: */
			result->getDs().setGridData(gridIndex,gridSize);
			
			/* Read the grid's vertex positions and values: */
			DS::Array& vertices=result->getDs().getGrid(gridIndex).getVertices();
			if(pipe!=0)
				{
				for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
					{
					gridFile.read<Scalar>(vIt->pos.getComponents(),3);
					pipe->write<Scalar>(vIt->pos.getComponents(),3);
					dataFile.read<VScalar>(vIt->value.scalar);
					pipe->write<VScalar>(vIt->value.scalar);
					dataFile.read<VScalar>(vIt->value.vector.getComponents(),3);
					pipe->write<VScalar>(vIt->value.vector.getComponents(),3);
					}
				}
			else
				{
				for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
					{
					gridFile.read<Scalar>(vIt->pos.getComponents(),3);
					dataFile.read<VScalar>(vIt->value.scalar);
					dataFile.read<VScalar>(vIt->value.vector.getComponents(),3);
					}
				}
			std::cout<<"\b\b\b\b"<<std::setw(3)<<((gridIndex+1)*100)/numGrids<<"%"<<std::flush;
			}
		std::cout<<"\b\b\b\bdone"<<std::endl;
		if(pipe!=0)
			pipe->flush();
		
		/* Finalize the grid structure: */
		std::cout<<"Finalizing grid structure..."<<std::flush;
		result->getDs().finalizeGrid();
		std::cout<<" done"<<std::endl;
		std::cout<<"Computed locator threshold: "<<result->getDs().getLocatorEpsilon()<<std::endl;
		}
	else
		{
		/* Read the number of grids: */
		if(pipe->read<int>()==0)
			Misc::throwStdErr("VanKekenFile::load: Grid file %s and data file %s have mismatching sizes",args[0].c_str(),args[1].c_str());
		int numGrids=pipe->read<int>();
		
		/* Create the result data set: */
		result=new EarthDataSet<DataSet>(args);
		result->setFlatteningFactor(0.0);
		result->getDs().setGrids(numGrids);
		
		/* Set the data value's name: */
		result->getDataValue().setScalarVariableName("Density");
		result->getDataValue().setVectorVariableName("Velocity");
		
		/* Read each grid: */
		for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
			{
			/* Read the grid size: */
			if(pipe->read<int>()==0)
				{
				delete result;
				Misc::throwStdErr("VanKekenFile::load: Grid file %s and data file %s have mismatching sizes",args[0].c_str(),args[1].c_str());
				}
			DS::Index gridSize;
			pipe->read<int>(gridSize.getComponents(),3);
			
			/* Add the grid to the data set: */
			result->getDs().setGridData(gridIndex,gridSize);
			
			/* Read the grid's vertex positions and values: */
			DS::Array& vertices=result->getDs().getGrid(gridIndex).getVertices();
			for(DS::Array::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
				{
				pipe->read<Scalar>(vIt->pos.getComponents(),3);
				pipe->read<VScalar>(vIt->value.scalar);
				pipe->read<VScalar>(vIt->value.vector.getComponents(),3);
				}
			}
		
		/* Finalize the grid structure: */
		result->getDs().finalizeGrid();
		}
	
	return result;
	}
#else
Visualization::Abstract::DataSet* VanKekenFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Read all coordinate and value file names from the given directory: */
	std::vector<GridFile> coordFiles;
	std::vector<GridFile> valueFiles;
	std::vector<double> gridRadii;
	DIR* gridFileDir=opendir(args[0].c_str());
	struct dirent* dirEntry;
	while((dirEntry=readdir(gridFileDir))!=0)
		{
		if(strcasecmp(dirEntry->d_name,"r.data")==0)
			{
			/* Read the radius data file: */
			char radiusFileName[2048];
			snprintf(radiusFileName,sizeof(radiusFileName),"%s/%s",args[0].c_str(),dirEntry->d_name);
			Misc::File radiusFile(radiusFileName,"rt");
			while(true)
				{
				double radius;
				if(fscanf(radiusFile.getFilePtr(),"%lf",&radius)!=1)
					break;
				gridRadii.push_back(radius);
				}
			}
		else
			{
			/* Split the directory entry's name into prefix and grid index: */
			const char* extPtr=0;
			for(const char* dnPtr=dirEntry->d_name;*dnPtr!='\0';++dnPtr)
				if(*dnPtr=='.')
					extPtr=dnPtr;
			bool gridIndexValid=isdigit(extPtr[1]);
			for(const char* giPtr=extPtr+2;*giPtr!='\0';++giPtr)
				if(!isdigit(*giPtr))
					gridIndexValid=false;
			if(gridIndexValid)
				{
				int gridIndex=atoi(extPtr+1);
				if(strncasecmp(dirEntry->d_name,"Coords",extPtr-dirEntry->d_name)==0)
					coordFiles.push_back(GridFile(dirEntry->d_name,gridIndex));
				else if(strncasecmp(dirEntry->d_name,args[1].c_str(),extPtr-dirEntry->d_name)==0)
					valueFiles.push_back(GridFile(dirEntry->d_name,gridIndex));
				}
			}
		}
	closedir(gridFileDir);
	
	/* Sort both lists of grid files by grid index: */
	std::sort(coordFiles.begin(),coordFiles.end());
	std::sort(valueFiles.begin(),valueFiles.end());
	
	/* Check grid file lists for consistency: */
	if(coordFiles.size()!=valueFiles.size())
		Misc::throwStdErr("VanKekenFile::load: Mismatching numbers of coordinate and value files");
	int numGrids=coordFiles.size();
	for(int i=0;i<numGrids;++i)
		if(coordFiles[i].gridIndex!=i)
			Misc::throwStdErr("VanKekenFile::load: Missing coordinate file for grid index %d",i);
	for(int i=0;i<numGrids;++i)
		if(valueFiles[i].gridIndex!=i)
			Misc::throwStdErr("VanKekenFile::load: Missing value file for grid index %d",i);
	
	/* Calculate size of each grid: */
	DS::Index gridSize;
	for(int i=0;i<2;++i)
		gridSize[i]=atoi(args[i+2].c_str());
	gridSize[2]=gridRadii.size();
	
	/* Create the data set: */
	EarthDataSet<DataSet>* result=new EarthDataSet<DataSet>(args);
	result->setFlatteningFactor(0.0);
	result->getDs().setGrids(numGrids);
	
	/* Set the data value's name: */
	result->getDataValue().setScalarVariableName("Some Scalar");
	result->getDataValue().setVectorVariableName("Velocity");
	
	/* Read all grid vertex positions and values: */
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		result->getDs().setGridData(gridIndex,gridSize);
		DS::Grid& grid=result->getDs().getGrid(gridIndex);
		
		/* Read the vertex longitudes/latitudes from the coordinate file: */
		char coordFileName[2048];
		snprintf(coordFileName,sizeof(coordFileName),"%s/%s",args[0].c_str(),coordFiles[gridIndex].fileName.c_str());
		Misc::File coordFile(coordFileName,"rt");
		LongLat* longLats=new LongLat[gridSize[1]*gridSize[0]];
		for(int i=0;i<gridSize[1]*gridSize[0];++i)
			{
			if(fscanf(coordFile.getFilePtr(),"%lf %lf",&longLats[i].latitude,&longLats[i].longitude)!=2)
				Misc::throwStdErr("VanKekenFile::load: Premature end of file in coordinate file %s",coordFileName);
			longLats[i].latitude=0.5*Math::Constants<double>::pi-longLats[i].latitude;
			}
		
		/* Read the radii from the coordinate file and compare against the already read radii: */
		for(int i=0;i<gridSize[2];++i)
			{
			double radius;
			if(fscanf(coordFile.getFilePtr(),"%lf",&radius)!=1)
				Misc::throwStdErr("VanKekenFile::load: Premature end of file in coordinate file %s",coordFileName);
			if(radius!=gridRadii[i])
				Misc::throwStdErr("VanKekenFile::load: Mismatching grid radius in coordinate file %s",coordFileName);
			}
		
		/* Parameters for geoid formula: */
		const double a=6378.14e3; // Equatorial radius in m
		const double f=1.0/298.257; // Geoid flattening factor
		const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
		
		/* Open the value file: */
		char valueFileName[2048];
		snprintf(valueFileName,sizeof(valueFileName),"%s/%s",args[0].c_str(),valueFiles[gridIndex].fileName.c_str());
		Misc::File valueFile(valueFileName,"rt");
		int dummy,numValues;
		if(fscanf(valueFile.getFilePtr(),"%d %d",&dummy,&numValues)!=2)
			Misc::throwStdErr("VanKekenFile::load: Missing header in value file %s",valueFileName);
		if(numValues!=gridSize[0]*gridSize[1]*gridSize[2])
			Misc::throwStdErr("VanKekenFile::load: Mismatching number of values in value file %s",valueFileName);
		
		/* Compute all vertex coordinates and read all vertex values: */
		DS::Array& vertices=grid.getVertices();
		DS::Index vIndex;
		for(vIndex[0]=0;vIndex[0]<gridSize[0];++vIndex[0])
			for(vIndex[1]=0;vIndex[1]<gridSize[1];++vIndex[1])
				{
				LongLat& ll=longLats[vIndex[0]*gridSize[1]+vIndex[1]];
				double s0=Math::sin(ll.latitude);
				double c0=Math::cos(ll.latitude);
				double s1=Math::sin(ll.longitude);
				double c1=Math::cos(ll.longitude);
				for(vIndex[2]=gridSize[2]-1;vIndex[2]>=0;--vIndex[2])
					{
					#if 1
					double r=a*gridRadii[gridSize[2]-1-vIndex[2]]*scaleFactor; // Use spherical globe model to simplify vector conversion
					#else
					double r=(a*(1.0-f*Math::sqr(s0))*gridRadii[gridSize[2]-1-vIndex[2]])*scaleFactor;
					#endif
					double xy=r*c0;
					DS::GridVertex* v=&vertices(vIndex);
					v->pos[0]=float(xy*c1);
					v->pos[1]=float(xy*s1);
					v->pos[2]=float(r*s0);

					/* Read the data values: */
					double vector[3];
					double scalar;
					if(fscanf(valueFile.getFilePtr(),"%lf %lf %lf %lf",&vector[0],&vector[1],&vector[2],&scalar)!=4)
						Misc::throwStdErr("VanKekenFile::load: Premature end of file in value file %s",valueFileName);

					/* Store the scalar value: */
					v->value.scalar=float(scalar);

					/* Convert the vector value from spherical to Cartesian coordinates: */
					#if 0
					/* Formula if vectors are given as dlat, dlong, dradius: */
					v->value.vector[0]=vector[2]*c0*c1-r*s0*vector[0]*c1-r*c0*s1*vector[1];
					v->value.vector[1]=vector[2]*c0*s1-r*s0*vector[0]*s1+r*c0*c1*vector[1];
					v->value.vector[2]=vector[2]*s0+r*c0*vector[0];
					#else
					/* Formula if vectors are given as Cartesian vectors in spherical coordinates: */
					vector[0]=-vector[0];
					v->value.vector[0]=float(c1*(c0*vector[2]-s0*vector[0])-s1*vector[1]);
					v->value.vector[1]=float(s1*(c0*vector[2]-s0*vector[0])+c1*vector[1]);
					v->value.vector[2]=float(c0*vector[0]+s0*vector[2]);
					#endif
					}
				}
		
		delete[] longLats;
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	#if 0
	/* Save the converted data to a pair of binary files: */
	Misc::File gridFile("Multigrid.grid","wb",Misc::File::LittleEndian);
	Misc::File dataFile("Multigrid.dat","wb",Misc::File::LittleEndian);
	
	/* Write the grid/data file header: */
	gridFile.write(numGrids);
	dataFile.write(numGrids);
	
	/* Write each grid: */
	for(int gridIndex=0;gridIndex<numGrids;++gridIndex)
		{
		/* Write the grid header: */
		gridFile.write<int>(gridSize.getComponents(),3);
		dataFile.write<int>(gridSize.getComponents(),3);
		
		/* Write the grid vertex positions: */
		const DS::Array& vertices=result->getDs().getGrid(gridIndex).getVertices();
		for(DS::Array::const_iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
			{
			gridFile.write<Scalar>(vIt->pos.getComponents(),3);
			dataFile.write<VScalar>(vIt->value.scalar);
			dataFile.write<VScalar>(vIt->value.vector.getComponents(),3);
			}
		}
	#endif
	
	return result;
	}
#endif

Visualization::Abstract::DataSetRenderer* VanKekenFile::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new EarthDataSetRenderer<DataSet,DataSetRenderer>(dataSet);
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::VanKekenFile* module=new Visualization::Concrete::VanKekenFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
