# **************************************************************************

if BUILD_LIBRARY
SRCDIRS = lib
else
SRCDIRS =
endif

if BUILD_HTMLPAGES
HTMLDIRS = html
else
HTMLDIRS =
endif

# **************************************************************************

SUBDIRS = bin $(SRCDIRS) $(HTMLDIRS)


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
	simvoleon.cfg.in \
	cfg/errors.txt \
	cfg/wrapmsvc.exe

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

docs/simvoleon.doxygen: $(srcdir)/docs/simvoleon.doxygen.in config.status
	@if test -d docs; then :; else mkdir docs; fi
	@./config.status --file=$@:$@.in
	@$(srcdir)/cfg/doxy4win.pl docs/simvoleon.doxygen

# **************************************************************************
