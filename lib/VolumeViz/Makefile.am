# Note misc/ must be built after nodes/gradients/.
SUBDIRS = C caches details elements nodes misc readers render .

DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
lib_LIBRARIES = simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.lib
else
lib_LTLIBRARIES = libSimVoleon@SUFFIX@.la
endif

# Adding in a dummy C++ source code file to "trick" Automake to set
# us up with C++ $CXXLD for linking (and not the default C $CCLD).
libSimVoleon@SUFFIX@_la_SOURCES = dummy.cpp
simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_SOURCES = dummy.cpp

libSimVoleon@SUFFIX@_la_LIBADD = \
	caches/libcaches.la \
	details/libdetails.la \
	elements/libelements.la \
	misc/libmisc.la \
	nodes/libnodes.la \
	readers/libreaders.la \
	render/2D/lib2Drender.la \
        render/3D/lib3Drender.la \
        render/Pointset/libpointsetrender.la \
        render/common/libcommonrender.la

simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_LIBADD = \
	caches/caches.lst \
	details/details.lst \
	elements/elements.lst \
	misc/misc.lst \
	nodes/nodes.lst \
	readers/readers.lst \
	render/2D/2Drender.lst \
        render/3D/3Drender.lst \
        render/Pointset/pointsetrender.lst \
        render/common/commonrender.lst

libSimVoleon@SUFFIX@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.lib: Makefile \
  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_DEPENDENCIES)
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS);

# **************************************************************************

if BUILD_WITH_MSVC
install-exec-local: simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.lib
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file" || exit 1; \
	  fi \
	done
else
install-exec-local:
endif

install-data-local:
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file" || exit 1; \
	  fi \
	done

uninstall-local:
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@"; \
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
	  "$$incbasedir/C" \
	  "$$incbasedir/caches" \
	  "$$incbasedir/details" \
	  "$$incbasedir/elements" \
	  "$$incbasedir/nodes" \
	  "$$incbasedir/readers" \
	  "$$incbasedir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir" "$$dir"; \
	    rmdir "$$dir"; \
	  fi \
	done; \
	:

