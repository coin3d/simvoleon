
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = elements.lst
else
noinst_LTLIBRARIES = libelements.la
endif

SOURCES = \
	VolumeDataElement.cpp \
	TransferFunctionElement.cpp

libelements_la_SOURCES = $(SOURCES)

elements_lst_SOURCES = $(SOURCES)

libelementsincdir = $(includedir)/VolumeViz/elements

libelementsinc_HEADERS = \
	SoVolumeDataElement.h \
	SoTransferFunctionElement.h

elements.lst: $(elements_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(elements_lst_OBJECTS); do echo $$i >>$@; done
