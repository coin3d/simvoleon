SUBDIRS = gradients

DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

SOURCES = \
	TransferFunction.cpp \
	VolumeData.cpp \
	VolumeRender.cpp \
	VolumeRendering.cpp \
	OrthoSlice.cpp \
	ObliqueSlice.cpp \
	ROI.cpp

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

