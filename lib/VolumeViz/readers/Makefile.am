DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

# **************************************************************************
#
# PublicHeaders    - header files that should be installed
# PrivateHeaders   - source files that should be distributed, but not
#                    installed - including source-included source files.
# RegularSources   - source files compiled in this directory

RegularSources = \
	VolumeReader.cpp \
	VRVolFileReader.cpp \
	VRMemReader.cpp

PublicHeaders = \
	SoVolumeReader.h \
	SoVRVolFileReader.h

PrivateHeaders = \
	SoVRMemReader.h

# **************************************************************************

if BUILD_WITH_MSVC
noinst_LIBRARIES = readers.lst
else
noinst_LTLIBRARIES = libreaders.la
endif

libreaders_la_SOURCES = $(RegularSources)
EXTRA_libreaders_la_SOURCES = $(PublicHeaders) $(PrivateHeaders)

readers_lst_SOURCES = $(RegularSources)
EXTRA_readers_lst_SOURCES = $(PublicHeaders) $(PrivateHeaders)

# **************************************************************************

libreadersincdir = $(includedir)/VolumeViz/readers
libreadersinc_HEADERS = $(PublicHeaders)

# **************************************************************************

readers.lst: $(readers_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(readers_lst_OBJECTS); do echo $$i >>$@; done

install-libreadersincHEADERS: $(libreadersinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libreadersinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libreadersincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libreadersincdir); \
	fi
	@list='$(libreadersinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libreadersincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libreadersincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

# **************************************************************************
