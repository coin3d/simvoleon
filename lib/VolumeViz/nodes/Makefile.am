
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

SOURCES = \
	SoTransferFunction.cpp \
	SoVolumeData.cpp \
	SoVolumeRender.cpp \
	SoVolumeRendering.cpp

libnodes_la_SOURCES = $(SOURCES)

nodes_lst_SOURCES = $(SOURCES)

libnodesincdir = $(includedir)/VolumeViz/nodes

libnodesinc_HEADERS = \
	SoObliqueSlice.h \
	SoOrthoSlice.h \
	SoROI.h \
	SoTransferFunction.h \
	SoVolumeData.h \
	SoVolumeRender.h \
	SoVolumeRendering.h
	

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

