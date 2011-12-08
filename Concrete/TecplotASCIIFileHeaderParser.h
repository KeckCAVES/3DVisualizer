/***********************************************************************
TecplotASCIIFileHeaderParser - Helper functions to parse the headers of
ASCII Tecplot files.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef VISUALIZATION_CONCRETE_TECPLOTASCIIFILEHEADERPARSER_INCLUDED
#define VISUALIZATION_CONCRETE_TECPLOTASCIIFILEHEADERPARSER_INCLUDED

#include <string>
#include <vector>
#include <IO/ValueSource.h>

namespace Visualization {

namespace Concrete {

class TecplotASCIIFileHeaderParser:public IO::ValueSource
	{
	/* Embedded classes: */
	public:
	enum ZoneType // Enumerated type for zone types in Tecplot files
		{
		STRUCTURED,
		UNSTRUCTURED
		};
	
	enum ZoneLayout // Enumerated type for zone layouts in Tecplot files
		{
		INTERLEAVED,BLOCKED
		};
	
	enum ElementType // Enumerated type for unstructured element types
		{
		INVALID,
		TETRAHEDRON,
		HEXAHEDRON
		};
	
	/* Elements: */
	private:
	std::string title; // File's title
	std::vector<std::string> variables; // The list of variables defined in the file header
	std::string zoneName; // Name of the current zone
	ZoneType zoneType; // Type of the current zone
	ZoneLayout zoneLayout; // Data layout of the current zone
	int zoneSize[3]; // Size of the current structured zone
	ElementType zoneElementType; // Type of element if the current zone is unstructured
	int zoneNumVertices; // Number of vertices in the current unstructured zone
	int zoneNumElements; // Number of elements in the current unstructured zone
	
	/* Private methods: */
	bool readEqual(void); // Skips whitespace, checks for an equal sign, skips whitespace
	void parseZone(void); // Parses a zone header, after the ZONE keyword has been read
	
	/* Constructors and destructors: */
	public:
	TecplotASCIIFileHeaderParser(IO::FilePtr source); // Creates a parser for the given character source
	
	/* Methods: */
	std::string getTitle(void) const // Returns the file's title
		{
		return title;
		}
	unsigned int getNumVariables(void) const // Returns the number of variables contained in the file
		{
		return variables.size();
		}
	std::string getVariableName(unsigned int variableIndex) const // Returns the name of the given variable
		{
		return variables[variableIndex];
		}
	bool readNextZoneHeader(void); // Reads the next zone header; returns false at end of file
	
	/* Methods to query information about the current zone: */
	std::string getZoneName(void) const // Returns the name of the current zone
		{
		return zoneName;
		}
	ZoneType getZoneType(void) const // Returns the type of the current zone
		{
		return zoneType;
		}
	ZoneLayout getZoneLayout(void) const // Returns the layout of the current zone
		{
		return zoneLayout;
		}
	
	/* Methods to query information about structured zones: */
	const int* getZoneSize(void) const // Returns the size of the current zone
		{
		return zoneSize;
		}
	int getZoneSize(int dimension) const // Returns the size of the current zone in the given dimension
		{
		return zoneSize[dimension];
		}
	
	/* Methods to query information about unstructured zones: */
	ElementType getZoneElementType(void) const // Returns the element type of the current zone
		{
		return zoneElementType;
		}
	int getZoneNumVertices(void) const // Returns the number of vertices in the current zone
		{
		return zoneNumVertices;
		}
	int getZoneNumElements(void) const // Returns the number of elements in the current zone
		{
		return zoneNumElements;
		}
	
	/* Methods to read column values: */
	void readDoubles(int numValues,const bool ignoreFlags[],double values[]); // Reads an array of double values, but ignores those values whose ignoreFlag is true
	};

}

}

#endif
