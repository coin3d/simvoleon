INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = 2Drender.lst
else
noinst_LTLIBRARIES = lib2Drender.la
endif

SOURCES = \
	PageHandler.cpp CvrPageHandler.h \
	2DTexSubPage.cpp Cvr2DTexSubPage.h \
	2DTexPage.cpp Cvr2DTexPage.h \
	TextureObject.cpp CvrTextureObject.h \
	TexManager.cpp CvrTexManager.h

lib2Drender_la_SOURCES = $(SOURCES)

2Drender_lst_SOURCES = $(SOURCES)

2Drender.lst: $(2Drender_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(2Drender_lst_OBJECTS); do echo $$i >>$@; done
