## Process this file with automake to generate Makefile.in.

SUBDIRS = lib

BUILT_SOURCES = \
	lib/VolumeViz/C/basic.h

# sources needed to generate docs must be created by this rule
built-sources: $(BUILT_SOURCES)

lib/VolumeViz/C/basic.h: $(srcdir)/lib/VolumeViz/C/basic.h.in config.status
	@./config.status --header=$@
