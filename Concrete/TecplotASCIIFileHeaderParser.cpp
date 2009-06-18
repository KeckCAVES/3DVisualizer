/***********************************************************************
TecplotASCIIFileHeaderParser - Helper functions to parse the headers of
ASCII Tecplot files.
Copyright (c) 2009 Oliver Kreylos

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

#include <string.h>
#include <stdlib.h>
#include <Misc/ThrowStdErr.h>

#include <Concrete/TecplotASCIIFileHeaderParser.h>

namespace Visualization {

namespace Concrete {

/*********************************************
Methods of class TecplotASCIIFileHeaderParser:
*********************************************/

bool TecplotASCIIFileHeaderParser::readEqual(void)
	{
	skipWs();
	
	/* Check for the equal sign: */
	if(lastChar!='=')
		return false;
	readNextChar();
	
	skipWs();
	
	return true;
	}

void TecplotASCIIFileHeaderParser::parseZone(void)
	{
	/* Initialize the zone state: */
	zoneName.clear();
	zoneType=STRUCTURED;
	zoneLayout=INTERLEAVED;
	for(int i=0;i<3;++i)
		zoneSize[i]=-1;
	zoneElementType=INVALID;
	zoneNumVertices=-1;
	zoneNumElements=-1;
	
	/* Read tag/value pairs until end of line: */
	skipWsLine();
	while(lastChar>=0&&lastChar!='\n')
		{
		/* Read the tag: */
		std::string tag=readString();
		
		/* Check for the equal sign: */
		if(!readEqual())
			Misc::throwStdErr("Missing = in ZONE header");
		
		/* Read the tag value: */
		std::string value=readString();
		
		if(tag.length()==1)
			{
			char t=toupper(tag[0]);
			if(t=='T')
				zoneName=value;
			else if(t=='I'||t=='J'||t=='K')
				{
				zoneType=STRUCTURED;
				zoneSize[2-(int(t)-'I')]=atoi(value.c_str());
				}
			else if(t=='F')
				{
				const char* vPtr=value.c_str();
				if(strncasecmp(vPtr,"FE",2)==0)
					{
					zoneType=UNSTRUCTURED;
					vPtr+=2;
					}
				else
					zoneType=STRUCTURED;
				if(strcasecmp(vPtr,"POINT")==0)
					zoneLayout=INTERLEAVED;
				else if(strcasecmp(vPtr,"BLOCK")==0)
					zoneLayout=BLOCKED;
				else
					Misc::throwStdErr("Invalid format specifier %s",value.c_str());
				}
			else if(t=='N')
				{
				zoneType=UNSTRUCTURED;
				zoneNumVertices=atoi(value.c_str());
				}
			else if(t=='E')
				{
				zoneType=UNSTRUCTURED;
				zoneNumElements=atoi(value.c_str());
				}
			}
		else if(tag.length()==2)
			{
			char t0=toupper(tag[0]);
			char t1=toupper(tag[1]);
			if(t0=='E'&&t1=='T')
				{
				zoneType=UNSTRUCTURED;
				if(value=="BRICK")
					zoneElementType=HEXAHEDRON;
				else if(value=="TETRAHEDRON")
					zoneElementType=TETRAHEDRON;
				else
					Misc::throwStdErr("Unsupported element type %s",value.c_str());
				}
			}
		
		skipWsLine();
		if(lastChar==',')
			{
			/* Skip comma and whitespace, including newlines: */
			readNextChar();
			skipWs();
			}
		}
	if(lastChar=='\n')
		readNextChar();
	
	/* Check if the zone header is complete: */
	if(zoneType==STRUCTURED)
		{
		if(zoneSize[0]<0||zoneSize[1]<0||zoneSize[2]<0)
			Misc::throwStdErr("Missing structured zone size specification");
		}
	else
		{
		if(zoneElementType==INVALID)
			Misc::throwStdErr("Missing element type specification");
		if(zoneNumVertices<0||zoneNumElements<0)
			Misc::throwStdErr("Missing unstructured zone size specification");
		}
	}

TecplotASCIIFileHeaderParser::TecplotASCIIFileHeaderParser(const char* inputFileName)
	:Threads::ASCIIFileReader(inputFileName)
	{
	/* Set the punctuation characters: */
	setPunctuation(",=");
	
	/* Read the first zone header: */
	if(!readNextZoneHeader())
		Misc::throwStdErr("TecplotASCIIFileHeaderParser::TecplotASCIIFileHeaderParser: Malformed header in file %s",inputFileName);
	}

bool TecplotASCIIFileHeaderParser::readNextZoneHeader(void)
	{
	/* Read lines from the file until the next ZONE keyword is encountered: */
	while(true)
		{
		skipWs();
		if(lastChar<0)
			break;
		
		/* Check for comment lines: */
		if(lastChar!='#')
			{
			/* Read a keyword: */
			std::string keyword=readString();
			if(keyword=="TITLE")
				{
				/* Read the equal sign: */
				if(!readEqual())
					Misc::throwStdErr("Missing = after TITLE keyword");
				
				/* Read the title: */
				title=readString();
				}
			else if(keyword=="VARIABLES")
				{
				/* Read the equal sign: */
				if(!readEqual())
					Misc::throwStdErr("Missing = after VARIABLES keyword");
				
				/* Read all variables: */
				variables.clear();
				variables.push_back(readString());
				skipWs();
				while(lastChar==',')
					{
					/* Skip the comma and whitespace: */
					readNextChar();
					skipWs();
					
					/* Read the next variable name: */
					variables.push_back(readString());
					}
				}
			else if(keyword=="ZONE")
				{
				/* Parse the zone header: */
				parseZone();
				break;
				}
			}
		
		skipLine();
		}
	
	return lastChar>=0;
	}

void TecplotASCIIFileHeaderParser::readDoubles(int numValues,const bool ignoreFlags[],double values[])
	{
	char valueBuffer[64];
	for(int i=0;i<numValues;++i)
		{
		while(cc[lastChar]&WHITESPACE) // Breaks on non-space or EOF
			readNextChar();
		if(lastChar<0)
			Misc::throwStdErr("Unexpected end of file");
		
		if(ignoreFlags[i])
			{
			/* Skip the value: */
			while(cc[lastChar]&FLOATINGPOINT)
				readNextChar();
			}
		else
			{
			/* Read, parse, and store the value: */
			char* valuePtr=valueBuffer;
			while(cc[lastChar]&FLOATINGPOINT)
				{
				*valuePtr=lastChar;
				++valuePtr;
				readNextChar();
				}
			*valuePtr='\0';
			values[i]=atof(valueBuffer);
			}
		}
	}

}

}
