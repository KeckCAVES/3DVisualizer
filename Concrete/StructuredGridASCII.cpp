/***********************************************************************
StructuredGridASCII - Class defining lowest-common-denominator ASCII
file format for curvilinear grids in Cartesian or spherical coordinates.
Vertex positions and attributes are stored in separate files.
Copyright (c) 2008 Oliver Kreylos

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
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

#include <Concrete/StructuredGridASCII.h>

namespace Visualization {

namespace Concrete {

/************************************
Methods of class StructuredGridASCII:
************************************/

StructuredGridASCII::StructuredGridASCII(void)
	:BaseModule("StructuredGridASCII")
	{
	}

Visualization::Abstract::DataSet* StructuredGridASCII::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	Misc::SelfDestructPointer<DataSet> result(new DataSet);
	
	/* Parse command line parameters related to the grid definition file: */
	std::vector<std::string>::const_iterator argIt=args.begin();
	bool storeSphericals=false;
	while((*argIt)[0]=='-')
		{
		/* Parse the command line parameter: */
		if(strcasecmp(argIt->c_str(),"-storeCoords")==0)
			storeSphericals=true;
		
		++argIt;
		}
	
	/* Open the grid definition file: */
	std::cout<<"Reading grid file "<<*argIt<<"..."<<std::flush;
	Misc::File gridFile(argIt->c_str(),"rt");
	
	/* Parse the grid file header: */
	DS::Index numVertices(-1,-1,-1);
	bool sphericalCoordinates=false;
	unsigned int lineIndex=0;
	char line[256];
	int parsedHeaderLines=0;
	while(parsedHeaderLines<2)
		{
		/* Check if the file is already over: */
		if(gridFile.eof())
			Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in grid file %s",argIt->c_str());
		
		/* Read the next line from the file: */
		gridFile.gets(line,sizeof(line));
		++lineIndex;
		
		/* Skip comment lines: */
		if(line[0]!='#')
			{
			/* Parse the line: */
			switch(parsedHeaderLines)
				{
				case 0:
					if(strncasecmp(line,"gridsize",8)==0&&(line[8]==' '||line[8]=='\t'))
						{
						/* Read the grid size: */
						if(sscanf(line+9,"%d %d %d",&numVertices[0],&numVertices[1],&numVertices[2])!=3)
							Misc::throwStdErr("StructuredGridASCII::load: invalid grid size in line %u in grid file %s",lineIndex,argIt->c_str());
						}
					else
						Misc::throwStdErr("StructuredGridASCII::load: missing grid size in line %u in grid file %s",lineIndex,argIt->c_str());
					break;
				
				case 1:
					if(strncasecmp(line,"coordinate",10)==0&&(line[10]==' '||line[10]=='\t'))
						{
						/* Read the coordinate mode: */
						const char* modeStartPtr;
						for(modeStartPtr=line+11;*modeStartPtr!='\0'&&isspace(*modeStartPtr);++modeStartPtr)
							;
						const char* modeEndPtr;
						for(modeEndPtr=modeStartPtr;*modeEndPtr!='\0'&&!isspace(*modeEndPtr);++modeEndPtr)
							; 
						if(modeEndPtr-modeStartPtr==9&&strncasecmp(modeStartPtr,"Cartesian",9)==0)
							sphericalCoordinates=false;
						else if(modeEndPtr-modeStartPtr==9&&strncasecmp(modeStartPtr,"spherical",9)==0)
							sphericalCoordinates=true;
						else
							Misc::throwStdErr("StructuredGridASCII::load: invalid coordinate mode in line %u in grid file %s",lineIndex,argIt->c_str());
						}
					else
						Misc::throwStdErr("StructuredGridASCII::load: missing coordinate mode in line %u in grid file %s",lineIndex,argIt->c_str());
					break;
				}
			
			++parsedHeaderLines;
			}
		}
	
	/* Initialize the data set: */
	DS& dataSet=result->getDs();
	dataSet.setGrid(numVertices);
	
	/* Initialize the result data set's data value: */
	DataValue& dataValue=result->getDataValue();
	dataValue.initialize(&dataSet,0);
	
	if(sphericalCoordinates&&storeSphericals)
		{
		/* Add three slices to the data set: */
		static const char* coordSliceNames[3]={"Latitude","Longitude","Radius"};
		for(int i=0;i<3;++i)
			{
			dataSet.addSlice();
			dataValue.addScalarVariable(coordSliceNames[i]);
			}
		}
	
	/* Prepare the spherical-to-Cartesian formula: */
	// const double a=6378.14e3; // Equatorial radius in m
	// const double f=1.0/298.247; // Geoid flattening factor (not used, since there could be vector values)
	const double scaleFactor=1.0e-3; // Scale factor for Cartesian coordinates
	
	/* Read all vertex positions: */
	std::cout<<"   0%"<<std::flush;
	DS::Index index(0);
	while(index[2]<numVertices[2])
		{
		/* Read the next line: */
		gridFile.gets(line,sizeof(line));
		++lineIndex;
		
		if(line[0]!='#')
			{
			/* Parse the line: */
			DS::Point& vertex=dataSet.getVertexPosition(index);
			if(sphericalCoordinates)
				{
				/* Read the vertex' position in spherical coordinates: */
				double longitude,latitude,radius;
				if(sscanf(line,"%lf %lf %lf",&longitude,&latitude,&radius)!=3)
					Misc::throwStdErr("StructuredGridASCII::load: Invalid spherical vertex coordinate in line %u in grid file %s",lineIndex,argIt->c_str());
				
				/* Convert the vertex position to Cartesian coordinates: */
				double s0=Math::sin(latitude);
				double c0=Math::cos(latitude);
				double r=radius*scaleFactor;
				double xy=r*c0;
				double s1=Math::sin(longitude);
				double c1=Math::cos(longitude);
				vertex[0]=Scalar(xy*c1);
				vertex[1]=Scalar(xy*s1);
				vertex[2]=Scalar(r*s0);
				
				if(storeSphericals)
					{
					/* Store the spherical coordinate components in the first three value slices: */
					dataSet.getVertexValue(0,index)=Scalar(Math::deg(latitude));
					dataSet.getVertexValue(1,index)=Scalar(Math::deg(longitude));
					dataSet.getVertexValue(2,index)=Scalar(r);
					}
				}
			else
				{
				/* Read the vertex' position in Cartesian coordinates: */
				if(sscanf(line,"%f %f %f",&vertex[0],&vertex[1],&vertex[2])!=3)
					Misc::throwStdErr("StructuredGridASCII::load: Invalid Cartesian vertex coordinate in line %u in grid file %s",lineIndex,argIt->c_str());
				}
			
			/* Go to the next vertex: */
			int incDim;
			for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
				index[incDim]=0;
			++index[incDim];
			if(incDim==2)
				std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
			}
		}
	std::cout<<"\b\b\b\bdone"<<std::endl;
	
	/* Finalize the grid structure: */
	std::cout<<"Finalizing grid structure..."<<std::flush;
	dataSet.finalizeGrid();
	std::cout<<" done"<<std::endl;
	
	/* Read all vertex attribute files given on the command line: */
	bool logNextScalar=false;
	for(++argIt;argIt!=args.end();++argIt)
		{
		if(strcasecmp(argIt->c_str(),"-log")==0)
			logNextScalar=true;
		else
			{
			/* Open the slice file: */
			std::cout<<"Reading slice file "<<*argIt<<"..."<<std::flush;
			Misc::File sliceFile(argIt->c_str(),"rt");
			
			/* Parse the slice file header: */
			bool vectorValue=false;
			int sliceIndex=dataSet.getNumSlices();
			lineIndex=0;
			parsedHeaderLines=0;
			while(parsedHeaderLines<2)
				{
				/* Check if the file is already over: */
				if(sliceFile.eof())
					Misc::throwStdErr("StructuredGridASCII::load: early end-of-file in slice file %s",argIt->c_str());
				
				/* Read the next line from the file: */
				sliceFile.gets(line,sizeof(line));
				++lineIndex;
				
				/* Skip comment lines: */
				if(line[0]!='#')
					{
					/* Parse the line: */
					switch(parsedHeaderLines)
						{
						case 0:
							if(strncasecmp(line,"gridsize",8)==0&&(line[8]==' '||line[8]=='\t'))
								{
								/* Read the grid size: */
								DS::Index sliceNumVertices;
								if(sscanf(line+9,"%d %d %d",&sliceNumVertices[0],&sliceNumVertices[1],&sliceNumVertices[2])!=3)
									Misc::throwStdErr("StructuredGridASCII::load: invalid grid size in line %u in slice file %s",lineIndex,argIt->c_str());
								
								/* Check if the slice grid size matches the grid size: */
								if(sliceNumVertices!=numVertices)
									Misc::throwStdErr("StructuredGridASCII::load: mismatching grid size in slice file %s",argIt->c_str());
								}
							else
								Misc::throwStdErr("StructuredGridASCII::load: missing grid size in line %u in slice file %s",lineIndex,argIt->c_str());
							break;

						case 1:
							if(strncasecmp(line,"scalar",6)==0&&(line[6]==' '||line[6]=='\t'))
								{
								vectorValue=false;
								
								/* Read the scalar value's name: */
								char* nameStartPtr;
								for(nameStartPtr=line+7;*nameStartPtr!='\0'&&isspace(*nameStartPtr);++nameStartPtr)
									;
								char* nameEndPtr;
								for(nameEndPtr=nameStartPtr;*nameEndPtr!='\0';++nameEndPtr)
									; 
								for(;isspace(nameEndPtr[-1]);--nameEndPtr)
									;
								if(nameStartPtr!=nameEndPtr)
									{
									*nameEndPtr='\0';
									
									/* Add another slice to the data set: */
									dataSet.addSlice();
									
									/* Add another scalar variable to the data value: */
									if(logNextScalar)
										{
										char variableName[256];
										snprintf(variableName,sizeof(variableName),"log(%s)",nameStartPtr);
										dataValue.addScalarVariable(variableName);
										}
									else
										dataValue.addScalarVariable(nameStartPtr);
									}
								else
									Misc::throwStdErr("StructuredGridASCII::load: empty vector variable name in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							else if(strncasecmp(line,"vector",6)==0&&(line[6]==' '||line[6]=='\t'))
								{
								vectorValue=true;
								
								/* Read the vector value's name: */
								char* nameStartPtr;
								for(nameStartPtr=line+7;*nameStartPtr!='\0'&&isspace(*nameStartPtr);++nameStartPtr)
									;
								char* nameEndPtr;
								for(nameEndPtr=nameStartPtr;*nameEndPtr!='\0';++nameEndPtr)
									; 
								for(;isspace(nameEndPtr[-1]);--nameEndPtr)
									;
								if(nameStartPtr!=nameEndPtr)
									{
									*nameEndPtr='\0';
									
									/* Add another vector variable to the data value: */
									int vectorVariableIndex=dataValue.getNumVectorVariables();
									dataValue.addVectorVariable(nameStartPtr);
									
									/* Add four new slices to the data set (3 components plus magnitude): */
									char variableName[256];
									for(int i=0;i<3;++i)
										{
										dataSet.addSlice();
										snprintf(variableName,sizeof(variableName),"%s %c",nameStartPtr,'X'+i);
										dataValue.addScalarVariable(variableName);
										dataValue.setVectorVariableScalarIndex(vectorVariableIndex,i,sliceIndex+i);
										}
									dataSet.addSlice();
									snprintf(variableName,sizeof(variableName),"%s Magnitude",nameStartPtr);
									dataValue.addScalarVariable(variableName);
									}
								else
									Misc::throwStdErr("StructuredGridASCII::load: empty vector variable name in line %u in slice file %s",lineIndex,argIt->c_str());
								}
							else
								Misc::throwStdErr("StructuredGridASCII::load: missing value type in line %u in slice file %s",lineIndex,argIt->c_str());
							break;
						}

					++parsedHeaderLines;
					}
				}
			
			/* Read all vertex attributes: */
			std::cout<<"   0%"<<std::flush;
			DS::Index index(0);
			while(index[2]<numVertices[2])
				{
				/* Read the next line: */
				sliceFile.gets(line,sizeof(line));
				++lineIndex;

				if(line[0]!='#')
					{
					/* Parse the line: */
					if(vectorValue)
						{
						DataValue::VVector vector;
						if(sphericalCoordinates)
							{
							/* Read the vector attribute in spherical coordinates: */
							double longitude,latitude,radius;
							if(sscanf(line,"%lf %lf %lf",&longitude,&latitude,&radius)!=3)
								Misc::throwStdErr("StructuredGridASCII::load: Invalid spherical vector attribute in line %u in slice file %s",lineIndex,argIt->c_str());
							
							/* Convert the vector to Cartesian coordinates: */
							const DS::Point& p=dataSet.getVertexPosition(index);
							double xy=Math::sqr(double(p[0]))+Math::sqr(double(p[1]));
							double r=xy+Math::sqr(double(p[2]));
							xy=Math::sqrt(xy);
							r=Math::sqrt(r);
							double s0=double(p[2])/r;
							double c0=xy/r;
							double s1=double(p[1])/xy;
							double c1=double(p[0])/xy;
							vector[0]=Scalar(c1*(c0*radius-s0*latitude)-s1*longitude);
							vector[1]=Scalar(s1*(c0*radius-s0*latitude)+c1*longitude);
							vector[2]=Scalar(c0*latitude+s0*radius);
							}
						else
							{
							/* Read the vector attribute in Cartesian coordinates: */
							if(sscanf(line,"%f %f %f",&vector[0],&vector[1],&vector[2])!=3)
								Misc::throwStdErr("StructuredGridASCII::load: Invalid Cartesian vector attribute in line %u in slice file %s",lineIndex,argIt->c_str());
							}
						
						/* Store the vector's components and magnitude: */
						for(int i=0;i<3;++i)
							dataSet.getVertexValue(sliceIndex+i,index)=vector[i];
						dataSet.getVertexValue(sliceIndex+3,index)=Scalar(Geometry::mag(vector));
						}
					else
						{
						/* Read the scalar attribute: */
						if(logNextScalar)
							{
							double value;
							if(sscanf(line,"%lf",&value)!=1)
								Misc::throwStdErr("StructuredGridASCII::load: Invalid logarithmic scalar vertex attribute in line %u in slice file %s",lineIndex,argIt->c_str());
							dataSet.getVertexValue(sliceIndex,index)=Scalar(Math::log10(value));
							}
						else
							{
							if(sscanf(line,"%f",&dataSet.getVertexValue(sliceIndex,index))!=1)
								Misc::throwStdErr("StructuredGridASCII::load: Invalid scalar vertex attribute in line %u in slice file %s",lineIndex,argIt->c_str());
							}
						}

					/* Go to the next vertex: */
					int incDim;
					for(incDim=0;incDim<2&&index[incDim]==numVertices[incDim]-1;++incDim)
						index[incDim]=0;
					++index[incDim];
					if(incDim==2)
						std::cout<<"\b\b\b\b"<<std::setw(3)<<(index[2]*100)/numVertices[2]<<"%"<<std::flush;
					}
				}
			std::cout<<"\b\b\b\bdone"<<std::endl;
			}
		}
	
	/* Return the result data set: */
	return result.releaseTarget();
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::StructuredGridASCII* module=new Visualization::Concrete::StructuredGridASCII();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
