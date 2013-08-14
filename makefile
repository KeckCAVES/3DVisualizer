########################################################################
# Makefile for 3D Visualizer, a generic visualization program for 3D
# multivariate gridded data.
# Copyright (c) 1999-2013 Oliver Kreylos
#
# This file is part of the WhyTools Build Environment.
# 
# The WhyTools Build Environment is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# The WhyTools Build Environment is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the WhyTools Build Environment; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
########################################################################

# Directory containing the Vrui build system. The directory below
# matches the default Vrui installation; if Vrui's installation
# directory was changed during Vrui's installation, the directory below
# must be adapted.
VRUI_MAKEDIR := $(HOME)/Vrui-3.0/share/make
ifdef DEBUG
  VRUI_MAKEDIR := $(VRUI_MAKEDIR)/debug
endif

# Base installation directory for 3D Visualizer and its module
# plug-ins. The module plug-ins cannot be moved from this location
# after installation, or 3D Visualizer will not find them. If this is
# set to the default of $(PWD), 3D Visualizer does not have to be
# installed to be run. 3D Visualizer's executable, plug-ins, and
# resources will be installed in the bin, lib (or lib64), and share
# directories underneath the given base directory, respectively.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR := $(shell pwd)

# Flag whether to use GLSL shaders instead of fixed OpenGL functionality
# for some visualization algorithms, especially volume rendering. This
# flag should only be set to 1 on newer, dedicated 3D graphics cards
# such as Nvidia's G80 series.
USE_SHADERS = 1

# Flag whether to build the 3D Visualizer collaboration module for
# spatially distributed shared data exploration. If the Vrui
# Collaboration Infrastructure is not installed on the host system, this
# flag will be ignored.
USE_COLLABORATION = 1

# List of default visualization modules:
MODULE_NAMES = SphericalASCIIFile \
               StructuredGridASCII \
               StructuredGridVTK \
               CitcomCUCartesianRawFile \
               CitcomCUSphericalRawFile \
               CitcomSRegionalASCIIFile \
               CitcomSGlobalASCIIFile \
               CitcomtFile \
               MultiCitcomtFile \
               CitcomtVectorFile \
               StructuredHexahedralTecplotASCIIFile \
               UnstructuredHexahedralTecplotASCIIFile \
               ImageStack \
               DicomImageStack \
               MultiChannelImageStack

# List of other available modules:
# Add any of these to the MODULE_NAMES list to build them
UNSUPPORTED_MODULE_NAMES = AnalyzeFile \
                           AvsUcdAsciiFile \
                           ByteVolFile \
                           SCTFile \
                           GaleFEMVectorFile \
                           GocadVoxetFile \
                           ConvectionFile \
                           ConvectionFileCartesian \
                           CSConvectionFile \
                           EarthTomographyGrid \
                           ReifSeismicTomography \
                           SeismicTomographyModel \
                           FloatGridFile \
                           FloatVolFile \
                           MultiVolFile \
                           VecVolFile \
                           Kollmann0p9File \
                           MagaliSubductionFile \
                           MargareteSubductionFile \
                           VanKekenFile \
                           UnstructuredPlot3DFile

########################################################################
# Everything below here should not have to be changed
########################################################################

# Define the root of the package source tree
PACKAGEROOT := $(shell pwd)

# Version number for installation subdirectories. This is used to keep
# subsequent release versions of 3D Visualizer from clobbering each
# other. The value should be identical to the major.minor version
# number found in VERSION in the root package directory.
VERSION = 1.14

# Set up resource directories: */
PLUGINSDIREXT = 3DVisualizer-$(VERSION)
RESOURCEDIR = share/3DVisualizer-$(VERSION)

# Include definitions for the system environment and system-provided
# packages
include $(VRUI_MAKEDIR)/SystemDefinitions
include $(VRUI_MAKEDIR)/Packages.System
include $(VRUI_MAKEDIR)/Configuration.Vrui
include $(VRUI_MAKEDIR)/Packages.Vrui

# Check if support for collaboration was requested
ifneq ($(USE_COLLABORATION),0)
  include $(VRUI_MAKEDIR)/Configuration.Collaboration
  include $(VRUI_MAKEDIR)/Packages.Collaboration
  
  # Check if the collaboration infrastructure is installed
  ifndef COLLABORATION_VERSION
    USE_COLLABORATION = 0
  endif
endif

# Add base directory to include path:
EXTRACINCLUDEFLAGS += -I.

# Set destination directory for plugins:
LIBDESTDIR := $(PACKAGEROOT)/$(MYLIBEXT)
PLUGINDESTDIR := $(LIBDESTDIR)/$(PLUGINSDIREXT)
ifneq ($(USE_COLLABORATION),0)
  COLLABORATIONPLUGINDESTDIR := $(LIBDESTDIR)/CollaborationPlugins
  
  # Collaboration plug-ins are installed into Vrui's plug-in
  # installation directory, not 3D Visualizer's:
  COLLABORATIONPLUGININSTALLDIR := $(PLUGININSTALLDIR)/$(COLLABORATIONPLUGINSDIREXT)
endif

# Set up installation directory structure:
EXECUTABLEINSTALLDIR = $(INSTALLDIR)/$(EXEDIR)
PLUGININSTALLDIR = $(INSTALLDIR)/$(MYLIBEXT)/$(PLUGINSDIREXT)
SHAREINSTALLDIR = $(INSTALLDIR)/$(RESOURCEDIR)

########################################################################
# Specify additional compiler and linker flags
########################################################################

CFLAGS += -Wall -pedantic

MODULENAME = $(PLUGINDESTDIR)/lib$(1).$(PLUGINFILEEXT)
COLLABORATIONPLUGINNAME = $(COLLABORATIONPLUGINDESTDIR)/lib$(1).$(PLUGINFILEEXT)

########################################################################
# List packages used by this project
# (Supported packages can be found in
# $(VRUI_MAKEDIR)/BuildRoot/Packages)
########################################################################

PACKAGES = MYGEOMETRY MYMATH MYCLUSTER MYIO MYTHREADS MYMISC

########################################################################
# Specify all final targets
########################################################################

EXECUTABLES = 
MODULES = 
COLLABORATIONPLUGINS = 

EXECUTABLES += $(EXEDIR)/3DVisualizer

MODULES += $(MODULE_NAMES:%=$(call MODULENAME,%))

ifneq ($(USE_COLLABORATION),0)
  EXECUTABLES += $(EXEDIR)/SharedVisualizationServer
  COLLABORATIONPLUGIN_NAMES = SharedVisualizationServer
  COLLABORATIONPLUGINS = $(COLLABORATIONPLUGIN_NAMES:%=$(call COLLABORATIONPLUGINNAME,%))
endif

ALL = $(EXECUTABLES) $(MODULES) $(COLLABORATIONPLUGINS)

.PHONY: all
all: config $(ALL)

########################################################################
# Pseudo-target to print configuration options
########################################################################

.PHONY: config
config: Configure-End

.PHONY: Configure-Begin
Configure-Begin:
	@echo "---- Configured 3D Visualizer options: ----"
	@echo "Installation directory: $(INSTALLDIR)"
ifneq ($(USE_SHADERS),0)
	@echo "Use of GLSL shaders enabled"
else
	@echo "Use of GLSL shaders disabled"
endif
ifneq ($(USE_COLLABORATION),0)
	@echo "Collaborative visualization enabled"
	@echo "  Run 'make plugins-install' to install collaborative visualization server plug-in"
else
	@echo "Collaborative visualization disabled"
  ifdef COLLABORATION_VERSION
	@echo "  (Vrui Collaboration Infrastructure installed)"
  else
	@echo "  (Vrui Collaboration Infrastructure not installed)"
  endif
endif
	@echo "Enabled modules: $(MODULE_NAMES)"

.PHONY: Configure-End
Configure-End: Configure-Begin
Configure-End:
	@echo "--------"

$(wildcard *.cpp Abstract/*.cpp Templatized/*.cpp Wrappers/*.cpp Concrete/*.cpp): config

########################################################################
# Specify other actions to be performed on a `make clean'
########################################################################

.PHONY: extraclean
extraclean:
	-rm -f $(MODULE_NAMES:%=$(call MODULENAME,%))
ifneq ($(USE_COLLABORATION),0)
	-rm -f $(COLLABORATIONPLUGIN_NAMES:%=$(call COLLABORATIONPLUGINNAME,%))
endif

.PHONY: extrasqueakyclean
extrasqueakyclean:
	-rm -f $(ALL)
	-rm -rf $(PACKAGEROOT)/$(LIBEXT)

# Include basic makefile
include $(VRUI_MAKEDIR)/BasicMakefile

########################################################################
# Specify build rules for executables
########################################################################

ABSTRACT_SOURCES = $(wildcard Abstract/*.cpp)

TEMPLATIZED_SOURCES = $(wildcard Templatized/*.cpp)

WRAPPERS_SOURCES = $(wildcard Wrappers/*.cpp)

CONCRETE_SOURCES = Concrete/SphericalCoordinateTransformer.cpp \
                   Concrete/EarthRenderer.cpp \
                   Concrete/PointSet.cpp

VISUALIZER_SOURCES = $(ABSTRACT_SOURCES) \
                     $(TEMPLATIZED_SOURCES) \
                     $(WRAPPERS_SOURCES) \
                     $(CONCRETE_SOURCES) \
                     GLRenderState.cpp \
                     BaseLocator.cpp \
                     CuttingPlaneLocator.cpp \
                     EvaluationLocator.cpp \
                     ScalarEvaluationLocator.cpp \
                     VectorEvaluationLocator.cpp \
                     Extractor.cpp \
                     ExtractorLocator.cpp \
                     ElementList.cpp \
                     ColorBar.cpp \
                     ColorMap.cpp \
                     PaletteEditor.cpp \
                     Visualizer.cpp
ifneq ($(USE_SHADERS),0)
  VISUALIZER_SOURCES += TwoSidedSurfaceShader.cpp \
                        TwoSided1DTexturedSurfaceShader.cpp \
                        Polyhedron.cpp \
                        Raycaster.cpp \
                        SingleChannelRaycaster.cpp \
                        TripleChannelRaycaster.cpp
else
  VISUALIZER_SOURCES += VolumeRenderer.cpp \
                        PaletteRenderer.cpp
endif
ifneq ($(USE_COLLABORATION),0)
  VISUALIZER_SOURCES += SharedVisualizationProtocol.cpp \
                        SharedVisualizationClient.cpp
endif

# List of required shaders:
SHADERS = SingleChannelRaycaster.vs \
          SingleChannelRaycaster.fs \
          TripleChannelRaycaster.vs \
          TripleChannelRaycaster.fs

# Per-source compiler flags:
$(OBJDIR)/Concrete/EarthRenderer.o: CFLAGS += -DEARTHRENDERER_IMAGEDIR='"$(SHAREINSTALLDIR)"'
$(OBJDIR)/SingleChannelRaycaster.o: CFLAGS += -DVISUALIZER_SHADERDIR='"$(SHAREINSTALLDIR)/Shaders"'
$(OBJDIR)/TripleChannelRaycaster.o: CFLAGS += -DVISUALIZER_SHADERDIR='"$(SHAREINSTALLDIR)/Shaders"'
$(OBJDIR)/Visualizer.o: CFLAGS += -DVISUALIZER_MODULENAMETEMPLATE='"$(PLUGININSTALLDIR)/lib%s.$(PLUGINFILEEXT)"'

#
# Rule to build 3D Visualizer main program
#

$(EXEDIR)/3DVisualizer: PACKAGES += MYVRUI MYREALTIME
$(EXEDIR)/3DVisualizer: LINKFLAGS += $(PLUGINHOSTLINKFLAGS)
ifneq ($(USE_COLLABORATION),0)
  $(EXEDIR)/3DVisualizer: PACKAGES += MYCOLLABORATIONCLIENT
  $(EXEDIR)/3DVisualizer: CFLAGS += -DVISUALIZER_USE_COLLABORATION
endif
$(EXEDIR)/3DVisualizer: $(VISUALIZER_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: 3DVisualizer
3DVisualizer: $(EXEDIR)/3DVisualizer

#
# Rule to build shared Visualizer server
#

SHAREDVISUALIZATIONSERVER_SOURCES = SharedVisualizationProtocol.cpp \
                                    SharedVisualizationServer.cpp \
                                    SharedVisualizationServerMain.cpp

$(EXEDIR)/SharedVisualizationServer: PACKAGES += MYCOLLABORATIONSERVER
$(EXEDIR)/SharedVisualizationServer: $(SHAREDVISUALIZATIONSERVER_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: SharedVisualizationServer
SharedVisualizationServer:$(EXEDIR)/SharedVisualizationServer

########################################################################
# Specify build rules for plug-ins
########################################################################

# Pattern rule to link visualization module plug-ins:
$(call MODULENAME,%): PACKAGES += MYGLGEOMETRY MYGLSUPPORT MYGLWRAPPERS GL
ifneq ($(USE_SHADERS),0)
  $(call MODULENAME,%): CFLAGS += -DVISUALIZATION_USE_SHADERS
endif
$(call MODULENAME,%): $(OBJDIR)/pic/Concrete/%.o
	@mkdir -p $(PLUGINDESTDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $(filter %.o,$^) $(LINKDIRFLAGS) $(LINKLIBFLAGS)
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $(filter %.o,$^) $(LINKDIRFLAGS) $(LINKLIBFLAGS)
endif

# Dependencies and special flags for visualization modules:
$(call MODULENAME,CitcomSRegionalASCIIFile): $(OBJDIR)/pic/Concrete/CitcomSRegionalASCIIFile.o \
                                             $(OBJDIR)/pic/Concrete/CitcomSCfgFileParser.o

$(call MODULENAME,CitcomSGlobalASCIIFile): $(OBJDIR)/pic/Concrete/CitcomSGlobalASCIIFile.o \
                                           $(OBJDIR)/pic/Concrete/CitcomSCfgFileParser.o

$(call MODULENAME,StructuredHexahedralTecplotASCIIFile): $(OBJDIR)/pic/Concrete/TecplotASCIIFileHeaderParser.o \
                                                         $(OBJDIR)/pic/Concrete/StructuredHexahedralTecplotASCIIFile.o

$(call MODULENAME,UnstructuredHexahedralTecplotASCIIFile): $(OBJDIR)/pic/Concrete/TecplotASCIIFileHeaderParser.o \
                                                           $(OBJDIR)/pic/Concrete/UnstructuredHexahedralTecplotASCIIFile.o

$(call MODULENAME,MultiChannelImageStack): PACKAGES += MYIMAGES

$(call MODULENAME,DicomImageStack): $(OBJDIR)/pic/Concrete/HuffmanTable.o \
                                    $(OBJDIR)/pic/Concrete/JPEGDecompressor.o \
                                    $(OBJDIR)/pic/Concrete/DicomFile.o \
                                    $(OBJDIR)/pic/Concrete/DicomImageStack.o

# Keep module object files around after building:
.SECONDARY: $(MODULE_NAMES:%=$(OBJDIR)/pic/Concrete/%.o)

# Pattern rule to link collaboration protocol plug-ins:
$(call COLLABORATIONPLUGINNAME,%): PACKAGES += MYCOLLABORATIONSERVER
$(call COLLABORATIONPLUGINNAME,%): $(OBJDIR)/pic/%.o
	@mkdir -p $(COLLABORATIONPLUGINDESTDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $(filter %.o,$^) $(LINKDIRFLAGS) $(LINKLIBFLAGS)
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $(filter %.o,$^) $(LINKDIRFLAGS) $(LINKLIBFLAGS)
endif

$(call COLLABORATIONPLUGINNAME,SharedVisualizationServer): $(OBJDIR)/pic/SharedVisualizationProtocol.o \
                                                           $(OBJDIR)/pic/SharedVisualizationServer.o

# Keep collaboration plugin object files around after building:
.SECONDARY: $(COLLABORATIONPLUGIN_NAMES:%=$(OBJDIR)/pic/%.o)

#
# Rule to install 3D Visualizer in a destination directory
#

install: $(ALL)
	@echo Installing 3D Visualizer in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(EXECUTABLEINSTALLDIR)
	@install $(EXECUTABLES) $(EXECUTABLEINSTALLDIR)
	@install -d $(PLUGININSTALLDIR)
	@install $(MODULES) $(PLUGININSTALLDIR)
	@install -d $(SHAREINSTALLDIR)
	@install $(RESOURCEDIR)/EarthTopography.png $(RESOURCEDIR)/EarthTopography.ppm $(SHAREINSTALLDIR)
ifneq ($(USE_SHADERS),0)
	@install -d $(SHAREINSTALLDIR)/Shaders
	@install $(SHADERS:%=$(RESOURCEDIR)/Shaders/%) $(SHAREINSTALLDIR)/Shaders
endif

#
# Rule to install 3D Visualizer's collaboration server plug-in
#

plugins-install: $(COLLABORATIONPLUGINS)
	@echo Installing 3D Visualizer collaboration server plugin in $(COLLABORATIONPLUGININSTALLDIR)
	@install $(COLLABORATIONPLUGINS) $(COLLABORATIONPLUGININSTALLDIR)

