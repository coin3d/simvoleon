## Process this file with automake to generate Makefile.in.

SUBDIRS = bin lib

BUILT_SOURCES = \
	lib/VolumeViz/C/basic.h \
	simvoleon-@CONFIG@.cfg

DISTCLEANFILES = \
	simvoleon-@CONFIG@.cfg

EXTRA_DIST = \
	simvoleon.cfg.in \
	testcode/Makefile.in

cfgdatadir = $(datadir)/Coin/conf
cfgdata_DATA = simvoleon-@CONFIG@.cfg

simvoleon-@CONFIG@.cfg: config.status $(srcdir)/simvoleon.cfg.in
	@$(SHELL) config.status --file=$@:simvoleon.cfg.in

# sources needed to generate docs must be created by this rule
built-sources: $(BUILT_SOURCES)

lib/VolumeViz/C/basic.h: $(srcdir)/lib/VolumeViz/C/basic.h.in config.status
	@./config.status --header=$@
