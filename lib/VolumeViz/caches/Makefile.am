DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = caches.lst
else
noinst_LTLIBRARIES = libcaches.la
endif

RegularSources = \
	GLTextureCache.cpp CvrGLTextureCache.h

libcaches_la_SOURCES = $(RegularSources)

caches_lst_SOURCES = $(RegularSources)

caches.lst: $(caches_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(caches_lst_OBJECTS); do echo $$i >>$@; done
