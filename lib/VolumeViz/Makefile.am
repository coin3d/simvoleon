INCLUDES = -I$(top_srcdir)/lib

# Note misc/ must be built after nodes/gradients/.
SUBDIRS = elements nodes misc readers render .

if BUILD_WITH_MSVC
lib_LIBRARIES = VolumeViz@VOLUMEVIZ_MAJOR_VERSION@.lib
else
lib_LTLIBRARIES = libVolumeViz.la
endif

# Adding in a dummy C++ source code file to "trick" Automake to set
# us up with C++ $CXXLD for linking (and not the default C $CCLD).
libVolumeViz_la_SOURCES = dummy.cpp

libVolumeViz_la_LIBADD = \
        elements/libelements.la \
	misc/libmisc.la \
	nodes/libnodes.la \
        readers/libreaders.la \
	render/2D/lib2Drender.la

libVolumeViz_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_SOURCES = dummy.cpp

VolumeViz@VOLUMEVIZ_MAJOR_VERSION@_lib_LIBADD = \
        elements/elements.lst \
        misc/misc.lst \
        nodes/nodes.lst \
        readers/readers.lst \
	render/2D/2Drender.lst

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
	  "$$incbasedir/elements" \
	  "$$incbasedir/nodes" \
	  "$$incbasedir/readers" \
	  "$$incbasedir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir" "$$dir"; \
	    rmdir "$$dir"; \
	  fi \
	done
