BUILT_SOURCES = \
	basic.h

DISTCLEANFILES = \
	basic.h

EXTRA_DIST = \
	basic.h.in

basic.h: $(srcdir)/basic.h.in
	@( cd $(top_builddir); $(MAKE) built-sources )

simvoleoncincdir = $(includedir)/VolumeViz/C
nodist_simvoleoncinc_HEADERS = basic.h
