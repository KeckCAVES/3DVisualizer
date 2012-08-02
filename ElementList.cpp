/***********************************************************************
ElementList - Class to manage a list of previously extracted
visualization elements.
Copyright (c) 2009-2012 Oliver Kreylos

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

#include "ElementList.h"

#include <Misc/StandardMarshallers.h>
#include <Misc/File.h>
#include <IO/File.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Separator.h>
#include <GLMotif/ScrolledListBox.h>
#include <Vrui/Vrui.h>
#include <Vrui/OpenFile.h>

#include <Abstract/Parameters.h>
#include <Abstract/BinaryParametersSink.h>
#include <Abstract/FileParametersSink.h>
#include <Abstract/Element.h>

/****************************
Methods of class ElementList:
****************************/

void ElementList::updateUiState(void)
	{
	/* Update the toggle buttons: */
	int selectedElementIndex=elementList->getSelectedItem();
	if(selectedElementIndex>=0)
		{
		/* Update the toggle buttons: */
		showElementToggle->setToggle(elements[selectedElementIndex].show);
		showElementSettingsToggle->setToggle(elements[selectedElementIndex].settingsDialogVisible);
		}
	else
		{
		/* Reset the toggle buttons: */
		showElementToggle->setToggle(false);
		showElementSettingsToggle->setToggle(false);
		}
	}

void ElementList::elementListValueChangedCallback(GLMotif::ListBox::ValueChangedCallbackData* cbData)
	{
	/* Update the user interface: */
	updateUiState();
	}

void ElementList::elementListItemSelectedCallback(GLMotif::ListBox::ItemSelectedCallbackData* cbData)
	{
	if(cbData->selectedItem>=0)
		{
		/* Toggle the visibility state of the selected item: */
		elements[cbData->selectedItem].show=!elements[cbData->selectedItem].show;
		
		/* Update the user interface: */
		updateUiState();
		}
	}

void ElementList::showElementToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int selectedElementIndex=elementList->getSelectedItem();
	if(selectedElementIndex>=0)
		{
		/* Show or hide the element: */
		elements[selectedElementIndex].show=cbData->set;
		}
	else
		cbData->toggle->setToggle(false);
	}

void ElementList::showElementSettingsToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int selectedElementIndex=elementList->getSelectedItem();
	if(selectedElementIndex>=0&&elements[selectedElementIndex].settingsDialog!=0)
		{
		/* Show or hide the element's settings dialog: */
		if(cbData->set)
			{
			typedef GLMotif::WidgetManager::Transformation WTransform;
			typedef WTransform::Vector WVector;
			
			/* Open the settings dialog right next to the element list dialog: */
			WTransform transform=widgetManager->calcWidgetTransformation(elementListDialogPopup);
			const GLMotif::Box& box=elementListDialogPopup->getExterior();
			WVector offset(box.origin[0]+box.size[0],box.origin[1]+box.size[1]*0.5f,0.0f);
			
			GLMotif::Widget* dialog=elements[selectedElementIndex].settingsDialog;
			offset[0]-=dialog->getExterior().origin[0];
			offset[1]-=dialog->getExterior().origin[1]+dialog->getExterior().size[1]*0.5f;
			transform*=WTransform::translate(offset);
			widgetManager->popupPrimaryWidget(dialog,transform);
			}
		else
			widgetManager->popdownWidget(elements[selectedElementIndex].settingsDialog);
		}
	else
		cbData->toggle->setToggle(false);
	}

void ElementList::elementSettingsCloseCallback(Misc::CallbackData* cbData)
	{
	GLMotif::PopupWindow::CloseCallbackData* myCbData=dynamic_cast<GLMotif::PopupWindow::CloseCallbackData*>(cbData);
	if(myCbData!=0)
		{
		/* Find the element whose settings dialog was closed: */
		ListElementList::iterator eIt;
		for(eIt=elements.begin();eIt!=elements.end();++eIt)
			if(myCbData->popupWindow==eIt->settingsDialog)
				{
				/* Update the element and the UI: */
				eIt->settingsDialogVisible=false;
				updateUiState();
				break;
				}
		}
	}

void ElementList::deleteElementSelectedCallback(Misc::CallbackData* cbData)
	{
	int selectedElementIndex=elementList->getSelectedItem();
	if(selectedElementIndex>=0)
		{
		/* Delete the visualization element and its settings dialog: */
		delete elements[selectedElementIndex].settingsDialog;
		elements.erase(elements.begin()+selectedElementIndex);
		
		/* Remove the entry from the list box: */
		elementList->removeItem(selectedElementIndex);
		
		/* Update the user interface: */
		updateUiState();
		}
	}

ElementList::ElementList(GLMotif::WidgetManager* sWidgetManager)
	:widgetManager(sWidgetManager),
	 elementListDialogPopup(0),elementList(0)
	{
	/* Create the settings dialog window: */
	elementListDialogPopup=new GLMotif::PopupWindow("ElementListDialogPopup",widgetManager,"Visualization Element List");
	elementListDialogPopup->setResizableFlags(true,true);
	
	GLMotif::RowColumn* elementListDialog=new GLMotif::RowColumn("ElementListDialog",elementListDialogPopup,false);
	elementListDialog->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	elementListDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	elementListDialog->setNumMinorWidgets(1);
	
	/* Create a listbox containing all visualization elements: */
	GLMotif::ScrolledListBox* scrolledElementList=new GLMotif::ScrolledListBox("ScrolledElementList",elementListDialog,GLMotif::ListBox::ALWAYS_ONE,20,10);
	scrolledElementList->showHorizontalScrollBar(false);
	elementList=scrolledElementList->getListBox();
	elementList->getValueChangedCallbacks().add(this,&ElementList::elementListValueChangedCallback);
	elementList->getItemSelectedCallbacks().add(this,&ElementList::elementListItemSelectedCallback);
	
	elementListDialog->setColumnWeight(0,1.0f);
	
	/* Create a list of buttons to control elements: */
	GLMotif::Margin* buttonBoxMargin=new GLMotif::Margin("ButtonBoxMargin",elementListDialog,false);
	buttonBoxMargin->setAlignment(GLMotif::Alignment::VCENTER);
	
	GLMotif::RowColumn* buttonBox=new GLMotif::RowColumn("ButtonBox",buttonBoxMargin,false);
	buttonBox->setOrientation(GLMotif::RowColumn::VERTICAL);
	buttonBox->setNumMinorWidgets(1);
	
	showElementToggle=new GLMotif::ToggleButton("ShowElementToggle",buttonBox,"Show");
	showElementToggle->getValueChangedCallbacks().add(this,&ElementList::showElementToggleValueChangedCallback);
	
	showElementSettingsToggle=new GLMotif::ToggleButton("ShowElementSettingsToggle",buttonBox,"Show Settings");
	showElementSettingsToggle->getValueChangedCallbacks().add(this,&ElementList::showElementSettingsToggleValueChangedCallback);
	
	new GLMotif::Separator("Separator",buttonBox,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::Button* deleteElementButton=new GLMotif::Button("DeleteElementButton",buttonBox,"Delete");
	deleteElementButton->getSelectCallbacks().add(this,&ElementList::deleteElementSelectedCallback);
	
	buttonBox->manageChild();
	
	buttonBoxMargin->manageChild();
	
	elementListDialog->manageChild();
	}

ElementList::~ElementList(void)
	{
	/* Delete all elements: */
	clear();
	
	/* Delete the element list dialog: */
	delete elementListDialogPopup;
	}

void ElementList::clear(void)
	{
	/* Delete all visualization elements: */
	for(ListElementList::iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		delete veIt->settingsDialog;
	elements.clear();
	
	/* Clear the list box: */
	elementList->clear();
	
	/* Update the GUI: */
	updateUiState();
	}

void ElementList::addElement(Element* newElement,const char* elementName)
	{
	/* Create the element's list structure: */
	ListElement le;
	le.element=newElement;
	le.name=elementName;
	le.settingsDialog=newElement->createSettingsDialog(widgetManager);
	le.settingsDialogVisible=false;
	le.show=true;
	
	/* Add the element to the list and select it: */
	elements.push_back(le);
	elementList->selectItem(elementList->addItem(elementName),true);
	
	/* Update the toggle buttons: */
	showElementToggle->setToggle(true);
	showElementSettingsToggle->setToggle(false);
	
	/* Check if the element's settings dialog is a dialog: */
	GLMotif::PopupWindow* sd=dynamic_cast<GLMotif::PopupWindow*>(le.settingsDialog);
	if(sd!=0)
		{
		/* Add a close button to the settings dialog, and register a close callback: */
		sd->setCloseButton(true);
		sd->getCloseCallbacks().add(this,&ElementList::elementSettingsCloseCallback);
		}
	}

void ElementList::saveElements(const char* elementFileName,bool ascii,const Visualization::Abstract::VariableManager* variableManager) const
	{
	if(ascii)
		{
		/* Create a text element file and a sink to write into it: */
		Misc::File elementFile(elementFileName,"wt");
		Visualization::Abstract::FileParametersSink sink(variableManager,elementFile);
		
		/* Save all visible visualization elements: */
		for(ListElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
			if(veIt->show)
				{
				/* Write the element's name: */
				elementFile.puts(veIt->name.c_str());
				elementFile.puts("\n");
				
				/* Write the element's parameters: */
				elementFile.puts("\t{\n");
				veIt->element->getParameters()->write(sink);
				elementFile.puts("\t}\n");
				}
		}
	else
		{
		/* Create a binary element file and a data sink to write into it: */
		IO::FilePtr elementFile(Vrui::openFile(elementFileName,IO::File::WriteOnly));
		elementFile->setEndianness(Misc::LittleEndian);
		Visualization::Abstract::BinaryParametersSink sink(variableManager,*elementFile,false);
		
		/* Save all visible visualization elements: */
		for(ListElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
			if(veIt->show)
				{
				/* Write the element's name: */
				Misc::Marshaller<std::string>::write(veIt->name,*elementFile);
				
				/* Write the element's parameters: */
				veIt->element->getParameters()->write(sink);
				}
		}
	}

void ElementList::renderElements(GLRenderState& renderState,bool transparent) const
	{
	/* Render all visualization elements whose transparency flags match the given flag: */
	for(ListElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		if(veIt->show&&veIt->element->usesTransparency()==transparent)
			veIt->element->glRenderAction(renderState);
	}
