
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = details.lst
else
noinst_LTLIBRARIES = libdetails.la
endif

SOURCES = \
        VolumeRenderDetail.cpp \
        OrthoSliceDetail.cpp \
        ObliqueSliceDetail.cpp

libdetails_la_SOURCES = $(SOURCES)

details_lst_SOURCES = $(SOURCES)

libdetailsincdir = $(includedir)/VolumeViz/details

libdetailsinc_HEADERS = \
        SoVolumeRenderDetail.h \
        SoOrthoSliceDetail.h \
        SoObliqueSliceDetail.h


details.lst: $(details_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(details_lst_OBJECTS); do echo $$i >>$@; done
