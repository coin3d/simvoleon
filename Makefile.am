## Process this file with automake to generate Makefile.in.

SUBDIRS = bin lib

BUILT_SOURCES = \
	lib/VolumeViz/C/basic.h \
	simvoleon-@CONFIG@.cfg

DISTCLEANFILES = \
	simvoleon-@CONFIG@.cfg

CLEANFILES = \
	simvoleon1.dsp \
	simvoleon1.dsw \
	simvoleon1.ncb \
	simvoleon1.opt \
	simvoleon1.plg \
	simvoleon1.dll \
	simvoleon1.ilk \
	simvoleon1d.dll \
	simvoleon1d.ilk

EXTRA_DIST = \
	simvoleon.cfg.in

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = cfg/simvoleon.m4

cfgdatadir = $(datadir)/Coin/conf
cfgdata_DATA = simvoleon-@CONFIG@.cfg

simvoleon-@CONFIG@.cfg: config.status $(srcdir)/simvoleon.cfg.in
	@$(SHELL) config.status --file=$@:simvoleon.cfg.in

# sources needed to generate docs must be created by this rule
built-sources: $(BUILT_SOURCES)

lib/VolumeViz/C/basic.h: $(srcdir)/lib/VolumeViz/C/basic.h.in config.status
	@./config.status --header=$@
