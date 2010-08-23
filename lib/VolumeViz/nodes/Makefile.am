SUBDIRS = gradients

DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

# **************************************************************************
#
# PublicHeaders    - header files that should be installed
# PrivateHeaders   - source files that should be distributed, but not
#                    installed - including source-included source files.
# RegularSources   - source files compiled in this directory

RegularSources = \
	volumeraypickintersection.cpp \
	SoTransferFunction.cpp \
	SoVolumeData.cpp \
	SoVolumeRender.cpp \
	SoVolumeRendering.cpp \
	SoObliqueSlice.cpp \
	SoOrthoSlice.cpp \
	SoVolumeSkin.cpp \
	SoVolumeFaceSet.cpp \
	SoVolumeIndexedFaceSet.cpp \
	SoVolumeTriangleStripSet.cpp \
	SoVolumeIndexedTriangleStripSet.cpp \
	CvrIndexedSetRenderBaseP.cpp \
	CvrNonIndexedSetRenderBaseP.cpp \
	CvrFaceSetRenderP.cpp \
	CvrIndexedFaceSetRenderP.cpp \
	CvrTriangleStripSetRenderP.cpp \
	CvrIndexedTriangleStripSetRenderP.cpp \
        SoRaycastRender.cpp

PublicHeaders = \
	SoOrthoSlice.h \
	SoVolumeSkin.h \
	SoObliqueSlice.h \
	SoTransferFunction.h \
	SoVolumeData.h \
	SoVolumeRender.h \
	SoVolumeRendering.h \
	SoVolumeFaceSet.h \
	SoVolumeIndexedFaceSet.h \
	SoVolumeTriangleStripSet.h \
	SoVolumeIndexedTriangleStripSet.h \
        SoRaycastRender.h

PrivateHeaders = \
	volumeraypickintersection.h \
	CvrIndexedSetRenderBaseP.h \
	CvrNonIndexedSetRenderBaseP.h \
	CvrFaceSetRenderP.h \
	CvrIndexedFaceSetRenderP.h \
	CvrTriangleStripSetRenderP.h \
	CvrIndexedTriangleStripSetRenderP.h

# **************************************************************************

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

libnodes_la_SOURCES = $(RegularSources)
EXTRA_libnodes_la_SOURCES = $(PublicHeaders) $(PrivateHeaders)

nodes_lst_SOURCES = $(RegularSources)
EXTRA_nodes_lst_SOURCES = $(PublicHeaders) $(PrivateHeaders)

# **************************************************************************

if MAC_FRAMEWORK
libnodesincdir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Headers/nodes
else
libnodesincdir = $(includedir)/VolumeViz/nodes
endif

libnodesinc_HEADERS = $(PublicHeaders)

# **************************************************************************

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

install-libnodesincHEADERS: $(libnodesinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libnodesinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libnodesincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libnodesincdir); \
	fi
	@list='$(libnodesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libnodesincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libnodesincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

# **************************************************************************
