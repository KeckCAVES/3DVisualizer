/***********************************************************************
VariableManager - Helper class to manage the scalar and vector variables
that can be extracted from a data set.
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

#ifndef VISUALIZATION_ABSTRACT_VARIABLEMANAGER_INCLUDED
#define VISUALIZATION_ABSTRACT_VARIABLEMANAGER_INCLUDED

#include <Abstract/DataSet.h>
#include <PaletteEditor.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}
class GLColorMap;
namespace GLMotif {
class PopupWindow;
class ColorBar;
}
namespace Visualization {
namespace Abstract {
class ScalarExtractor;
class VectorExtractor;
}
}

namespace Visualization {

namespace Abstract {

class VariableManager
	{
	/* Embedded classes: */
	public:
	enum PaletteType // Enumerated type for palette generation types
		{
		LUMINANCE_GREY,LUMINANCE_RED,LUMINANCE_YELLOW,LUMINANCE_GREEN,LUMINANCE_CYAN,LUMINANCE_BLUE,LUMINANCE_MAGENTA,
		SATURATION_RED_CYAN,SATURATION_YELLOW_BLUE,SATURATION_GREEN_MAGENTA,SATURATION_CYAN_RED,SATURATION_BLUE_YELLOW,SATURATION_MAGENTA_GREEN,
		RAINBOW
		};
	
	private:
	struct ScalarVariable // Structure containing state of a scalar variable
		{
		/* Elements: */
		public:
		ScalarExtractor* scalarExtractor; // Scalar extractor for the scalar variable
		DataSet::VScalarRange valueRange; // Value range of the scalar variable
		GLColorMap* colorMap; // The color map to render the scalar variable
		PaletteEditor::Storage* palette; // Pointer to palette editor state for the scalar variable
		
		/* Constructors and destructors: */
		ScalarVariable(void);
		~ScalarVariable(void);
		};
	
	/* Elements: */
	const DataSet* dataSet; // Data set containing the scalar and vector variables
	char* defaultColorMapName; // Name of default color map file, or 0 if no default given
	int numScalarVariables; // Total number of scalar variables
	ScalarVariable* scalarVariables; // Array of scalar variables for the data set; initialized on demand
	GLMotif::PopupWindow* colorBarDialogPopup; // Dialog showing a color bar with tick marks and number labels
	GLMotif::ColorBar* colorBar; // Widget to display color maps
	PaletteEditor* paletteEditor; // Editor for color maps
	int numVectorVariables; // Total number of vector variables
	VectorExtractor** vectorExtractors; // Array of extractors for the data set's vector variables
	int currentScalarVariableIndex; // The index of the currently selected scalar variable
	int currentVectorVariableIndex; // The index of the currently selected vector variable
	
	/* Private methods: */
	void prepareScalarVariable(int scalarVariableIndex);
	void colorMapChangedCallback(Misc::CallbackData* cbData);
	void savePaletteCallback(Misc::CallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	VariableManager(const DataSet* sDataSet,const char* sDefaultColorMapName); // Creates variable manager for the given data set
	~VariableManager(void);
	
	/* Methods: */
	int getNumScalarVariables(void) const // Returns the number of scalar variables in the data set
		{
		return numScalarVariables;
		}
	int getNumVectorVariables(void) const // Returns the number of vector variables in the data set
		{
		return numVectorVariables;
		}
	const DataSet* getDataSetByScalarVariable(int scalarVariableIndex) const; // Returns the data set owning the given scalar variable
	const DataSet* getDataSetByVectorVariable(int vectorVariableIndex) const; // Returns the data set owning the given vector variable
	const char* getScalarVariableName(int scalarVariableIndex) const // Returns the name of the given scalar variable
		{
		return dataSet->getScalarVariableName(scalarVariableIndex);
		}
	const char* getVectorVariableName(int vectorVariableIndex) const // Returns the name of the given vector variable
		{
		return dataSet->getVectorVariableName(vectorVariableIndex);
		}
	int getCurrentScalarVariable(void) const // Returns the index of the currently selected scalar variable
		{
		return currentScalarVariableIndex;
		}
	int getCurrentVectorVariable(void) const // Returns the index of the currently selected vector variable
		{
		return currentVectorVariableIndex;
		}
	void setCurrentScalarVariable(int newCurrentScalarVariable); // Sets the currently selected scalar variable
	void setCurrentVectorVariable(int newCurrentVectorVariable); // Sets the currently selected vector variable
	const ScalarExtractor* getScalarExtractor(int scalarVariableIndex); // Returns a new scalar extractor for the given scalar variable
	const DataSet::VScalarRange& getScalarValueRange(int scalarVariableIndex); // Returns the value range of the given scalar variable
	const GLColorMap* getColorMap(int scalarVariableIndex); // Returns the color map for the given scalar variable
	const VectorExtractor* getVectorExtractor(int vectorVariableIndex); // Returns a new vector extractor for the given vector variable
	const ScalarExtractor* getCurrentScalarExtractor(void) const // Returns the current scalar extractor
		{
		return scalarVariables[currentScalarVariableIndex].scalarExtractor;
		}
	const DataSet::VScalarRange& getCurrentScalarValueRange(void) const // Returns the current scalar value range
		{
		return scalarVariables[currentScalarVariableIndex].valueRange;
		}
	const GLColorMap* getCurrentColorMap(void) const // Returns the current color map
		{
		return scalarVariables[currentScalarVariableIndex].colorMap;
		}
	const VectorExtractor* getCurrentVectorExtractor(void) const // Returns the current vector extractor
		{
		return vectorExtractors[currentVectorVariableIndex];
		}
	void showColorBar(bool show); // Shows or hides the color bar dialog
	void showPaletteEditor(bool show); // Shows or hides the palette editor
	void createPalette(int newPaletteType); // Creates a default palette for the current scalar variable
	void loadPalette(const char* paletteFileName); // Loads a palette for the current scalar variable
	void insertPaletteEditorControlPoint(double newControlPoint); // Inserts a new control point into the palette editor at the given value
	};

}

}

#endif
