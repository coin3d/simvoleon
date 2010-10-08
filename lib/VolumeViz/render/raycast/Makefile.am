DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = raycastrender.lst
else
noinst_LTLIBRARIES = libraycastrender.la
endif

RegularSources = \
        CvrRaycastSubCube.cpp CvrRaycastSubCube.h \
        CvrRaycastCube.cpp CvrRaycastCube.h \
        CvrRaycastTexture.cpp CvrRaycastTexture.h \
        SoRaycastRendering.cpp SoRaycastRendering.h

libraycastrender_la_SOURCES = $(RegularSources)

raycastrender_lst_SOURCES = $(RegularSources)

raycastrender.lst: $(raycastrender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(raycastrender_lst_OBJECTS); do echo $$i >>$@; done
