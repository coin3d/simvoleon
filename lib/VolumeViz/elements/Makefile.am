DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

# **************************************************************************
#
# PublicHeaders    - header files that should be installed
# PrivateHeaders   - source files that should be distributed, but not
#                    installed - including source-included source files.
# RegularSources   - source files compiled in this directory

RegularSources = \
	CompressedTexturesElement.cpp \
	GLInterpolationElement.cpp \
	PalettedTexturesElement.cpp \
	PageSizeElement.cpp \
	StorageHintElement.cpp \
	VoxelBlockElement.cpp \
	TransferFunctionElement.cpp

PublicHeaders = 

PrivateHeaders = \
	SoTransferFunctionElement.h \
	CvrCompressedTexturesElement.h \
	CvrGLInterpolationElement.h \
	CvrPalettedTexturesElement.h \
	CvrPageSizeElement.h \
	CvrStorageHintElement.h \
	CvrVoxelBlockElement.h

# **************************************************************************

if BUILD_WITH_MSVC
noinst_LIBRARIES = elements.lst
else
noinst_LTLIBRARIES = libelements.la
endif

libelements_la_SOURCES = $(RegularSources)
EXTRA_libelements_la_SOURCES = $(PublicHeaders) $(PrivateHeaders)

elements_lst_SOURCES = $(RegularSources)
EXTRA_elements_lst_SOURCES = $(PublicHeaders) $(PrivateHeaders)

# **************************************************************************

libelementsincdir = $(includedir)/VolumeViz/elements
libelementsinc_HEADERS = $(PublicHeaders)

# **************************************************************************

elements.lst: $(elements_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(elements_lst_OBJECTS); do echo $$i >>$@; done

install-libelementsincHEADERS: $(libelementsinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libelementsinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libelementsincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libelementsincdir); \
	fi
	@list='$(libelementsinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libelementsincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libelementsincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

# **************************************************************************
