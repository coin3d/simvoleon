
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = misc.lst
else
noinst_LTLIBRARIES = libmisc.la
endif

SOURCES = \
	VolumeDataPage.cpp \
	VolumeDataSlice.cpp

libmisc_la_SOURCES = $(SOURCES)

misc_lst_SOURCES = $(SOURCES)

libmiscincdir = $(includedir)/VolumeViz/misc

libmiscinc_HEADERS = \
	SoVolumeDataSlice.h \
	SoVolumeDataPage.h

misc.lst: $(misc_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(misc_lst_OBJECTS); do echo $$i >>$@; done

