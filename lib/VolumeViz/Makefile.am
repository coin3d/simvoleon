
INCLUDES = -I$(top_srcdir)/lib

SUBDIRS = details elements nodes readers .

if BUILD_WITH_MSVC
lib_LIBRARIES = VolumeViz@VOLUMEVIZ_MAJOR_VERSION@.lib
else
lib_LTLIBRARIES = libVolumeViz.la
endif

libVolumeViz_la_SOURCES =

libVolumeViz_la_LIBADD = \
        details/libdetails.la \
        elements/libelements.la \
	nodes/libnodes.la \
        readers/libreaders.la

libVolumeViz_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_SOURCES =

VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_LIBADD = \
        details/details.lst \
        elements/elements.lst \
        nodes/nodes.lst \
        readers/readers.lst

VolumeViz@VOLUMEVIZ_MAJOR_VERSION@.lib: Makefile \
  $(libVolumeViz_lib_OBJECTS) \
  $(libVolumeViz_lib_DEPENDENCIES)
	@basename="VolumeViz@VOLUMEVIZ_MAJOR_VERSION@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_OBJECTS) \
	  $(VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_OBJECTS) \
	  $(VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_LIBADD) $(LIBS);

# **************************************************************************

install-exec-local:
	@basename="VolumeViz@VOLUMEVIZ_MAJOR_VERSION@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	  fi \
	done

install-data-local:
	@basename="VolumeViz@VOLUMEVIZ_MAJOR_VERSION@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	  fi \
	done

uninstall-local:
	@basename="VolumeViz@VOLUMEVIZ_MAJOR_VERSION@"; \
	for file in \
	  "$(DESTDIR)$(bindir)/$$basename.dll" \
	  "$(DESTDIR)$(bindir)/$$basename.pdb" \
	  "$(DESTDIR)$(libdir)/$$basename.lib"; do \
	  if test -f "$$file"; then \
	    echo " rm -f" "$$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	incbasedir="$(DESTDIR)$(includedir)/VolumeViz"; \
	for dir in \
	  "$$incbasedir/details" \
	  "$$incbasedir/elements" \
	  "$$incbasedir/nodes" \
	  "$$incbasedir/readers" \
	  "$$incbasedir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir" "$$dir"; \
	    rmdir "$$dir"; \
	  fi \
	done

