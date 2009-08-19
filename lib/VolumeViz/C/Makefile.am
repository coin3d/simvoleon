BUILT_SOURCES = \
	basic.h

DISTCLEANFILES = \
	basic.h \
	stamp-h3

EXTRA_DIST = \
	basic.h.in

basic.h: $(srcdir)/basic.h.in
	@( cd $(top_builddir); $(MAKE) built-sources )

if MAC_FRAMEWORK
simvoleoncincdir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Headers/C
else
simvoleoncincdir = $(includedir)/VolumeViz/C
endif

nodist_simvoleoncinc_HEADERS = basic.h
