# Note misc/ must be built after nodes/gradients/.
SUBDIRS = C details elements nodes misc readers render .

DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
lib_LIBRARIES = simvoleon@SIMVOLEON_MAJOR_VERSION@.lib
else
lib_LTLIBRARIES = libSimVoleon.la
endif

# Adding in a dummy C++ source code file to "trick" Automake to set
# us up with C++ $CXXLD for linking (and not the default C $CCLD).
libSimVoleon_la_SOURCES = dummy.cpp
simvoleon_lib_SOURCES = dummy.cpp

libSimVoleon_la_LIBADD = \
        details/libdetails.la \
        elements/libelements.la \
	misc/libmisc.la \
	nodes/libnodes.la \
        readers/libreaders.la \
	render/2D/lib2Drender.la

simvoleon_lib_LIBADD = \
        details/details.lst \
        elements/elements.lst \
        misc/misc.lst \
        nodes/nodes.lst \
        readers/readers.lst \
	render/2D/2Drender.lst

libSimVoleon_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

simvoleon@SIMVOLEON_MAJOR_VERSION@.lib: Makefile \
  $(simvoleon_lib_OBJECTS) \
  $(simvoleon_lib_DEPENDENCIES)
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(simvoleon_lib_OBJECTS) \
	  $(simvoleon_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(simvoleon_lib_OBJECTS) \
	  $(simvoleon_lib_LIBADD) $(LIBS);

# **************************************************************************

if BUILD_WITH_MSVC
install-exec-local: simvoleon@SIMVOLEON_MAJOR_VERSION@.lib
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	  fi \
	done
else
install-exec-local:
endif

install-data-local:
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	  fi \
	done

uninstall-local:
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@"; \
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
