DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = 3Drender.lst
else
noinst_LTLIBRARIES = lib3Drender.la
endif

SOURCES = \
	CubeHandler.cpp CvrCubeHandler.h \
	3DTexSubCube.cpp Cvr3DTexSubCube.h \
	3DTexCube.cpp Cvr3DTexCube.h

lib3Drender_la_SOURCES = $(SOURCES)

3Drender_lst_SOURCES = $(SOURCES)

3Drender.lst: $(3Drender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(3Drender_lst_OBJECTS); do echo $$i >>$@; done
