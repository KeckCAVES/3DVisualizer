/***********************************************************************
Visualizer - Test application for the new visualization component
framework.
Copyright (c) 2005-2013 Oliver Kreylos

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

#ifndef VISUALIZER_INCLUDED
#define VISUALIZER_INCLUDED

#include <string>
#include <vector>
#include <Plugins/FactoryManager.h>
#include <GL/GLColor.h>
#include <GLMotif/Menu.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>
#include <GLMotif/FileSelectionDialog.h>
#include <SceneGraph/GroupNode.h>
#include <Vrui/Geometry.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>

/* Forward declarations: */
namespace GLMotif {
class Widget;
class Popup;
class PopupMenu;
class PopupWindow;
class RowColumn;
class TextField;
class ToggleButton;
}
#ifdef VISUALIZER_USE_COLLABORATION
namespace Collaboration {
class CollaborationClient;
}
#endif
namespace Visualization {
namespace Abstract {
class DataSet;
class VariableManager;
class DataSetRenderer;
class Parameters;
class Algorithm;
class Element;
class CoordinateTransformer;
class Module;
}
}
struct CuttingPlane;
#ifdef VISUALIZER_USE_COLLABORATION
class SharedVisualizationClient;
#endif
class BaseLocator;
class ElementList;

class Visualizer:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::DataSet DataSet;
	typedef Visualization::Abstract::VariableManager VariableManager;
	typedef Visualization::Abstract::DataSetRenderer DataSetRenderer;
	typedef Visualization::Abstract::Parameters Parameters;
	typedef Visualization::Abstract::Algorithm Algorithm;
	typedef Visualization::Abstract::Element Element;
	typedef Visualization::Abstract::CoordinateTransformer CoordinateTransformer;
	typedef Visualization::Abstract::Module Module;
	typedef Plugins::FactoryManager<Module> ModuleManager;
	
	struct SG // Structure for additional scene graphs
		{
		/* Elements: */
		public:
		SceneGraph::GroupNodePointer root; // Scene graph root
		std::string name; // Name of the scene graph
		bool render; // Flag if the scene graph is being rendered
		};
	
	typedef std::vector<BaseLocator*> BaseLocatorList;
	
	#ifdef VISUALIZER_USE_COLLABORATION
	friend class SharedVisualizationClient;
	#endif
	friend class BaseLocator;
	friend class CuttingPlaneLocator;
	friend class EvaluationLocator;
	friend class ScalarEvaluationLocator;
	friend class VectorEvaluationLocator;
	friend class ExtractorLocator;
	
	/* Elements: */
	private:
	ModuleManager moduleManager; // Manager to load 3D visualization modules from dynamic libraries
	Module* module; // Visualization module
	DataSet* dataSet; // Data set to visualize
	VariableManager* variableManager; // Manager to organize data sets and scalar and vector variables
	bool renderDataSet; // Flag whether to render the data set
	GLColor<GLfloat,4> dataSetRenderColor; // Color to use when rendering the data set
	DataSetRenderer* dataSetRenderer; // Renderer for the data set
	std::vector<SG> sceneGraphs; // List of additional scene graphs
	bool renderSceneGraphs; // Flag indicating whether any scene graphs are supposed to be rendered
	CoordinateTransformer* coordinateTransformer; // Transformer from Cartesian coordinates back to data set coordinates
	int firstScalarAlgorithmIndex; // Index of first module-provided scalar algorithm in algorithm menu
	int firstVectorAlgorithmIndex; // Index of first module-provided vector algorithm in algorithm menu
	#ifdef VISUALIZER_USE_COLLABORATION
	Collaboration::CollaborationClient* collaborationClient; // Client module to support collaborative visualization in spatially distributed VR environments
	SharedVisualizationClient* sharedVisualizationClient; // Protocol plug-in for shared Visualizer protocol
	#endif
	size_t numCuttingPlanes; // Maximum number of cutting planes supported
	CuttingPlane* cuttingPlanes; // Array of available cutting planes
	BaseLocatorList baseLocators; // List of active locators
	ElementList* elementList; // List of previously extracted visualization elements
	int algorithm; // The currently selected algorithm
	GLMotif::PopupMenu* mainMenu; // The main menu widget
	GLMotif::ToggleButton* showColorBarToggle; // Toggle button to show the color bar
	GLMotif::ToggleButton* showPaletteEditorToggle; // Toggle button to show the palette editor
	GLMotif::ToggleButton* showElementListToggle; // Toggle button to show the element list dialog
	GLMotif::ToggleButton* showClientDialogToggle; // Toggle button to show the collaboration client dialog
	
	/* Lock flags for modal dialogs: */
	bool inLoadPalette; // Flag whether the user is currently selecting a palette to load
	bool inLoadElements; // Flag whether the user is currently selecting an element file to load
	
	/* Private methods: */
	GLMotif::Popup* createRenderingModesMenu(void);
	GLMotif::Popup* createScalarVariablesMenu(void);
	GLMotif::Popup* createVectorVariablesMenu(void);
	GLMotif::Popup* createAlgorithmsMenu(void);
	GLMotif::Popup* createElementsMenu(void);
	GLMotif::Popup* createStandardLuminancePalettesMenu(void);
	GLMotif::Popup* createStandardSaturationPalettesMenu(void);
	GLMotif::Popup* createColorMenu(void);
	GLMotif::PopupMenu* createMainMenu(void);
	void loadElements(const char* elementFileName,bool ascii); // Loads all visualization elements defined in the given file
	
	/* Constructors and destructors: */
	public:
	Visualizer(int& argc,char**& argv,char**& appDefaults);
	virtual ~Visualizer(void);
	
	/* Methods from Vrui::Application: */
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void frame();
	virtual void display(GLContextData& contextData) const;
	virtual void sound(ALContextData& contextData) const;
	
	/* New methods: */
	void changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void toggleSceneGraphCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& sceneGraphIndex);
	void changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void loadPaletteCallback(Misc::CallbackData* cbData);
	void loadPaletteOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadPaletteCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData);
	void showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void colorBarClosedCallback(Misc::CallbackData* cbData);
	void showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void paletteEditorClosedCallback(Misc::CallbackData* cbData);
	void createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	void createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	void showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void elementListClosedCallback(Misc::CallbackData* cbData);
	void loadElementsCallback(Misc::CallbackData* cbData);
	void loadElementsOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadElementsCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData);
	void saveElementsCallback(Misc::CallbackData* cbData);
	void clearElementsCallback(Misc::CallbackData* cbData);
	void showClientDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void clientDialogClosedCallback(Misc::CallbackData* cbData);
	void centerDisplayCallback(Misc::CallbackData* cbData);
	};

#endif
