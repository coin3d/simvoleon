INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = misc.lst
else
noinst_LTLIBRARIES = libmisc.la
endif

SOURCES = \
	VoxelChunk.cpp CvrVoxelChunk.h \
	CLUT.cpp CvrCLUT.h \
	Util.cpp CvrUtil.h \
	GIMPGradient.cpp CvrGIMPGradient.h

libmisc_la_SOURCES = $(SOURCES)

misc_lst_SOURCES = $(SOURCES)

misc.lst: $(misc_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(misc_lst_OBJECTS); do echo $$i >>$@; done
