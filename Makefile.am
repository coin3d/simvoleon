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
	simvoleon-@CONFIG@.cfg \
	Info.plist \
	version.plist

DISTCLEANFILES = \
	Info.plist \
	version.plist \
	simvoleon-@CONFIG@.cfg

CLEANFILES = \
	vc70.pdb

EXTRA_DIST = \
	simvoleon.cfg.in \
	SIMVoleon.pc.in \
	Info.plist.in \
	version.plist.in \
	cfg/errors.txt \
	cfg/wrapmsvc.exe \
	cfg/gendsp.sh \
	packaging/Windows/NSIS/Makefile.in \
	packaging/Windows/NSIS/bfheader.nsi.in \
	packaging/Windows/NSIS/footer.nsi.in \
	packaging/Windows/NSIS/heading.nsi.in \
	packaging/Windows/NSIS/middle.nsi.in \
	packaging/Windows/NSIS/post.nsi.in \
	packaging/Windows/NSIS/simvoleon-sdk.html.in \
	docs/ChangeLog.v2.0.0 \
	docs/ChangeLog.v2.0.1 \
	docs/ChangeLog.v2.1.0 \
	docs/announcement-1_0_0.txt \
	docs/announcement-2_0_0.txt \
	docs/announcement-2_0_1.txt

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = cfg/simvoleon.m4

cfgdatadir = $(datadir)/Coin/conf
cfgdata_DATA = simvoleon-@CONFIG@.cfg

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= SIMVoleon.pc

if MAC_FRAMEWORK
frameworkdatadir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Resources
frameworkdata_DATA = Info.plist version.plist
endif

Info.plist: config.status $(srcdir)/Info.plist.in
	@$(SHELL) config.status --file=$@:Info.plist.in

version.plist: config.status $(srcdir)/version.plist.in
	@$(SHELL) config.status --file=$@:version.plist.in

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
# misc rules for automatic NSIS packaging.
# main: `windows-packages'

windows-files:
	@if test -d windows; then :; else mkdir windows; fi
	@for file in $(top_srcdir)/packaging/windows/NSIS/*.in; do \
	  basename=`echo $$file | sed 's/^.*\///g; s/\.in$$//'`; \
	  ./config.status --file=windows/$$basename:$$file; \
	done

windows-packages: windows-files
	@( cd windows; $(MAKE) VERSION=$(VERSION) )

# **************************************************************************
