DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = 3Drender.lst
else
noinst_LTLIBRARIES = lib3Drender.la
endif

RegularSources = \
	CvrCubeHandler.cpp CvrCubeHandler.h \
	Cvr3DTexSubCube.cpp Cvr3DTexSubCube.h \
	Cvr3DTexCube.cpp Cvr3DTexCube.h

lib3Drender_la_SOURCES = $(RegularSources)

3Drender_lst_SOURCES = $(RegularSources)

3Drender.lst: $(3Drender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(3Drender_lst_OBJECTS); do echo $$i >>$@; done
