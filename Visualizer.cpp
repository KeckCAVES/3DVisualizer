/***********************************************************************
Visualizer - Test application for the new visualization component
framework.
Copyright (c) 2005-2008 Oliver Kreylos

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
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/File.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Menu.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSetRenderer.h>
#include <Abstract/CoordinateTransformer.h>
#include <Abstract/VariableManager.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>
#include <Abstract/Module.h>

#include "Visualizer.h"

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
	
	GLMotif::CascadeButton* colorCascade=new GLMotif::CascadeButton("ColorCascade",mainMenu,"Color Maps");
	colorCascade->setPopup(createColorMenu());
	
	showElementListToggle=new GLMotif::ToggleButton("ShowElementListToggle",mainMenu,"Show Element List");
	showElementListToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementListCallback);
	
	GLMotif::Button* clearElementsButton=new GLMotif::Button("ClearElementsButton",mainMenu,"Clear Visualization Elements");
	clearElementsButton->getSelectCallbacks().add(this,&Visualizer::clearElementsCallback);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&Visualizer::centerDisplayCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* Visualizer::createElementListDialog(void)
	{
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* elementListDialogPopup=new GLMotif::PopupWindow("ElementListDialogPopup",Vrui::getWidgetManager(),"Visualization Element List");
	elementListDialogPopup->setResizableFlags(false,false);
	
	elementListDialog=new GLMotif::RowColumn("ElementListDialog",elementListDialogPopup,false);
	elementListDialog->setNumMinorWidgets(3);
	
	elementListDialog->manageChild();
	
	return elementListDialogPopup;
	}


void Visualizer::addElement(Visualizer::Element* newElement)
	{
	/* Create the element's list structure: */
	ListElement le;
	le.element=newElement;
	le.name=newElement->getName();
	le.settingsDialog=newElement->createSettingsDialog(Vrui::getWidgetManager());
	le.settingsDialogVisible=false;
	le.show=true;
	
	/* Add the element to the list: */
	elements.push_back(le);
	
	/* Create an entry in the element list dialog: */
	GLMotif::TextField* elementName=new GLMotif::TextField("ElementName",elementListDialog,20);
	elementName->setHAlignment(GLFont::Left);
	elementName->setLabel(le.name.c_str());
	
	GLMotif::ToggleButton* showSettingsDialogToggle=new GLMotif::ToggleButton("ShowSettingsDialogToggle",elementListDialog,"Show Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementSettingsDialogCallback);
	
	GLMotif::ToggleButton* showElementToggle=new GLMotif::ToggleButton("ShowElementToggle",elementListDialog,"Show");
	showElementToggle->setToggle(le.show);
	showElementToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementCallback);
	}

Visualizer::Visualizer(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 moduleManager(VISUALIZER_MODULENAMETEMPLATE),
	 module(0),dataSet(0),variableManager(0),
	 dataSetRenderer(0),coordinateTransformer(0),
	 firstScalarAlgorithmIndex(0),firstVectorAlgorithmIndex(0),
	 numCuttingPlanes(0),cuttingPlanes(0),
	 algorithm(0),
	 mainMenu(0),
	 showElementListToggle(0),
	 elementListDialogPopup(0),elementListDialog(0),
	 inLoadPalette(false)
	{
	/* Parse the command line: */
	std::string moduleClassName="";
	std::vector<std::string> dataSetArgs;
	const char* argColorMapName=0;
	const char* viewFileName=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"PALETTE")==0)
				{
				++i;
				argColorMapName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"VIEW")==0)
				{
				++i;
				viewFileName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"CLASS")==0)
				{
				/* Get visualization module class name and data set arguments from command line: */
				++i;
				moduleClassName=argv[i];
				++i;
				while(i<argc)
					{
					dataSetArgs.push_back(argv[i]);
					++i;
					}
				}
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
		dataSet=module->load(dataSetArgs,0); // pipe); // Don't actually use pipe for now
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
	
	/* Create the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Create the element list dialog: */
	elementListDialogPopup=createElementListDialog();
	
	/* Initialize navigation transformation: */
	if(viewFileName!=0)
		{
		/* Open viewpoint file: */
		Misc::File viewpointFile(viewFileName,"rb",Misc::File::LittleEndian);
		
		/* Read the navigation transformation: */
		Vrui::NavTransform::Vector translation;
		viewpointFile.read(translation.getComponents(),3);
		Vrui::NavTransform::Rotation::Scalar quaternion[4];
		viewpointFile.read(quaternion,4);
		Vrui::NavTransform::Scalar scaling=viewpointFile.read<Vrui::NavTransform::Scalar>();
		
		/* Set the navigation transformation: */
		Vrui::setNavigationTransformation(Vrui::NavTransform(translation,Vrui::NavTransform::Rotation::fromQuaternion(quaternion),scaling));
		}
	else
		centerDisplayCallback(0);
	}

Visualizer::~Visualizer(void)
	{
	delete mainMenu;
	delete elementListDialogPopup;
	
	/* Delete all finished visualization elements: */
	for(ElementList::iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		{
		if(veIt->settingsDialogVisible)
			Vrui::popdownPrimaryWidget(veIt->settingsDialog);
		delete veIt->settingsDialog;
		}
	
	/* Delete all locators: */
	for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		delete *blIt;
	
	/* Delete the cutting planes: */
	delete[] cuttingPlanes;
	
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
			Visualization::Abstract::Algorithm* extractor=module->getScalarAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new DataLocator(locatorTool,this,module->getScalarAlgorithmName(algorithmIndex),extractor);
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
			Visualization::Abstract::Algorithm* extractor=module->getVectorAlgorithm(algorithmIndex,variableManager,Vrui::openPipe());
			newLocator=new DataLocator(locatorTool,this,module->getScalarAlgorithmName(algorithmIndex),extractor);
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

void Visualizer::display(GLContextData& contextData) const
	{
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
	for(ElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		if(veIt->show&&!veIt->element->usesTransparency())
			veIt->element->glRenderAction(contextData);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderAction(contextData);

	/* Render all transparent visualization elements: */
	for(ElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		if(veIt->show&&veIt->element->usesTransparency())
			veIt->element->glRenderAction(contextData);
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
		{
		if(elements.size()>0)
			{
			/* Pop up the element list at the same position as the main menu: */
			Vrui::getWidgetManager()->popupPrimaryWidget(elementListDialogPopup,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
			}
		else
			cbData->toggle->setToggle(false);
		}
	else
		Vrui::popdownPrimaryWidget(elementListDialogPopup);
	}

void Visualizer::showElementSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int rowIndex=elementListDialog->getChildIndex(cbData->toggle)/3;
	if(rowIndex>=0&&rowIndex<int(elements.size()))
		{
		if(cbData->set)
			{
			if(elements[rowIndex].settingsDialog!=0&&!elements[rowIndex].settingsDialogVisible)
				{
				Vrui::getWidgetManager()->popupPrimaryWidget(elements[rowIndex].settingsDialog,Vrui::getWidgetManager()->calcWidgetTransformation(cbData->toggle));
				elements[rowIndex].settingsDialogVisible=true;
				}
			else
				cbData->toggle->setToggle(false);
			}
		else
			{
			if(elements[rowIndex].settingsDialog!=0&&elements[rowIndex].settingsDialogVisible)
				{
				Vrui::popdownPrimaryWidget(elements[rowIndex].settingsDialog);
				elements[rowIndex].settingsDialogVisible=false;
				}
			}
		}
	else
		cbData->toggle->setToggle(false);
	}

void Visualizer::showElementCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int rowIndex=elementListDialog->getChildIndex(cbData->toggle)/3;
	if(rowIndex>=0&&rowIndex<int(elements.size()))
		elements[rowIndex].show=cbData->set;
	else
		cbData->toggle->setToggle(false);
	}

void Visualizer::clearElementsCallback(Misc::CallbackData*)
	{
	/* Delete all finished visualization elements: */
	for(ElementList::iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		{
		if(veIt->settingsDialogVisible)
			Vrui::popdownPrimaryWidget(veIt->settingsDialog);
		delete veIt->settingsDialog;
		}
	elements.clear();
	
	/* Reset the element list dialog: */
	Vrui::popdownPrimaryWidget(elementListDialogPopup);
	showElementListToggle->setToggle(false);
	delete elementListDialogPopup;
	elementListDialogPopup=createElementListDialog();
	}

void Visualizer::centerDisplayCallback(Misc::CallbackData*)
	{
	/* Get the data set's domain box: */
	DataSet::Box domain=dataSet->getDomainBox();
	Vrui::Point center=Geometry::mid(domain.getMin(),domain.getMax());
	Vrui::Scalar radius=Geometry::dist(domain.getMin(),domain.getMax());
	
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
