DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = misc.lst
else
noinst_LTLIBRARIES = libmisc.la
endif

RegularSources = \
	CvrVoxelChunk.cpp CvrVoxelChunk.h \
	CvrCLUT.cpp CvrCLUT.h \
	CvrUtil.cpp CvrUtil.h \
	CvrResourceManager.cpp CvrResourceManager.h \
	CvrGlobalRenderLock.h CvrGlobalRenderLock.cpp \
	CvrGIMPGradient.cpp CvrGIMPGradient.h \
	CvrGradient.cpp CvrGradient.h \
	CvrCentralDifferenceGradient.cpp CvrCentralDifferenceGradient.h

libmisc_la_SOURCES = $(RegularSources)

misc_lst_SOURCES = $(RegularSources)

misc.lst: $(misc_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(misc_lst_OBJECTS); do echo $$i >>$@; done
