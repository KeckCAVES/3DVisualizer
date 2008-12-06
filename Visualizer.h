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

#ifndef VISUALIZER_INCLUDED
#define VISUALIZER_INCLUDED

#include <vector>
#include <Misc/Autopointer.h>
#include <Plugins/FactoryManager.h>
#include <Threads/Mutex.h>
#include <Threads/Cond.h>
#include <Threads/Thread.h>
#include <Geometry/Plane.h>
#include <GL/GLColor.h>
#include <GL/GLColorMap.h>
#include <GL/GLContextData.h>
#include <GLMotif/Menu.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>
#include <GLMotif/FileSelectionDialog.h>
#include <Vrui/Geometry.h>
#include <Vrui/Tools/LocatorTool.h>
#include <Vrui/LocatorToolAdapter.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>

#include <Abstract/DataSet.h>

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
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class VectorExtractor;
class CoordinateTransformer;
class VariableManager;
class DataSetRenderer;
class Element;
class Algorithm;
class Module;
}
}

class Visualizer:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef Visualization::Abstract::Module Module;
	typedef Visualization::Abstract::ScalarExtractor ScalarExtractor;
	typedef ScalarExtractor::Scalar Scalar;
	typedef Visualization::Abstract::VectorExtractor VectorExtractor;
	typedef VectorExtractor::Vector Vector;
	typedef Visualization::Abstract::DataSet DataSet;
	typedef DataSet::Locator Locator;
	typedef Visualization::Abstract::DataSetRenderer DataSetRenderer;
	typedef Visualization::Abstract::CoordinateTransformer CoordinateTransformer;
	typedef Visualization::Abstract::Element Element;
	typedef Misc::Autopointer<Element> ElementPointer;
	typedef Visualization::Abstract::Algorithm Algorithm;
	typedef Visualization::Abstract::VariableManager VariableManager;
	typedef Plugins::FactoryManager<Module> ModuleManager;
	
	struct ListElement // Structure storing information relating to a visualization element
		{
		/* Elements: */
		public:
		ElementPointer element; // Pointer to the element itself
		std::string name; // Descriptive name of the element
		GLMotif::Widget* settingsDialog; // Pointer to the element's settings dialog (or NULL)
		bool settingsDialogVisible; // Flag if the element's settings dialog is currently popped up
		bool show; // Flag if the element is being rendered
		};
	
	typedef std::vector<ListElement> ElementList;
	
	struct CuttingPlane // Structure describing a cutting plane
		{
		/* Embedded classes: */
		public:
		typedef Vrui::Plane Plane; // Data type for planes
		
		/* Elements: */
		bool allocated; // Flag if this cutting plane is currently allocated by a cutting plane locator
		Plane plane; // Current plane equation of cutting plane
		bool active; // Flag if this cutting plane is currently enabled
		};
	
	class BaseLocator:public Vrui::LocatorToolAdapter // Base class for locators
		{
		/* Elements: */
		protected:
		Visualizer* application;
		
		/* Constructors and destructors: */
		public:
		BaseLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication);
		
		/* Methods: */
		virtual void highlightLocator(GLContextData& contextData) const; // Renders the locator itself
		virtual void glRenderAction(GLContextData& contextData) const; // Renders opaque elements and other objects controlled by the locator
		virtual void glRenderActionTransparent(GLContextData& contextData) const; // Renders transparent elements and other objects controlled by the locator
		};
	
	class CuttingPlaneLocator:public BaseLocator // Class for locators to render cutting planes
		{
		/* Elements: */
		private:
		CuttingPlane* cuttingPlane; // Pointer to the cutting plane allocated for this tool
		
		/* Constructors and destructors: */
		public:
		CuttingPlaneLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication);
		virtual ~CuttingPlaneLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		};
	
	class EvaluationLocator:public BaseLocator // Base class for locators evaluating scalar or vector properties of a data set
		{
		/* Elements: */
		protected:
		GLMotif::PopupWindow* evaluationDialogPopup; // Pointer to the evaluation dialog window
		GLMotif::RowColumn* evaluationDialog; // Pointer to the evaluation dialog
		GLMotif::TextField* pos[3]; // The coordinate labels for the evaluation position
		Locator* locator; // A locator for evaluation
		Vrui::Point point; // The evaluation point
		bool dragging; // Flag if the locator is currently dragging the evaluation point
		bool hasPoint; // Flag whether the locator has a position
		
		/* Constructors and destructors: */
		public:
		EvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const char* dialogWindowTitle);
		virtual ~EvaluationLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		virtual void highlightLocator(GLContextData& contextData) const;
		};
	
	class ScalarEvaluationLocator:public EvaluationLocator // Class to evaluate scalar properties of a data set
		{
		/* Elements: */
		private:
		const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value
		GLMotif::TextField* value; // The value text field
		bool valueValid; // Flag if the evaluation value is valid
		Scalar currentValue; // The current evaluation value
		
		/* Constructors and destructors: */
		public:
		ScalarEvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication);
		virtual ~ScalarEvaluationLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		void insertControlPointCallback(Misc::CallbackData* cbData);
		};
	
	class VectorEvaluationLocator:public EvaluationLocator // Class to evaluate vector properties of a data set
		{
		/* Elements: */
		private:
		const ScalarExtractor* scalarExtractor; // Extractor for the evaluated scalar value (to color arrow rendering)
		const GLColorMap* colorMap; // Color map for the evaluated scalar value
		const VectorExtractor* vectorExtractor; // Extractor for the evaluated vector value
		GLMotif::TextField* values[3]; // The vector component value text field
		bool valueValid; // Flag if the evaluation value is valid
		Scalar currentScalarValue; // The current scalar value
		Vector currentValue; // The current evaluation value
		GLMotif::TextField* arrowScaleValue; // Text field showing current arrow scale
		GLMotif::Slider* arrowScaleSlider; // Slider to adjust the arrow scale
		Scalar arrowLengthScale; // Scaling factor for arrow rendering
		
		/* Constructors and destructors: */
		public:
		VectorEvaluationLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication);
		virtual ~VectorEvaluationLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void highlightLocator(GLContextData& contextData) const;
		void arrowScaleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
		};
	
	class DataLocator:public BaseLocator // Class for locators applying visualization algorithms to data sets
		{
		/* Elements: */
		private:
		
		/* Extractor thread state: */
		#ifdef __DARWIN__
		volatile bool terminate; // Flag to tell the extractor thread to shut itself down
		#endif
		Algorithm* extractor; // Visualization element extractor
		GLMotif::Widget* settingsDialog; // The element extractor's settings dialog
		Threads::Thread extractorThread; // The visualization element extractor thread
		GLMotif::PopupWindow* busyDialog; // Dialog window to show while a non-incremental extractor is busy
		
		/* Locator state: */
		Locator* locator; // A locator for the visualization algorithm
		bool dragging; // Flag if the tool's button is currently pressed
		bool firstExtraction; // Flag if dragging mode has just started
		
		/* Extractor thread communication input: */
		Threads::Mutex seedRequestMutex; // Mutex protecting the seed request state
		Threads::Cond seedRequestCond; // Condition variable for the extractor thread to block on
		volatile bool seedTracking; // True while the locator is tracking a visualization element
		Locator* volatile seedLocator; // Locator at which to seed a visualization element; ==0 if there is no current seed request
		volatile bool extracting; // Flag if the extractor thread is currently extracting a visualization element
		
		/* Extractor thread communication output: */
		ElementPointer trackedElements[3]; // Triple-buffer of currently tracked visualization elements
		volatile int renderIndex; // Index of rendered element
		volatile int mostRecentIndex; // Index of most recently extracted element
		
		/* Private methods: */
		void* incrementalExtractorThreadMethod(void); // The extractor thread method for incremental visualization algorithms
		void* immediateExtractorThreadMethod(void); // The extractor thread method for immediate seeded or global visualization algorithms
		void* slaveExtractorThreadMethod(void); // The extractor thread method for slaves in a cluster environment
		GLMotif::PopupWindow* createBusyDialog(const char* algorithmName); // Creates the busy dialog
		
		/* Constructors and destructors: */
		public:
		DataLocator(Vrui::LocatorTool* sTool,Visualizer* sApplication,const char* algorithmName,Algorithm* sExtractor);
		virtual ~DataLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		virtual void highlightLocator(GLContextData& contextData) const;
		virtual void glRenderAction(GLContextData& contextData) const;
		virtual void glRenderActionTransparent(GLContextData& contextData) const;
		};
	
	typedef std::vector<BaseLocator*> BaseLocatorList;
	
	friend class BaseLocator;
	friend class VectorEvaluationLocator;
	friend class DataLocator;
	
	/* Elements: */
	private:
	ModuleManager moduleManager; // Manager to load 3D visualization modules from dynamic libraries
	Module* module; // Visualization module
	DataSet* dataSet; // Data set to visualize
	VariableManager* variableManager; // Manager to organize data sets and scalar and vector variables
	GLColor<GLfloat,4> dataSetRenderColor; // Color to use when rendering the data set
	DataSetRenderer* dataSetRenderer; // Renderer for the data set
	CoordinateTransformer* coordinateTransformer; // Transformer from Cartesian coordinates back to data set coordinates
	int firstScalarAlgorithmIndex; // Index of first module-provided scalar algorithm in algorithm menu
	int firstVectorAlgorithmIndex; // Index of first module-provided vector algorithm in algorithm menu
	size_t numCuttingPlanes; // Maximum number of cutting planes supported
	CuttingPlane* cuttingPlanes; // Array of available cutting planes
	BaseLocatorList baseLocators; // List of active locators
	ElementList elements; // List of previously extracted visualization elements
	int algorithm; // The currently selected algorithm
	GLMotif::PopupMenu* mainMenu; // The main menu widget
	GLMotif::ToggleButton* showElementListToggle; // Toggle button to show the element list dialog
	GLMotif::PopupWindow* elementListDialogPopup; // Dialog listing visualization elements
	GLMotif::RowColumn* elementListDialog; // Widget containing the rows for all visualization elements
	
	/* Lock flags for modal dialogs: */
	bool inLoadPalette; // Flag whether the user is currently selecting a palette to load
	
	/* Private methods: */
	GLMotif::Popup* createRenderingModesMenu(void);
	GLMotif::Popup* createScalarVariablesMenu(void);
	GLMotif::Popup* createVectorVariablesMenu(void);
	GLMotif::Popup* createAlgorithmsMenu(void);
	GLMotif::Popup* createStandardLuminancePalettesMenu(void);
	GLMotif::Popup* createStandardSaturationPalettesMenu(void);
	GLMotif::Popup* createColorMenu(void);
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createElementListDialog(void);
	void addElement(Element* newElement); // Adds a new visualization element to the list
	
	/* Constructors and destructors: */
	public:
	Visualizer(int& argc,char**& argv,char**& appDefaults);
	virtual ~Visualizer(void);
	
	/* Methods: */
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void display(GLContextData& contextData) const;
	void changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void loadPaletteCallback(Misc::CallbackData* cbData);
	void loadPaletteOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadPaletteCancelCallback(GLMotif::FileSelectionDialog::CancelCallbackData* cbData);
	void showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	void createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData);
	void showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showElementSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showElementCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void clearElementsCallback(Misc::CallbackData* cbData);
	void centerDisplayCallback(Misc::CallbackData* cbData);
	};

#endif
