#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <Misc/File.h>

#include <Concrete/CitcomSCfgFileParser.h>

namespace Visualization {

namespace Concrete {

void
parseCitcomSCfgFile(
	const char* cfgFileName,
	std::string& dataDir,
	std::string& dataFileName,
	int& numSurfaces,
	Misc::ArrayIndex<3>& numCpus,
	Misc::ArrayIndex<3>& numVertices)
	{
	/* Open the run's configuration file: */
	Misc::File cfgFile(cfgFileName,"rt");
	bool inSolverSection=false;
	bool inSolverMesherSection=false;
	while(!cfgFile.eof())
		{
		/* Read the next line from the file: */
		char line[256];
		cfgFile.gets(line,sizeof(line));
		
		/* Skip initial whitespace: */
		char* linePtr;
		for(linePtr=line;*linePtr!='\0'&&isspace(*linePtr);++linePtr)
			;
		
		/* Check if it's a section header: */
		char* sectionStart=0;
		char* sectionEnd;
		if(*linePtr=='[')
			{
			for(sectionEnd=linePtr+1;*sectionEnd!='\0'&&*sectionEnd!=']';++sectionEnd)
				;
			if(*sectionEnd==']')
				{
				sectionStart=linePtr+1;
				*sectionEnd='\0';
				}
			}
		if(sectionStart!=0)
			{
			inSolverSection=strcasecmp(sectionStart,"CitcomS.solver")==0;
			inSolverMesherSection=strcasecmp(sectionStart,"CitcomS.solver.mesher")==0;
			}
		else
			{
			/* Look for known tag/value pairs: */
			char* tagStart=linePtr;
			char* eqPtr;
			for(eqPtr=linePtr;*eqPtr!='\0'&&*eqPtr!='=';++eqPtr)
				;
			if(*eqPtr=='=')
				{
				char* tagEnd;
				for(tagEnd=eqPtr;tagEnd>tagStart&&isspace(tagEnd[-1]);--tagEnd)
					;
				*tagEnd='\0';
				char* valueStart;
				for(valueStart=eqPtr+1;*valueStart!='\0'&&isspace(*valueStart);++valueStart)
					;
				char* valueEnd;
				for(valueEnd=valueStart;*valueEnd!='\0';++valueEnd)
					;
				for(;valueEnd>valueStart&&isspace(valueEnd[-1]);--valueEnd)
					;
				*valueEnd='\0';
				
				if(inSolverSection)
					{
					if(strcasecmp(tagStart,"datadir")==0)
						{
						if(*valueStart!='/')
							{
							/* Concatenate the given relative data directory to the directory containing the cfg file: */
							const char* slashPtr=0;
							for(const char* cfgPtr=cfgFileName;*cfgPtr!='\0';++cfgPtr)
								if(*cfgPtr=='/')
									slashPtr=cfgPtr;
							if(slashPtr==0)
								{
								/* Cfg file is in the current directory: */
								dataDir=valueStart;
								}
							else
								{
								/* Append the given path to the cfg file's path: */
								dataDir=std::string(cfgFileName,slashPtr+1-cfgFileName);
								dataDir.append(valueStart);
								}
							}
						else
							{
							/* Use the given absolute data directory: */
							dataDir=valueStart;
							}
						}
					else if(strcasecmp(tagStart,"datafile")==0)
						{
						/* Remember the data file's name: */
						dataFileName=valueStart;
						}
					}
				else if(inSolverMesherSection)
					{
					if(strcasecmp(tagStart,"nproc_surf")==0)
						numSurfaces=atoi(valueStart);
					else if(strcasecmp(tagStart,"nprocx")==0)
						numCpus[0]=atoi(valueStart);
					else if(strcasecmp(tagStart,"nprocy")==0)
						numCpus[1]=atoi(valueStart);
					else if(strcasecmp(tagStart,"nprocz")==0)
						numCpus[2]=atoi(valueStart);
					else if(strcasecmp(tagStart,"nodex")==0)
						numVertices[0]=atoi(valueStart);
					else if(strcasecmp(tagStart,"nodey")==0)
						numVertices[1]=atoi(valueStart);
					else if(strcasecmp(tagStart,"nodez")==0)
						numVertices[2]=atoi(valueStart);
					}
				}
			}
		}
	}

}

}
