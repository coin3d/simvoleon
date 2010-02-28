DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = 2Drender.lst
else
noinst_LTLIBRARIES = lib2Drender.la
endif

RegularSources = \
	PageHandler.cpp CvrPageHandler.h \
	2DTexSubPage.cpp Cvr2DTexSubPage.h \
	2DTexPage.cpp Cvr2DTexPage.h

EXTRA_DIST = \
	texmemfullimg.h

lib2Drender_la_SOURCES = $(RegularSources)

2Drender_lst_SOURCES = $(RegularSources)

2Drender.lst: $(2Drender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(2Drender_lst_OBJECTS); do echo $$i >>$@; done
