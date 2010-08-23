DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = details.lst
else
noinst_LTLIBRARIES = libdetails.la
endif

RegularSources = \
	SoVolumeDetail.cpp \
	SoVolumeRenderDetail.cpp \
	SoVolumeSkinDetail.cpp \
	SoOrthoSliceDetail.cpp \
	SoObliqueSliceDetail.cpp

libdetails_la_SOURCES = $(RegularSources)

details_lst_SOURCES = $(RegularSources)

if MAC_FRAMEWORK
libdetailsincdir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Headers/details
else
libdetailsincdir = $(includedir)/VolumeViz/details
endif

libdetailsinc_HEADERS = \
	SoVolumeDetail.h \
	SoVolumeRenderDetail.h \
	SoVolumeSkinDetail.h \
	SoOrthoSliceDetail.h \
	SoObliqueSliceDetail.h

details.lst: $(details_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(details_lst_OBJECTS); do echo $$i >>$@; done

install-libdetailsincHEADERS: $(libdetailsinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libdetailsinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libdetailsincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libdetailsincdir); \
	fi
	@list='$(libdetailsinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libdetailsincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libdetailsincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

