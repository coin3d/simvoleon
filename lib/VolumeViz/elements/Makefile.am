DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = elements.lst
else
noinst_LTLIBRARIES = libelements.la
endif

SOURCES = \
	VolumeDataElement.cpp \
	TransferFunctionElement.cpp

libelements_la_SOURCES = $(SOURCES)

elements_lst_SOURCES = $(SOURCES)

libelementsincdir = $(includedir)/VolumeViz/elements

libelementsinc_HEADERS = \
	SoVolumeDataElement.h \
	SoTransferFunctionElement.h

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

