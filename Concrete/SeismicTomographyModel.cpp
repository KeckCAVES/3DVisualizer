/***********************************************************************
SeismicTomographyModel - Class to visualize results of seismic
tomographic analyses in Mercator grid format.
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
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <algorithm>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/SphericalCoordinateTransformer.h>
#include <Concrete/EarthDataSet.h>

#include <Concrete/SeismicTomographyModel.h>

namespace Visualization {

namespace Concrete {

namespace {

/*****************
Helper structures:
*****************/

struct SliceFile
	{
	/* Elements: */
	public:
	std::string fileName; // Name of slice file
	float depth; // Depth of slice file's grid values
	
	/* Constructors and destructors: */
	SliceFile(const char* sFileName,float sDepth)
		:fileName(sFileName),depth(sDepth)
		{
		}
	
	/* Methods: */
	friend bool operator==(const SliceFile& gf1,const SliceFile& gf2)
		{
		return gf1.depth==gf2.depth;
		}
	friend bool operator<(const SliceFile& gf1,const SliceFile& gf2)
		{
		return gf1.depth<gf2.depth;
		}
	};

}

/**************************************
Methods of class SeismicTomographyModel:
**************************************/

SeismicTomographyModel::SeismicTomographyModel(void)
	:BaseModule("SeismicTomographyModel")
	{
	}

Visualization::Abstract::DataSet* SeismicTomographyModel::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Parse the module command line: */
	bool haveNumVertices=false;
	DS::Index numVertices;
	bool multiFile=false; // Flag whether the data is stored in multiple slice files
	const char* dataFileName=0; // Data file name, or data file name template for slice files
	std::string sliceBaseDir; // Base directory name for slice files
	int column[4]={0,1,2,3}; // Default column order is latitude, longitude, depth, value
	bool cellCentered=false;
	int order[3]={0,1,2}; // Default data order is lat varies fastest, depth varies most slowly
	bool invert=false; // Flag whether to invert the order of the innermost loop
	bool colatitude=false; // Flag whether nodes are defined in colatitude instead of latitude
	float ignoreValue=0.0f; // "Sentinel" value to be ignored and set to zero upon reading
	bool storeSphericals=false; // Flag whether to store spherical coordinates as scalar variables
	for(unsigned int i=0;i<args.size();++i)
		{
		if(args[i][0]=='-')
			{
			if(strcasecmp(args[i].c_str()+1,"size")==0) // Size is given in lat, long, depth order
				{
				int j;
				for(j=0;j<3&&i<args.size()-1;++j)
					{
					++i;
					numVertices[j]=atoi(args[i].c_str());
					}
				haveNumVertices=j==3;
				}
			else if(strcasecmp(args[i].c_str()+1,"cell")==0)
				{
				/* Data is cell-centered: */
				cellCentered=true;
				}
			else if(strcasecmp(args[i].c_str()+1,"invert")==0)
				{
				/* Data order must be inverted: */
				invert=true;
				}
			else if(strcasecmp(args[i].c_str()+1,"colatitude")==0)
				{
				/* Data uses colatitude instead of latitude: */
				colatitude=true;
				}
			else if(strcasecmp(args[i].c_str()+1,"column")==0) // Column order is given in lat, long, depth, value order
				{
				int j;
				for(j=0;j<4&&i<args.size()-1;++j)
					{
					++i;
					column[j]=atoi(args[i].c_str());
					}
				if(j<4)
					Misc::throwStdErr("SeismicTomographyModel::load: Too few components in -column option");
				int columnMask=0x0;
				for(int j=0;j<4;++j)
					columnMask|=0x1<<column[j];
				if(columnMask!=0xf)
					Misc::throwStdErr("SeismicTomographyModel::load: -column option does not define a permutation");
				}
			else if(strcasecmp(args[i].c_str()+1,"order")==0) // Variation order is given in lat, long, depth order where 0 varies fastest
				{
				int j;
				for(j=0;j<3&&i<args.size()-1;++j)
					{
					++i;
					order[j]=atoi(args[i].c_str());
					}
				if(j<3)
					Misc::throwStdErr("SeismicTomographyModel::load: Too few components in -order option");
				int orderMask=0x0;
				for(int j=0;j<3;++j)
					orderMask|=0x1<<order[j];
				if(orderMask!=0x7)
					Misc::throwStdErr("SeismicTomographyModel::load: -order option does not define a permutation");
				}
			else if(strcasecmp(args[i].c_str()+1,"multi")==0)
				multiFile=true;
			else if(strcasecmp(args[i].c_str()+1,"ignore")==0)
				{
				if(i<args.size()-1)
					{
					++i;
					ignoreValue=float(atof(args[i].c_str()));
					}
				}
			else if(strcasecmp(args[i].c_str()+1,"storeCoords")==0)
				storeSphericals=true;
			}
		else
			dataFileName=args[i].c_str();
		}
	if(!haveNumVertices)
		Misc::throwStdErr("SeismicTomographyModel::load: Missing data set size");
	if(dataFileName==0)
		Misc::throwStdErr("SeismicTomographyModel::load: Missing data set file name");
	
	/* Compute the mapping from file order to C memory order: */
	int am[3];
	for(int i=0;i<3;++i)
		{
		for(int j=0;j<3;++j)
			if(order[j]==2-i)
				am[i]=j;
		}
	
	std::vector<SliceFile> sliceFiles;
	if(multiFile)
		{
		/* Check if the file name template is a proper template (has one float conversion in the file name): */
		int numConversions=0;
		bool hasFloatConversion=false;
		for(const char* dfnPtr=dataFileName;*dfnPtr!='\0';++dfnPtr)
			{
			if(*dfnPtr=='%'&&dfnPtr[1]!='%')
				{
				++numConversions;
				if(dfnPtr[1]=='f')
					hasFloatConversion=true;
				}
			else if(*dfnPtr=='/')
				hasFloatConversion=false;
			}
		
		if(!hasFloatConversion)
			Misc::throwStdErr("SeismicTomographyModel::load: Slice file name template \"%s\" does not contain %f float conversion",dataFileName);
		if(numConversions>1)
			Misc::throwStdErr("SeismicTomographyModel::load: Slice file name template \"%s\" contains too many conversions",dataFileName);
		
		/* Split the file name template into base directory and file name: */
		const char* slashPtr=0;
		for(const char* dfnPtr=dataFileName;*dfnPtr!='\0';++dfnPtr)
			if(*dfnPtr=='/')
				slashPtr=dfnPtr;
		std::string fileName;
		if(slashPtr==0)
			{
			sliceBaseDir=".";
			fileName=dataFileName;
			}
		else
			{
			sliceBaseDir=std::string(dataFileName,(slashPtr-dataFileName)+1);
			fileName=slashPtr+1;
			}
		
		/* Load all slice files: */
		DIR* sliceFileDir=opendir(sliceBaseDir.c_str());
		if(sliceFileDir==0)
			Misc::throwStdErr("SeismicTomographyModel::load: Could not open slice file directory %s",sliceBaseDir.c_str());
		struct dirent* dirEntry;
		while((dirEntry=readdir(sliceFileDir))!=0)
			{
			/* Check if the directory entry is a slice file: */
			float depth;
			if(sscanf(dirEntry->d_name,fileName.c_str(),&depth)==1)
				{
				/* Remember the slice file: */
				sliceFiles.push_back(SliceFile(dirEntry->d_name,depth));
				}
			}
		closedir(sliceFileDir);
		
		/* Sort the list of slice files by depth: */
		std::sort(sliceFiles.begin(),sliceFiles.end());
		
		/* Override the data set size with the number of slice files: */
		numVertices[am[0]]=sliceFiles.size();
		}
	
	/* Data size is depth, longitude, (co-)latitude in C memory order ((co-)latitude varies fastest) */
	DS::Index gridSize=numVertices;
	if(cellCentered)
		++gridSize[1]; // Replicate meridian to stich grid
	
	/* Create the data set: */
	EarthDataSet<DataSet>* result=new EarthDataSet<DataSet>(args);
	DS& dataSet=result->getDs();
	result->getSphericalCoordinateTransformer()->setColatitude(colatitude);
	result->getSphericalCoordinateTransformer()->setDepth(true);
	dataSet.setNumGrids(1);
	dataSet.setGrid(0,gridSize);
	
	/* Initialize the data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	if(storeSphericals)
		{
		dataValue.addScalarVariable(colatitude?"Colatitude":"Latitude");
		dataSet.addSlice();
		dataValue.addScalarVariable("Longitude");
		dataSet.addSlice();
		dataValue.addScalarVariable("Depth");
		dataSet.addSlice();
		}
	dataValue.addScalarVariable("Differential Wave Velocity");
	dataSet.addSlice();
	
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.257; // Geoid flattening factor
	const double scaleFactor=1.0e-3;
	
	/* Read all grid points from the input file(s): */
	Misc::File* vFile=0;
	double depth=0.0;
	DS::GridArray& vertices=result->getDs().getGrid(0).getGrid();
	DS::Index index;
	VScalar* valueSlices[4];
	if(storeSphericals)
		{
		for(int i=0;i<4;++i)
			valueSlices[i]=dataSet.getSliceArray(i,0);
		}
	else
		valueSlices[3]=dataSet.getSliceArray(0,0);
	if(!multiFile)
		{
		/* Open the input wave velocity file: */
		vFile=new Misc::File(dataFileName,"rt");
		}
	for(index[am[0]]=0;index[am[0]]<numVertices[am[0]];++index[am[0]])
		{
		if(multiFile)
			{
			/* Open the next slice file: */
			delete vFile;
			std::string sliceFileName=sliceBaseDir;
			sliceFileName+=sliceFiles[index[am[0]]].fileName;
			vFile=new Misc::File(sliceFileName.c_str(),"rt");
			depth=double(sliceFiles[index[am[0]]].depth);
			}
		for(index[am[1]]=0;index[am[1]]<numVertices[am[1]];++index[am[1]])
			{
			if(invert)
				{
				for(index[am[2]]=numVertices[am[2]]-1;index[am[2]]>=0;--index[am[2]])
					{
					/* Read the next grid point from the file: */
					char line[80];
					vFile->gets(line,sizeof(line));
					double col[4];
					if(multiFile)
						{
						col[0]=depth;
						sscanf(line,"%lf %lf %lf",&col[1],&col[2],&col[3]);
						}
					else
						sscanf(line,"%lf %lf %lf %lf",&col[0],&col[1],&col[2],&col[3]);
					double lat=col[column[0]];
					double lng=col[column[1]];
					double depth=col[column[2]];
					VScalar value=VScalar(col[column[3]]);
					if(value==ignoreValue)
						value=VScalar(0);
					
					/* Convert geoid coordinates to Cartesian coordinates: */
					lat=colatitude?Math::rad(90.0-lat):Math::rad(lat);
					lng=Math::rad(lng);
					double s0=Math::sin(lat);
					double c0=Math::cos(lat);
					double r=(a*(1.0-f*Math::sqr(s0))-depth*1000.0)*scaleFactor;
					double xy=r*c0;
					double s1=Math::sin(lng);
					double c1=Math::cos(lng);
					vertices(index)=DS::Point(float(xy*c1),float(xy*s1),float(r*s0));
					
					int linearIndex=vertices.calcLinearIndex(index);
					if(storeSphericals)
						{
						/* Store the spherical coordinates: */
						for(int i=0;i<3;++i)
							valueSlices[i][linearIndex]=VScalar(col[column[i]]);
						}
					
					/* Store the velocity value: */
					valueSlices[3][linearIndex]=value;
					}
				}
			else
				{
				for(index[am[2]]=0;index[am[2]]<numVertices[am[2]];++index[am[2]])
					{
					/* Read the next grid point from the file: */
					char line[80];
					vFile->gets(line,sizeof(line));
					double col[4];
					if(multiFile)
						{
						col[0]=depth;
						sscanf(line,"%lf %lf %lf",&col[1],&col[2],&col[3]);
						}
					else
						sscanf(line,"%lf %lf %lf %lf",&col[0],&col[1],&col[2],&col[3]);
					double lat=col[column[0]];
					double lng=col[column[1]];
					double depth=col[column[2]];
					VScalar value=VScalar(col[column[3]]);
					if(value==ignoreValue)
						value=VScalar(0);
					
					/* Convert geoid coordinates to Cartesian coordinates: */
					lat=colatitude?Math::rad(90.0-lat):Math::rad(lat);
					lng=Math::rad(lng);
					double s0=Math::sin(lat);
					double c0=Math::cos(lat);
					double r=(a*(1.0-f*Math::sqr(s0))-depth*1000.0)*scaleFactor;
					double xy=r*c0;
					double s1=Math::sin(lng);
					double c1=Math::cos(lng);
					vertices(index)=DS::Point(float(xy*c1),float(xy*s1),float(r*s0));
					
					int linearIndex=vertices.calcLinearIndex(index);
					if(storeSphericals)
						{
						/* Store the spherical coordinates: */
						for(int i=0;i<3;++i)
							valueSlices[i][linearIndex]=VScalar(col[column[i]]);
						}
					
					/* Store the velocity value: */
					valueSlices[3][linearIndex]=VScalar(col[column[3]]);
					}
				}
			}
		}
	delete vFile;
	
	if(cellCentered)
		{
		/* Stitch the grid across the longitude boundaries: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				{
				DS::Index s(index[0],0,index[2]);
				DS::Index d(index[0],numVertices[1],index[2]);
				int ls=vertices.calcLinearIndex(s);
				int ld=vertices.calcLinearIndex(d);
				vertices.getArray()[ld]=vertices.getArray()[ls];
				if(storeSphericals)
					for(int i=0;i<3;++i)
						valueSlices[i][ld]=valueSlices[i][ls];
				valueSlices[3][ld]=valueSlices[3][ls];
				}
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

Visualization::Abstract::DataSetRenderer* SeismicTomographyModel::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
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
	Visualization::Concrete::SeismicTomographyModel* module=new Visualization::Concrete::SeismicTomographyModel();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
