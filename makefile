########################################################################
# Makefile for 3D Visualizer, a generic visualization program for 3D
# multivariate gridded data.
# Copyright (c) 1999-2011 Oliver Kreylos
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

# Root directory of the Vrui software installation. This must match the
# same setting in Vrui's makefile. By default the directories match; if
# the installation directory was adjusted during Vrui's installation, it
# must be adjusted here as well.
VRUIDIR = $(HOME)/Vrui-2.1

# Base installation directory for 3D Visualizer and its module
# plug-ins. The module plug-ins cannot be moved from this location
# after installation, or 3D Visualizer will not find them. If this is
# set to the default of $(PWD), 3D Visualizer does not have to be
# installed to be run. 3D Visualizer's executable, plug-ins, and
# resources will be installed in the bin, lib (or lib64), and share
# directories underneath the given base directory, respectively.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR = $(shell pwd)

# Flag whether to use GLSL shaders instead of fixed OpenGL functionality
# for some visualization algorithms, especially volume rendering. This
# flag should only be set to 1 on newer, dedicated 3D graphics cards
# such as Nvidia's G80 series.
USE_SHADERS = 0

# Flag whether to build the 3D Visualizer collaboration module for
# spatially distributed shared data exploration.
USE_COLLABORATION = 0

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
                           ByteVolFile \
                           GaleFEMVectorFile \
                           GocadVoxetFile \
                           ConvectionFile \
                           ConvectionFileCartesian \
                           CSConvectionFile \
                           DicomImageStack \
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
# Nothing underneath here needs to be changed.
########################################################################

# Version number for installation subdirectories. This is used to keep
# subsequent release versions of 3D Visualizer from clobbering each
# other. The value should be identical to the major.minor version
# number found in VERSION in the root package directory.
VERSION = 1.8

# Set up destination directories for compilation products:
OBJDIRBASE = o
ifeq ($(shell uname -m),x86_64)
  LIBDIRBASE = lib64
else
  LIBDIRBASE = lib
endif
BINDIRBASE = bin
MODULEDIR = 3DVisualizer-$(VERSION)

# Set up resource directories: */
RESOURCEDIR = share/3DVisualizer-$(VERSION)

# Set up additional flags for the C++ compiler:
CFLAGS = 

# Create debug or fully optimized versions of the software:
VRUIMAKEDIR = $(VRUIDIR)/share
ifdef DEBUG
  # Include the debug version of the Vrui application makefile fragment:
  include $(VRUIMAKEDIR)/Vrui.debug.makeinclude
  # Enable debugging and disable optimization:
  CFLAGS += -g3 -O0
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)/debug
  LIBDIR = $(LIBDIRBASE)/debug
  BINDIR = $(BINDIRBASE)/debug
else
  # Include the release version of the Vrui application makefile fragment:
  include $(VRUIMAKEDIR)/Vrui.makeinclude
  # Disable debugging and enable optimization:
  CFLAGS += -g0 -O3 -DNDEBUG
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)
  LIBDIR = $(LIBDIRBASE)
  BINDIR = $(BINDIRBASE)
endif

# Set up installation directory structure:
BININSTALLDIR = $(INSTALLDIR)/$(BINDIRBASE)
PLUGININSTALLDIR = $(INSTALLDIR)/$(LIBDIR)/$(MODULEDIR)
SHAREINSTALLDIR = $(INSTALLDIR)/$(RESOURCEDIR)

# Add base directory to include path:
CFLAGS += -I.

# Pattern rule to compile C++ sources:
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/$(*D)
	@echo Compiling $<...
	@g++ -c -o $@ $(VRUI_CFLAGS) $(CFLAGS) $<

# Function to generate full plug-in names:
PLUGINNAME = $(LIBDIR)/$(MODULEDIR)/lib$(1).$(VRUI_PLUGINFILEEXT)

# Pattern rule to link visualization module plug-ins:
$(call PLUGINNAME,%): CFLAGS += $(VRUI_PLUGINCFLAGS)
ifneq ($(USE_SHADERS),0)
  $(call PLUGINNAME,%): CFLAGS += -DVISUALIZATION_USE_SHADERS
endif
$(call PLUGINNAME,%): CFLAGS += $(VRUI_PLUGINCFLAGS)
$(call PLUGINNAME,%): $(OBJDIR)/Concrete/%.o
	@mkdir -p $(LIBDIR)/$(MODULEDIR)
	@echo Linking $@...
	@g++ $(VRUI_PLUGINLINKFLAGS) -o $@ $^

# Rule to build all 3D Visualizer components:
MODULES = $(MODULE_NAMES:%=$(call PLUGINNAME,%))
ALL = $(BINDIR)/3DVisualizer \
      $(MODULES)
ifneq ($(USE_COLLABORATION),0)
  ALL += $(BINDIR)/SharedVisualizationServer
endif
.PHONY: all
all: $(ALL)

# Rule to remove build results:
clean:
	-rm -f $(OBJDIR)/*.o $(OBJDIR)/Abstract/*.o $(OBJDIR)/Wrappers/*.o $(OBJDIR)/Concrete/*.o
	-rmdir $(OBJDIR)/Abstract $(OBJDIR)/Wrappers $(OBJDIR)/Concrete
	-rm -f $(ALL)

# Rule to clean the source directory for packaging:
distclean:
	-rm -rf $(OBJDIRBASE)
	-rm -rf $(BINDIRBASE)
	-rm -rf lib lib64

# List of required source files:
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
  VISUALIZER_SOURCES += Polyhedron.cpp \
                        Raycaster.cpp \
                        SingleChannelRaycaster.cpp \
                        TripleChannelRaycaster.cpp
else
  VISUALIZER_SOURCES += VolumeRenderer.cpp \
                        PaletteRenderer.cpp
endif
ifneq ($(USE_COLLABORATION),0)
  VISUALIZER_SOURCES += SharedVisualizationPipe.cpp \
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
$(OBJDIR)/Visualizer.o: CFLAGS += -DVISUALIZER_MODULENAMETEMPLATE='"$(PLUGININSTALLDIR)/lib%s.$(VRUI_PLUGINFILEEXT)"'

#
# Rule to build 3D Visualizer main program
#

ifneq ($(USE_COLLABORATION),0)
  $(BINDIR)/3DVisualizer: CFLAGS += -DVISUALIZER_USE_COLLABORATION
  $(BINDIR)/3DVisualizer: VRUI_LINKFLAGS += -lCollaboration.g++-3
endif
$(BINDIR)/3DVisualizer: $(VISUALIZER_SOURCES:%.cpp=$(OBJDIR)/%.o)
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@g++ -o $@ $^ $(VRUI_LINKFLAGS) $(VRUI_PLUGINHOSTLINKFLAGS)
.PHONY: 3DVisualizer
3DVisualizer: $(BINDIR)/3DVisualizer

# Dependencies and special flags for visualization modules:
$(call PLUGINNAME,CitcomSRegionalASCIIFile): $(OBJDIR)/Concrete/CitcomSRegionalASCIIFile.o \
                                             $(OBJDIR)/Concrete/CitcomSCfgFileParser.o

$(call PLUGINNAME,CitcomSGlobalASCIIFile): $(OBJDIR)/Concrete/CitcomSGlobalASCIIFile.o \
                                           $(OBJDIR)/Concrete/CitcomSCfgFileParser.o

$(call PLUGINNAME,StructuredHexahedralTecplotASCIIFile): $(OBJDIR)/Concrete/TecplotASCIIFileHeaderParser.o \
                                                         $(OBJDIR)/Concrete/StructuredHexahedralTecplotASCIIFile.o

$(call PLUGINNAME,UnstructuredHexahedralTecplotASCIIFile): $(OBJDIR)/Concrete/TecplotASCIIFileHeaderParser.o \
                                                           $(OBJDIR)/Concrete/UnstructuredHexahedralTecplotASCIIFile.o

$(call PLUGINNAME,MultiChannelImageStack): PACKAGES += MYIMAGES

$(call PLUGINNAME,DicomImageStack): $(OBJDIR)/Concrete/HuffmanTable.o \
                                    $(OBJDIR)/Concrete/JPEGDecompressor.o \
                                    $(OBJDIR)/Concrete/DicomFile.o \
                                    $(OBJDIR)/Concrete/DicomImageStack.o

# Keep module object files around after building:
.SECONDARY: $(MODULE_NAMES:%=$(OBJDIR)/Concrete/%.o)

#
# Rule to build shared Visualizer server
#

SHAREDVISUALIZATIONSERVER_SOURCES = SharedVisualizationPipe.cpp \
                                    SharedVisualizationServer.cpp \
                                    SharedVisualizationServerMain.cpp

$(BINDIR)/SharedVisualizationServer: VRUI_LINKFLAGS += -lCollaboration.g++-3
$(BINDIR)/SharedVisualizationServer: $(SHAREDVISUALIZATIONSERVER_SOURCES:%.cpp=$(OBJDIR)/%.o)
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@g++ -o $@ $^ $(VRUI_LINKFLAGS) $(VRUI_PLUGINHOSTLINKFLAGS)

#
# Rule to install 3D Visualizer in a destination directory
#

install: $(ALL)
	@echo Installing 3D Visualizer in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(BININSTALLDIR)
	@install $(BINDIR)/3DVisualizer $(BININSTALLDIR)
ifneq ($(USE_COLLABORATION),0)
	@install $(BINDIR)/SharedVisualizationServer $(BININSTALLDIR)
endif
	@install -d $(PLUGININSTALLDIR)
	@install $(MODULES) $(PLUGININSTALLDIR)
	@install -d $(SHAREINSTALLDIR)
	@install $(RESOURCEDIR)/EarthTopography.png $(RESOURCEDIR)/EarthTopography.ppm $(SHAREINSTALLDIR)
ifneq ($(USE_SHADERS),0)
	@install -d $(SHAREINSTALLDIR)/Shaders
	@install $(SHADERS:%=$(RESOURCEDIR)/Shaders/%) $(SHAREINSTALLDIR)/Shaders
endif
