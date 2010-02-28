DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = pointsetrender.lst
else
noinst_LTLIBRARIES = libpointsetrender.la
endif

RegularSources = \
	PointRendering.cpp PointRendering.h

libpointsetrender_la_SOURCES = $(RegularSources)

pointsetrender_lst_SOURCES = $(RegularSources)

pointsetrender.lst: $(pointsetrender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(pointsetrender_lst_OBJECTS); do echo $$i >>$@; done
