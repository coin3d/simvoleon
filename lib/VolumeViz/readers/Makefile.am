
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = readers.lst
else
noinst_LTLIBRARIES = libreaders.la
endif

SOURCES = \
	SoVolumeReader.cpp

libreaders_la_SOURCES = $(SOURCES)

readers_lst_SOURCES = $(SOURCES)

libreadersincdir = $(includedir)/VolumeViz/readers

libreadersinc_HEADERS = \
	SoVolumeReader.h

readers.lst: $(readers_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(readers_lst_OBJECTS); do echo $$i >>$@; done

