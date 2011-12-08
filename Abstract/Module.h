/***********************************************************************
Module - Abstract base class to represent modules of visualization data
types and algorithms. A module corresponds to a dynamically-linkable
unit of functionality in a 3D visualization application.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2005-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_MODULE_INCLUDED
#define VISUALIZATION_ABSTRACT_MODULE_INCLUDED

#include <string>
#include <vector>
#include <Plugins/Factory.h>
#include <IO/File.h>
#include <IO/SeekableFile.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}
class GLColorMap;
namespace Visualization {
namespace Abstract {
class DataSet;
class DataSetRenderer;
class ScalarExtractor;
class VectorExtractor;
class VariableManager;
class Algorithm;
}
}

namespace Visualization {

namespace Abstract {

class Module:public Plugins::Factory
	{
	/* Elements: */
	private:
	std::string baseDirectory; // Base directory for all input files
	
	/* Protected methods: */
	protected:
	static std::string makeVectorSliceName(std::string vectorName,int sliceIndex); // Creates a scalar slice name for a vector component
	std::string getFullPath(std::string fileName) const; // Returns the full path name of the given file relative to the base directory
	IO::FilePtr openFile(std::string fileName,Cluster::MulticastPipe* pipe) const; // Opens the given file relative to the base directory
	IO::SeekableFilePtr openSeekableFile(std::string fileName,Cluster::MulticastPipe* pipe) const; // Ditto, for seekable files
	
	/* Constructors and destructors: */
	public:
	Module(const char* sClassName); // Default constructor with class name of concrete module class
	private:
	Module(const Module& source); // Prohibit copy constructor
	Module& operator=(const Module& source); // Prohibit assignment operator
	public:
	virtual ~Module(void); // Destroys the module
	
	/* Methods: */
	void setBaseDirectory(std::string newBaseDirectory); // Sets the base directory for all following file operations
	virtual DataSet* load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const =0; // Loads a data set from the given list of arguments
	virtual DataSetRenderer* getRenderer(const DataSet* dataSet) const =0; // Creates a renderer for the given data set
	virtual int getNumScalarAlgorithms(void) const; // Returns number of available visualization algorithms
	virtual const char* getScalarAlgorithmName(int scalarAlgorithmIndex) const; // Returns the name of the given algorithm
	virtual Algorithm* getScalarAlgorithm(int scalarAlgorithmIndex,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const; // Returns the given visualization algorithm
	virtual int getNumVectorAlgorithms(void) const; // Returns number of available visualization algorithms
	virtual const char* getVectorAlgorithmName(int vectorAlgorithmIndex) const; // Returns the name of the given algorithm
	virtual Algorithm* getVectorAlgorithm(int vectorAlgorithmIndex,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const; // Returns the given visualization algorithm
	Algorithm* getAlgorithm(const char* algorithmName,VariableManager* variableManager,Cluster::MulticastPipe* pipe) const; // Convenience function to retrieve a scalar or vector algorithm by name
	};

}

}

#endif
