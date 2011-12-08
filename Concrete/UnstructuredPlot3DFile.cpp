/***********************************************************************
UnstructuredPlot3DFile - Class to read unstructured mesh data in NASA
Plot3D format.
Copyright (c) 2004-2007 Oliver Kreylos

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
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/UnstructuredPlot3DFile.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper structure:
****************/

struct SolutionParameters // Structure containing simulation parameters read from a solution file
	{
	/* Elements: */
	public:
	float mach;
	float alpha;
	float reynolds;
	float time;
	
	/* Constructors and destructors: */
	SolutionParameters(Misc::File& file)
		{
		file.read(mach);
		file.read(alpha);
		file.read(reynolds);
		file.read(time);
		}
	};

/****************
Helper functions:
****************/

void readGrid(UnstructuredPlot3DFile::DS* dataSet,const char* gridFileName)
	{
	/* Open the grid file: */
	Misc::File gridFile(gridFileName,"rb",Misc::File::BigEndian);
	
	/* Read the grid file header: */
	int numVertices=gridFile.read<int>();
	int numTriangles=gridFile.read<int>();
	int numTetrahedra=gridFile.read<int>();
	
	/* Add all (uninitialized) vertices to the data set: */
	UnstructuredPlot3DFile::DS::GridVertexIterator* vertices=new UnstructuredPlot3DFile::DS::GridVertexIterator[numVertices];
	for(int i=0;i<numVertices;++i)
		vertices[i]=dataSet->addVertex(UnstructuredPlot3DFile::DS::Point(),UnstructuredPlot3DFile::DS::Value());
	
	/* Read the vertices' coordinates: */
	float* vertexCoords=new float[numVertices];
	for(int coord=0;coord<3;++coord)
		{
		gridFile.read(vertexCoords,numVertices);
		for(int i=0;i<numVertices;++i)
			vertices[i]->pos[coord]=vertexCoords[i];
		}
	delete[] vertexCoords;
	
	/* Skip the triangle data: */
	gridFile.seekCurrent(numTriangles*4*sizeof(int));
	
	/* Read tetrahedra indices: */
	int* tetVertexIndices=new int[numTetrahedra*4];
	gridFile.read(tetVertexIndices,numTetrahedra*4);
	
	/* Add all tetrahedra to the data set: */
	for(int i=0;i<numTetrahedra;++i)
		{
		/* Convert the one-based indices to vertex iterators: */
		UnstructuredPlot3DFile::DS::GridVertexIterator cellVertices[4];
		for(int j=0;j<4;++j)
			cellVertices[j]=vertices[tetVertexIndices[i*4+j]-1];
		
		/* Add the cell: */
		dataSet->addCell(cellVertices);
		}
	
	/* Delete temporary data: */
	delete[] tetVertexIndices;
	delete[] vertices;
	
	/* Finalize the mesh structure: */
	dataSet->finalizeGrid();
	}

SolutionParameters readData(UnstructuredPlot3DFile::DS* grid,const char* solutionFileName)
	{
	/* Open the solution file: */
	Misc::File solutionFile(solutionFileName,"rb",Misc::File::BigEndian);
	
	/* Read the solution file header: */
	int numVertices=solutionFile.read<int>();
	// int numTriangles=solutionFile.read<int>();
	// int numTetrahedra=solutionFile.read<int>();
	solutionFile.seekCurrent(sizeof(int)*2);
	
	/* Check that the solution file matches the grid: */
	if((unsigned int)(numVertices)!=grid->getTotalNumVertices())
		Misc::throwStdErr("UnstructuredPlot3DFile::readData: Solution file does not match grid");
	
	/* Read the simulation parameters: */
	SolutionParameters sp(solutionFile);
	
	/* Read the vertex values: */
	float* valueSlice=new float[numVertices];
	for(int i=0;i<5;++i)
		{
		/* Read the value slice from file: */
		solutionFile.read(valueSlice,numVertices);
		
		/* Set the grid's vertex data components: */
		float* vsPtr=valueSlice;
		for(UnstructuredPlot3DFile::DS::GridVertexIterator vIt=grid->beginGridVertices();vIt!=grid->endGridVertices();++vIt,++vsPtr)
			{
			switch(i)
				{
				case 0:
					vIt->value.density=*vsPtr;
					break;
				
				case 1:
				case 2:
				case 3:
					vIt->value.momentum[i-1]=*vsPtr;
					break;
				
				case 4:
					vIt->value.energy=*vsPtr;
					break;
				}
			}
		}
	delete[] valueSlice;
	
	/* Return simulation parameters: */
	return sp;
	}

}

/***************************************
Methods of class UnstructuredPlot3DFile:
***************************************/

UnstructuredPlot3DFile::UnstructuredPlot3DFile(void)
	:BaseModule("UnstructuredPlot3DFile")
	{
	}

Visualization::Abstract::DataSet* UnstructuredPlot3DFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
	/* Create result data set: */
	DataSet* result=new DataSet;
	
	/* Read the grid structure: */
	char gridFilename[1024];
	snprintf(gridFilename,sizeof(gridFilename),"%s.grid",args[0].c_str());
	readGrid(&result->getDs(),gridFilename);
	
	/* Read the data values: */
	char solutionFilename[1024];
	snprintf(solutionFilename,sizeof(solutionFilename),"%s.sol",args[0].c_str());
	readData(&result->getDs(),solutionFilename);
	
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
	Visualization::Concrete::UnstructuredPlot3DFile* module=new Visualization::Concrete::UnstructuredPlot3DFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
