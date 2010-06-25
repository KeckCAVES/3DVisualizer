/***********************************************************************
Visualizer - Test application for the new visualization component
framework.
Copyright (c) 2005-2009 Oliver Kreylos

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

#include "Visualizer.h"

#include <ctype.h>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/File.h>
#include <Comm/MulticastPipe.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/Separator.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <Vrui/Vrui.h>
#ifdef VISUALIZER_USE_COLLABORATION
#include <Collaboration/CollaborationClient.h>
#endif

#include <Abstract/DataSetRenderer.h>
#include <Abstract/CoordinateTransformer.h>
#include <Abstract/VariableManager.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>
#include <Abstract/Module.h>

#include "CuttingPlane.h"
#ifdef VISUALIZER_USE_COLLABORATION
#include "SharedVisualizationClient.h"
#endif
#include "BaseLocator.h"
#include "CuttingPlaneLocator.h"
#include "ScalarEvaluationLocator.h"
#include "VectorEvaluationLocator.h"
#include "ExtractorLocator.h"
#include "ElementList.h"

namespace {

/****************
Helper functions:
****************/

std::string readToken(Misc::File& file,int& nextChar)
	{
	/* Skip whitespace: */
	while(nextChar!=EOF&&isspace(nextChar))
		nextChar=file.getc();
	
	/* Read the next token: */
	std::string result="";
	if(nextChar=='"')
		{
		/* Skip the opening quote: */
		nextChar=file.getc();
		
		/* Read a quoted token: */
		while(nextChar!=EOF&&nextChar!='"')
			{
			result+=char(nextChar);
			nextChar=file.getc();
			}
		
		if(nextChar=='"')
			nextChar=file.getc();
		else
			Misc::throwStdErr("unterminated quoted token in input file");
		}
	else
		{
		/* Read an unquoted token: */
		while(nextChar!=EOF&&!isspace(nextChar))
			{
			result+=char(nextChar);
			nextChar=file.getc();
			}
		}
	
	return result;
	}

}

/***************************
Methods of class Visualizer:
***************************/

GLMotif::Popup* Visualizer::createRenderingModesMenu(void)
	{
	GLMotif::Popup* renderingModesMenuPopup=new GLMotif::Popup("RenderingModesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* renderingModes=new GLMotif::RadioBox("RenderingModes",renderingModesMenuPopup,false);
	renderingModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	int numRenderingModes=dataSetRenderer->getNumRenderingModes();
	for(int i=0;i<numRenderingModes;++i)
		renderingModes->addToggle(dataSetRenderer->getRenderingModeName(i));
	
	renderingModes->setSelectedToggle(dataSetRenderer->getRenderingMode());
	renderingModes->getValueChangedCallbacks().add(this,&Visualizer::changeRenderingModeCallback);
	
	renderingModes->manageChild();
	
	return renderingModesMenuPopup;
	}

GLMotif::Popup* Visualizer::createScalarVariablesMenu(void)
	{
	GLMotif::Popup* scalarVariablesMenuPopup=new GLMotif::Popup("ScalarVariablesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* scalarVariables=new GLMotif::RadioBox("ScalarVariables",scalarVariablesMenuPopup,false);
	scalarVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	for(int i=0;i<variableManager->getNumScalarVariables();++i)
		scalarVariables->addToggle(variableManager->getScalarVariableName(i));
	
	scalarVariables->setSelectedToggle(variableManager->getCurrentScalarVariable());
	scalarVariables->getValueChangedCallbacks().add(this,&Visualizer::changeScalarVariableCallback);
	
	scalarVariables->manageChild();
	
	return scalarVariablesMenuPopup;
	}

GLMotif::Popup* Visualizer::createVectorVariablesMenu(void)
	{
	GLMotif::Popup* vectorVariablesMenuPopup=new GLMotif::Popup("VectorVariablesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* vectorVariables=new GLMotif::RadioBox("VectorVariables",vectorVariablesMenuPopup,false);
	vectorVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	for(int i=0;i<variableManager->getNumVectorVariables();++i)
		vectorVariables->addToggle(variableManager->getVectorVariableName(i));
	
	vectorVariables->setSelectedToggle(variableManager->getCurrentVectorVariable());
	vectorVariables->getValueChangedCallbacks().add(this,&Visualizer::changeVectorVariableCallback);
	
	vectorVariables->manageChild();
	
	return vectorVariablesMenuPopup;
	}

GLMotif::Popup* Visualizer::createAlgorithmsMenu(void)
	{
	GLMotif::Popup* algorithmsMenuPopup=new GLMotif::Popup("AlgorithmsMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* algorithms=new GLMotif::RadioBox("Algorithms",algorithmsMenuPopup,false);
	algorithms->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	/* Add the cutting plane algorithm: */
	int algorithmIndex=0;
	algorithms->addToggle("Cutting Plane");
	++algorithmIndex;
	
	if(variableManager->getNumScalarVariables()>0)
		{
		/* Add the scalar evaluator algorithm: */
		algorithms->addToggle("Evaluate Scalars");
		++algorithmIndex;
		
		/* Add scalar algorithms: */
		firstScalarAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumScalarAlgorithms();++i)
			{
			algorithms->addToggle(module->getScalarAlgorithmName(i));
			++algorithmIndex;
			}
		}
	
	if(variableManager->getNumVectorVariables()>0)
		{
		/* Add the vector evaluator algorithm: */
		algorithms->addToggle("Evaluate Vectors");
		++algorithmIndex;
		
		/* Add vector algorithms: */
		firstVectorAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumVectorAlgorithms();++i)
			{
			algorithms->addToggle(module->getVectorAlgorithmName(i));
			++algorithmIndex;
			}
		}
		
	algorithms->setSelectedToggle(algorithm);
	algorithms->getValueChangedCallbacks().add(this,&Visualizer::changeAlgorithmCallback);
	
	algorithms->manageChild();
	
	return algorithmsMenuPopup;
	}

GLMotif::Popup* Visualizer::createElementsMenu(void)
	{
	GLMotif::Popup* elementsMenuPopup=new GLMotif::Popup("ElementsMenuPopup",Vrui::getWidgetManager());
	
	/* Create the elements menu: */
	GLMotif::SubMenu* elementsMenu=new GLMotif::SubMenu("ElementsMenu",elementsMenuPopup,false);
	
	showElementListToggle=new GLMotif::ToggleButton("ShowElementListToggle",elementsMenu,"Show Element List");
	showElementListToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementListCallback);
	
	GLMotif::Button* loadElementsButton=new GLMotif::Button("LoadElementsButton",elementsMenu,"Load Visualization Elements");
	loadElementsButton->getSelectCallbacks().add(this,&Visualizer::loadElementsCallback);
	
	GLMotif::Button* saveElementsButton=new GLMotif::Button("SaveElementsButton",elementsMenu,"Save Visualization Elements");
	saveElementsButton->getSelectCallbacks().add(this,&Visualizer::saveElementsCallback);
	
	new GLMotif::Separator("ClearElementsSeparator",elementsMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::Button* clearElementsButton=new GLMotif::Button("ClearElementsButton",elementsMenu,"Clear Visualization Elements");
	clearElementsButton->getSelectCallbacks().add(this,&Visualizer::clearElementsCallback);
	
	elementsMenu->manageChild();
	
	return elementsMenuPopup;
	}

GLMotif::Popup* Visualizer::createStandardLuminancePalettesMenu(void)
	{
	GLMotif::Popup* standardLuminancePalettesMenuPopup=new GLMotif::Popup("StandardLuminancePalettesMenuPopup",Vrui::getWidgetManager());
	
	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardLuminancePalettes=new GLMotif::SubMenu("StandardLuminancePalettes",standardLuminancePalettesMenuPopup,false);
	
	standardLuminancePalettes->addEntry("Grey");
	standardLuminancePalettes->addEntry("Red");
	standardLuminancePalettes->addEntry("Yellow");
	standardLuminancePalettes->addEntry("Green");
	standardLuminancePalettes->addEntry("Cyan");
	standardLuminancePalettes->addEntry("Blue");
	standardLuminancePalettes->addEntry("Magenta");
	
	standardLuminancePalettes->getEntrySelectCallbacks().add(this,&Visualizer::createStandardLuminancePaletteCallback);
	
	standardLuminancePalettes->manageChild();
	
	return standardLuminancePalettesMenuPopup;
	}

GLMotif::Popup* Visualizer::createStandardSaturationPalettesMenu(void)
	{
	GLMotif::Popup* standardSaturationPalettesMenuPopup=new GLMotif::Popup("StandardSaturationPalettesMenuPopup",Vrui::getWidgetManager());
	
	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardSaturationPalettes=new GLMotif::SubMenu("StandardSaturationPalettes",standardSaturationPalettesMenuPopup,false);
	
	standardSaturationPalettes->addEntry("Red -> Cyan");
	standardSaturationPalettes->addEntry("Yellow -> Blue");
	standardSaturationPalettes->addEntry("Green -> Magenta");
	standardSaturationPalettes->addEntry("Cyan -> Red");
	standardSaturationPalettes->addEntry("Blue -> Yellow");
	standardSaturationPalettes->addEntry("Magenta -> Green");
	standardSaturationPalettes->addEntry("Rainbow");
	
	standardSaturationPalettes->getEntrySelectCallbacks().add(this,&Visualizer::createStandardSaturationPaletteCallback);
	
	standardSaturationPalettes->manageChild();
	
	return standardSaturationPalettesMenuPopup;
	}

GLMotif::Popup* Visualizer::createColorMenu(void)
	{
	GLMotif::Popup* colorMenuPopup=new GLMotif::Popup("ColorMenuPopup",Vrui::getWidgetManager());
	
	/* Create the color menu and add entries for all standard palettes: */
	GLMotif::SubMenu* colorMenu=new GLMotif::SubMenu("ColorMenu",colorMenuPopup,false);
	
	GLMotif::CascadeButton* standardLuminancePalettesCascade=new GLMotif::CascadeButton("StandardLuminancePalettesCascade",colorMenu,"Create Luminance Palette");
	standardLuminancePalettesCascade->setPopup(createStandardLuminancePalettesMenu());
	
	GLMotif::CascadeButton* standardSaturationPalettesCascade=new GLMotif::CascadeButton("StandardSaturationPalettesCascade",colorMenu,"Create Saturation Palette");
	standardSaturationPalettesCascade->setPopup(createStandardSaturationPalettesMenu());
	
	GLMotif::Button* loadPaletteButton=new GLMotif::Button("LoadPaletteButton",colorMenu,"Load Palette File");
	loadPaletteButton->getSelectCallbacks().add(this,&Visualizer::loadPaletteCallback);
	
	GLMotif::ToggleButton* showColorBarToggle=new GLMotif::ToggleButton("ShowColorBarToggle",colorMenu,"Show Color Bar");
	showColorBarToggle->getValueChangedCallbacks().add(this,&Visualizer::showColorBarCallback);
	
	GLMotif::ToggleButton* showPaletteEditorToggle=new GLMotif::ToggleButton("ShowPaletteEditorToggle",colorMenu,"Show Palette Editor");
	showPaletteEditorToggle->getValueChangedCallbacks().add(this,&Visualizer::showPaletteEditorCallback);
	
	colorMenu->manageChild();
	
	return colorMenuPopup;
	}

GLMotif::PopupMenu* Visualizer::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("3D Visualizer");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::CascadeButton* renderingModesCascade=new GLMotif::CascadeButton("RenderingModesCascade",mainMenu,"Rendering Modes");
	renderingModesCascade->setPopup(createRenderingModesMenu());
	
	if(variableManager->getNumScalarVariables()>0)
		{
		GLMotif::CascadeButton* scalarVariablesCascade=new GLMotif::CascadeButton("ScalarVariablesCascade",mainMenu,"Scalar Variables");
		scalarVariablesCascade->setPopup(createScalarVariablesMenu());
		}
	
	if(variableManager->getNumVectorVariables()>0)
		{
		GLMotif::CascadeButton* vectorVariablesCascade=new GLMotif::CascadeButton("VectorVariablesCascade",mainMenu,"Vector Variables");
		vectorVariablesCascade->setPopup(createVectorVariablesMenu());
		}
	
	GLMotif::CascadeButton* algorithmsCascade=new GLMotif::CascadeButton("AlgorithmsCascade",mainMenu,"Algorithms");
	algorithmsCascade->setPopup(createAlgorithmsMenu());
	
	GLMotif::CascadeButton* elementsCascade=new GLMotif::CascadeButton("ElementsCascade",mainMenu,"Elements");
	elementsCascade->setPopup(createElementsMenu());
	
	GLMotif::CascadeButton* colorCascade=new GLMotif::CascadeButton("ColorCascade",mainMenu,"Color Maps");
	colorCascade->setPopup(createColorMenu());
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&Visualizer::centerDisplayCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void Visualizer::loadElements(const char* elementFileName,bool ascii)
	{
	/* Open a pipe for cluster communication: */
	Comm::MulticastPipe* pipe=Vrui::openPipe();
	
	if(pipe==0||pipe->isMaster())
		{
		/* Open the element file: */
		Misc::File elementFile(elementFileName,ascii?"r":"rb",ascii?Misc::File::DontCare:Misc::File::LittleEndian);
		
		/* Read all elements from the file: */
		while(true)
			{
			/* Read the next algorithm name: */
			unsigned int nameLength=0;
			char name[256];
			if(ascii)
				{
				/* Skip whitespace: */
				int nextChar;
				while((nextChar=elementFile.getc())!=EOF&&isspace(nextChar))
					;
				
				/* Read until end of line: */
				while(nextChar!='\n'&&nextChar!=EOF)
					{
					name[nameLength]=char(nextChar);
					++nameLength;
					nextChar=elementFile.getc();
					}
				
				if(nextChar==EOF) // Check for end-of-file indicator
					{
					/* Tell the slaves to bail out: */
					if(pipe!=0)
						pipe->write<unsigned int>(0);
					
					/* Bail out: */
					break;
					}
				
				/* Remove trailing whitespace: */
				while(isspace(name[nameLength-1]))
					--nameLength;
				}
			else
				{
				nameLength=elementFile.read<unsigned int>();
				if(nameLength==0) // Check for end-of-file indicator
					{
					/* Tell the slaves to bail out: */
					if(pipe!=0)
						pipe->write<unsigned int>(0);
					
					/* Bail out: */
					break;
					}
				elementFile.read(name,nameLength);
				}
			name[nameLength]='\0';
			
			if(pipe!=0)
				{
				/* Send the algorithm name to the slaves: */
				pipe->write<unsigned int>(nameLength);
				pipe->write(name,nameLength);
				}
			
			/* Create an extractor for the given name: */
			Algorithm* algorithm=0;
			for(int i=0;algorithm==0&&i<module->getNumScalarAlgorithms();++i)
				if(strcmp(name,module->getScalarAlgorithmName(i))==0)
					algorithm=module->getScalarAlgorithm(i,variableManager,Vrui::openPipe());
			for(int i=0;algorithm==0&&i<module->getNumVectorAlgorithms();++i)
				if(strcmp(name,module->getVectorAlgorithmName(i))==0)
					algorithm=module->getVectorAlgorithm(i,variableManager,Vrui::openPipe());
			
			/* Extract an element using the given extractor: */
			if(algorithm!=0)
				{
				std::cout<<"Creating "<<name<<"..."<<std::flush;
				Misc::Timer extractionTimer;
				
				/* Read the element's extraction parameters from the file: */
				Parameters* parameters=algorithm->cloneParameters();
				parameters->read(elementFile,ascii,variableManager);
				
				if(pipe!=0)
					{
					/* Send the extraction parameters to the slaves: */
					parameters->write(*pipe,variableManager);
					pipe->finishMessage();
					}
				
				/* Extract the element: */
				Element* element=algorithm->createElement(parameters);
				
				/* Store the element: */
				elementList->addElement(element,name);
				
				/* Destroy the extractor: */
				delete algorithm;
				
				extractionTimer.elapse();
				std::cout<<" done in "<<extractionTimer.getTime()*1000.0<<" ms"<<std::endl;
				}
			}
		}
	else
		{
		/* Receive all visualization elements from the master: */
		while(true)
			{
			/* Receive the algorithm name from the master: */
			unsigned int nameLength=pipe->read<unsigned int>();
			if(nameLength==0) // Check for end-of-file indicator
				break;
			char name[256];
			pipe->read(name,nameLength);
			name[nameLength]=0;
			
			/* Create an extractor for the given name: */
			Algorithm* algorithm=0;
			for(int i=0;algorithm==0&&i<module->getNumScalarAlgorithms();++i)
				if(strcmp(name,module->getScalarAlgorithmName(i))==0)
					algorithm=module->getScalarAlgorithm(i,variableManager,Vrui::openPipe());
			for(int i=0;algorithm==0&&i<module->getNumVectorAlgorithms();++i)
				if(strcmp(name,module->getVectorAlgorithmName(i))==0)
					algorithm=module->getVectorAlgorithm(i,variableManager,Vrui::openPipe());
			
			/* Extract an element using the given extractor: */
			if(algorithm!=0)
				{
				/* Receive the extraction parameters: */
				Parameters* parameters=algorithm->cloneParameters();
				parameters->read(*pipe,variableManager);
				
				/* Receive the element: */
				Element* element=algorithm->startSlaveElement(parameters);
				algorithm->continueSlaveElement();
				
				/* Store the element: */
				elementList->addElement(element,name);
				
				/* Destroy the extractor: */
				delete algorithm;
				}
			}
		}
	
	if(pipe!=0)
		{
		/* Close the communication pipe: */
		delete pipe;
		}
	}

Visualizer::Visualizer(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 moduleManager(VISUALIZER_MODULENAMETEMPLATE),
	 module(0),dataSet(0),variableManager(0),
	 dataSetRenderer(0),coordinateTransformer(0),
	 firstScalarAlgorithmIndex(0),firstVectorAlgorithmIndex(0),
	 #ifdef VISUALIZER_USE_COLLABORATION
	 collaborationClient(0),sharedVisualizationClient(0),
	 #endif
	 numCuttingPlanes(0),cuttingPlanes(0),
	 elementList(0),
	 algorithm(0),
	 mainMenu(0),
	 showElementListToggle(0),
	 inLoadPalette(false),inLoadElements(false)
	{
	/* Parse the command line: */
	std::string moduleClassName="";
	std::vector<std::string> dataSetArgs;
	const char* argColorMapName=0;
	std::vector<const char*> loadFileNames;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"class")==0)
				{
				/* Get visualization module class name and data set arguments from command line: */
				++i;
				if(i>=argc)
					Misc::throwStdErr("Visualizer::Visualizer: missing module class name after -class");
				moduleClassName=argv[i];
				++i;
				while(i<argc&&strcmp(argv[i],";")!=0)
					{
					dataSetArgs.push_back(argv[i]);
					++i;
					}
				}
			else if(strcasecmp(argv[i]+1,"palette")==0)
				{
				++i;
				if(i<argc)
					argColorMapName=argv[i];
				else
					std::cerr<<"Missing palette file name after -palette"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"load")==0)
				{
				++i;
				if(i<argc)
					{
					/* Load an element file later: */
					loadFileNames.push_back(argv[i]);
					}
				else
					std::cerr<<"Missing element file name after -load"<<std::endl;
				}
			#ifdef VISUALIZER_USE_COLLABORATION
			else if(strcasecmp(argv[i]+1,"share")==0)
				{
				try
					{
					/* Create a configuration object: */
					Collaboration::CollaborationClient::Configuration* cfg=new Collaboration::CollaborationClient::Configuration;
					
					/* Check if the next argument is a server name: */
					if(i+2<argc&&strcasecmp(argv[i+1],"-server")==0)
						{
						i+=2;
						
						/* Split the server name into host name and port ID: */
						char* colonPtr=0;
						for(char* sPtr=argv[i];*sPtr!='\0';++sPtr)
							if(*sPtr==':')
								colonPtr=sPtr;
						if(colonPtr!=0)
							cfg->setServer(std::string(argv[i],colonPtr),atoi(colonPtr+1));
						else
							{
							/* Use the default port: */
							cfg->setServer(argv[i],26000);
							}
						}
					
					/* Create the collaboration client: */
					collaborationClient=new Collaboration::CollaborationClient(cfg);
					
					/* Register the shared Visualizer protocol: */
					collaborationClient->registerProtocol(new Collaboration::SharedVisualizationClient(this));
					}
				catch(std::runtime_error err)
					{
					std::cerr<<"Caught exception "<<err.what()<<" while creating shared Visualizer client"<<std::endl;
					delete collaborationClient;
					collaborationClient=0;
					}
				}
			#endif
			}
		else
			{
			/* Read the meta-input file of the given name: */
			Misc::File inputFile(argv[i],"rt");
			
			/* Parse the meta-input file: */
			int nextChar=inputFile.getc();
			
			/* Read the module class name: */
			moduleClassName=readToken(inputFile,nextChar);
			
			/* Read the data set arguments: */
			dataSetArgs.clear();
			while(true)
				{
				/* Read the next argument: */
				std::string arg=readToken(inputFile,nextChar);
				if(arg=="")
					break;

				/* Store the argument in the list: */
				dataSetArgs.push_back(arg);
				}
			}
		}
	
	/* Check if a module class name and data set arguments were provided: */
	if(moduleClassName=="")
		Misc::throwStdErr("Visualizer::Visualizer: no visualization module class name provided");
	if(dataSetArgs.empty())
		Misc::throwStdErr("Visualizer::Visualizer: no data set arguments provided");
	
	/* Load a visualization module and a data set: */
	try
		{
		/* Load the appropriate visualization module: */
		module=moduleManager.loadClass(moduleClassName.c_str());
		
		/* Load a data set: */
		Misc::Timer t;
		Comm::MulticastPipe* pipe=Vrui::openPipe(); // Implicit synchronization point
		dataSet=module->load(dataSetArgs,pipe);
		delete pipe; // Implicit synchronization point
		t.elapse();
		if(Vrui::isMaster())
			std::cout<<"Time to load data set: "<<t.getTime()*1000.0<<" ms"<<std::endl;
		}
	catch(std::runtime_error err)
		{
		Misc::throwStdErr("Visualizer::Visualizer: Could not load data set due to exception %s",err.what());
		}
	
	/* Create a variable manager: */
	variableManager=new VariableManager(dataSet,argColorMapName);
	
	/* Determine the color to render the data set: */
	for(int i=0;i<3;++i)
		dataSetRenderColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	dataSetRenderColor[3]=0.2f;
	
	/* Create a data set renderer: */
	dataSetRenderer=module->getRenderer(dataSet);
	
	/* Get the data set's coordinate transformer: */
	coordinateTransformer=dataSet->getCoordinateTransformer();
	
	/* Create cutting planes: */
	numCuttingPlanes=6;
	cuttingPlanes=new CuttingPlane[numCuttingPlanes];
	for(size_t i=0;i<numCuttingPlanes;++i)
		{
		cuttingPlanes[i].allocated=false;
		cuttingPlanes[i].active=false;
		}
	
	#ifdef VISUALIZER_USE_COLLABORATION
	if(collaborationClient!=0)
		{
		try
			{
			/* Connect to the server: */
			collaborationClient->connect();
			}
		catch(std::runtime_error err)
			{
			std::cerr<<"Caught exception "<<err.what()<<" while connecting to shared Visualizer server"<<std::endl;
			delete collaborationClient;
			collaborationClient=0;
			}
		
		/* Get a pointer to the shared Visualizer protocol: */
		sharedVisualizationClient=dynamic_cast<Collaboration::SharedVisualizationClient*>(collaborationClient->getProtocol(Collaboration::SharedVisualizationClient::protocolName));
		}
	#endif
	
	/* Create the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Create the element list: */
	elementList=new ElementList(Vrui::getWidgetManager());
	
	/* Load all element files listed on the command line: */
	for(std::vector<const char*>::const_iterator lfnIt=loadFileNames.begin();lfnIt!=loadFileNames.end();++lfnIt)
		{
		/* Determine the type of the element file: */
		if(Misc::hasCaseExtension(*lfnIt,".asciielem"))
			{
			/* Load an ASCII elements file: */
			loadElements(*lfnIt,true);
			}
		else if(Misc::hasCaseExtension(*lfnIt,".binelem"))
			{
			/* Load a binary elements file: */
			loadElements(*lfnIt,false);
			}
		}
	
	/* Initialize navigation transformation: */
	centerDisplayCallback(0);
	}

Visualizer::~Visualizer(void)
	{
	delete mainMenu;
	
	/* Delete all finished visualization elements: */
	delete elementList;
	
	/* Delete all locators: */
	for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		delete *blIt;
	
	/* Delete the cutting planes: */
	delete[] cuttingPlanes;
	
	#ifdef VISUALIZER_USE_COLLABORATION
	/* Delete a shared visualization client: */
	delete collaborationClient;
	#endif
	
	/* Delete the coordinate transformer: */
	delete coordinateTransformer;
	
	/* Delete the data set renderer: */
	delete dataSetRenderer;
	
	/* Delete the variable manager: */
	delete variableManager;
	
	/* Delete the data set: */
	delete dataSet;
	}

void Visualizer::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		BaseLocator* newLocator;
		if(algorithm==0)
			{
			/* Create a cutting plane locator object and associate it with the new tool: */
			newLocator=new CuttingPlaneLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex)
			{
			/* Create a scalar evaluation locator object and associate it with the new tool: */
			newLocator=new ScalarEvaluationLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex+module->getNumScalarAlgorithms())
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstScalarAlgorithmIndex;
			Algorithm* extractor=module->getScalarAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new ExtractorLocator(locatorTool,this,extractor);
			}
		else if(algorithm<firstVectorAlgorithmIndex)
			{
			/* Create a vector evaluation locator object and associate it with the new tool: */
			newLocator=new VectorEvaluationLocator(locatorTool,this);
			}
		else
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstVectorAlgorithmIndex;
			Algorithm* extractor=module->getVectorAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new ExtractorLocator(locatorTool,this,extractor);
			}
		
		/* Add new locator to list: */
		baseLocators.push_back(newLocator);
		}
	}

void Visualizer::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		/* Find the data locator associated with the tool in the list: */
		for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
			if((*blIt)->getTool()==locatorTool)
				{
				/* Remove the locator: */
				delete *blIt;
				baseLocators.erase(blIt);
				break;
				}
		}
	}

void Visualizer::frame(void)
	{
	#ifdef VISUALIZER_USE_COLLABORATION
	if(collaborationClient!=0)
		{
		/* Call the collaboratoin client's frame method: */
		collaborationClient->frame();
		}
	#endif
	}

void Visualizer::display(GLContextData& contextData) const
	{
	#ifdef VISUALIZER_USE_COLLABORATION
	if(collaborationClient!=0)
		{
		/* Call the collaboration client's display method: */
		collaborationClient->display(contextData);
		}
	#endif
	
	/* Highlight all locators: */
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->highlightLocator(contextData);
	
	/* Enable all cutting planes: */
	int numSupportedCuttingPlanes;
	glGetIntegerv(GL_MAX_CLIP_PLANES,&numSupportedCuttingPlanes);
	int cuttingPlaneIndex=0;
	for(size_t i=0;i<numCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Enable the cutting plane: */
			glEnable(GL_CLIP_PLANE0+cuttingPlaneIndex);
			GLdouble cuttingPlane[4];
			for(int j=0;j<3;++j)
				cuttingPlane[j]=cuttingPlanes[i].plane.getNormal()[j];
			cuttingPlane[3]=-cuttingPlanes[i].plane.getOffset();
			glClipPlane(GL_CLIP_PLANE0+cuttingPlaneIndex,cuttingPlane);
			
			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}
	
	/* Render all opaque visualization elements: */
	elementList->renderElements(contextData,false);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderAction(contextData);

	/* Render all transparent visualization elements: */
	elementList->renderElements(contextData,true);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderActionTransparent(contextData);
	
	/* Render the data set: */
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	if(lineWidth!=1.0f)
		glLineWidth(1.0f);
	glColor(dataSetRenderColor);
	dataSetRenderer->glRenderAction(contextData);
	glLineWidth(lineWidth);
	
	/* Disable all cutting planes: */
	cuttingPlaneIndex=0;
	for(size_t i=0;i<numCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Disable the cutting plane: */
			glDisable(GL_CLIP_PLANE0+cuttingPlaneIndex);
			
			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}
	}

void Visualizer::changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new rendering mode: */
	dataSetRenderer->setRenderingMode(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	}

void Visualizer::changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	if(!inLoadPalette)
		{
		/* Set the new scalar variable: */
		variableManager->setCurrentScalarVariable(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
		}
	}

void Visualizer::changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new vector variable: */
	variableManager->setCurrentVectorVariable(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	}

void Visualizer::changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new algorithm: */
	algorithm=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	}

void Visualizer::loadPaletteCallback(Misc::CallbackData*)
	{
	if(!inLoadPalette)
		{
		/* Create a file selection dialog to select a palette file: */
		GLMotif::FileSelectionDialog* fsDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Palette File...",0,".pal",Vrui::openPipe());
		fsDialog->getOKCallbacks().add(this,&Visualizer::loadPaletteOKCallback);
		fsDialog->getCancelCallbacks().add(this,&Visualizer::loadPaletteCancelCallback);
		Vrui::popupPrimaryWidget(fsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
		inLoadPalette=true;
		}
	}

void Visualizer::loadPaletteOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	try
		{
		/* Load the palette file: */
		variableManager->loadPalette(cbData->selectedFileName.c_str());
		}
	catch(std::runtime_error)
		{
		/* Ignore the error... */
		}
	
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadPalette=false;
	}

void Visualizer::loadPaletteCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData)
	{
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadPalette=false;
	}

void Visualizer::showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show color bar dialog based on toggle button state: */
	variableManager->showColorBar(cbData->set);
	}

void Visualizer::showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show palette editor based on toggle button state: */
	variableManager->showPaletteEditor(cbData->set);
	}

void Visualizer::createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
	{
	if(!inLoadPalette)
		variableManager->createPalette(VariableManager::LUMINANCE_GREY+cbData->menu->getEntryIndex(cbData->selectedButton));
	}

void Visualizer::createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
	{
	if(!inLoadPalette)
		variableManager->createPalette(VariableManager::SATURATION_RED_CYAN+cbData->menu->getEntryIndex(cbData->selectedButton));
	}

void Visualizer::showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show element list based on toggle button state: */
	if(cbData->set)
		elementList->showElementList(Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
	else
		elementList->hideElementList();
	}

void Visualizer::loadElementsCallback(Misc::CallbackData*)
	{
	if(!inLoadElements)
		{
		/* Create a file selection dialog to select an element file: */
		GLMotif::FileSelectionDialog* fsDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Visualization Elements...",0,".asciielem;.binelem",Vrui::openPipe());
		fsDialog->getOKCallbacks().add(this,&Visualizer::loadElementsOKCallback);
		fsDialog->getCancelCallbacks().add(this,&Visualizer::loadElementsCancelCallback);
		Vrui::popupPrimaryWidget(fsDialog,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
		inLoadElements=true;
		}
	}

void Visualizer::loadElementsOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	try
		{
		/* Determine the type of the element file: */
		if(Misc::hasCaseExtension(cbData->selectedFileName.c_str(),".asciielem"))
			{
			/* Load the ASCII elements file: */
			loadElements(cbData->selectedFileName.c_str(),true);
			}
		else if(Misc::hasCaseExtension(cbData->selectedFileName.c_str(),".binelem"))
			{
			/* Load the binary elements file: */
			loadElements(cbData->selectedFileName.c_str(),false);
			}
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<" while loading element file"<<std::endl;
		}
	
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadElements=false;
	}

void Visualizer::loadElementsCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData)
	{
	/* Destroy the file selection dialog: */
	Vrui::getWidgetManager()->deleteWidget(cbData->fileSelectionDialog);
	inLoadElements=false;
	}

void Visualizer::saveElementsCallback(Misc::CallbackData*)
	{
	if(Vrui::isMaster())
		{
		#if 1
		/* Create the ASCII element file: */
		char elementFileNameBuffer[256];
		Misc::createNumberedFileName("SavedElements.asciielem",4,elementFileNameBuffer);
		
		/* Save the visible elements to a binary file: */
		elementList->saveElements(elementFileNameBuffer,true,variableManager);
		#else
		/* Create the binary element file: */
		char elementFileNameBuffer[256];
		Misc::createNumberedFileName("SavedElements.binelem",4,elementFileNameBuffer);
		
		/* Save the visible elements to a binary file: */
		elementList->saveElements(elementFileNameBuffer,false,variableManager);
		#endif
		}
	}

void Visualizer::clearElementsCallback(Misc::CallbackData*)
	{
	/* Delete all finished visualization elements: */
	elementList->clear();
	}

void Visualizer::centerDisplayCallback(Misc::CallbackData*)
	{
	/* Get the data set's domain box: */
	DataSet::Box domain=dataSet->getDomainBox();
	Vrui::Point center=Geometry::mid(domain.min,domain.max);
	Vrui::Scalar radius=Geometry::dist(domain.min,domain.max);
	
	Vrui::setNavigationTransformation(center,radius);
	}

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		Visualizer iso(argc,argv,appDefaults);
		iso.run();
		return 0;
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	}
