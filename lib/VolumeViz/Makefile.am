# Note misc/ must be built after nodes/gradients/.
SUBDIRS = C caches details elements nodes misc readers render .

# **************************************************************************

DEFAULT_INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib

# **************************************************************************

if BUILD_WITH_MSVC
lib_LIBRARIES = simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.lib
else
lib_LTLIBRARIES = libSimVoleon@SUFFIX@.la
endif

# On Mac OS 10.2 or later, create two-level namespace binaries and use
# prebinding. Also pad the install_name to make it possible to change it
# later (e.g. to include the library in an app bundle).

if MACOSX_10_2FF
LibtoolLinkArgs = -no-undefined -Wl,-twolevel_namespace -Wl,-undefined,error -Wl,-prebind -Wl,-seg1addr,0x30000000 -Wl,-headerpad_max_install_names 
else
LibtoolLinkArgs = -no-undefined
endif


if MAC_FRAMEWORK
# hack to change libdir value when installing Mac OS X compiler framework
lib@nop@dir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Libraries
endif


# **************************************************************************

# Adding in a dummy C++ source code file to "trick" Automake to set
# us up with C++ $CXXLD for linking (and not the default C $CCLD).
Sources = dummy.cpp \
	Coin/gl/CoinGLPerformance.cpp \
	Coin/gl/CoinGLPerformance.h \
        SIMVoleon.cpp SIMVoleon.h

libSimVoleon@SUFFIX@_la_SOURCES = $(Sources)
simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@_lib_SOURCES = $(Sources)

# **************************************************************************

libSimVoleon@SUFFIX@_la_LIBADD = \
	caches/libcaches.la \
	details/libdetails.la \
	elements/libelements.la \
	misc/libmisc.la \
	nodes/libnodes.la \
	readers/libreaders.la \
	render/2D/lib2Drender.la \
        render/3D/lib3Drender.la \
        render/pointset/libpointsetrender.la \
        render/raycast/libraycastrender.la \
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
        render/pointset/pointsetrender.lst \
        render/raycast/raycastrender.lst \
        render/common/commonrender.lst

libSimVoleon@SUFFIX@_la_LDFLAGS = \
        $(LibtoolLinkArgs) \
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

CLEANFILES = \
	simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.pdb \
	simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.dll \
	simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@.exp

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

if MAC_FRAMEWORK
install-data-local:
	@$(NORMAL_INSTALL)
	@echo "Setting up Mac OS X framework"
	@$(mkinstalldirs) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)
	@$(mkinstalldirs) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Resources
	@$(mkinstalldirs) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Resources/include
	@ln -sf $(MAC_FRAMEWORK_VERSION) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/Current
	@ln -sf Versions/Current/Headers $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Headers
	@ln -sf Versions/Current/Libraries $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Libraries
	@ln -sf Versions/Current/Resources $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Resources
	cd $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/Current && ln -sf Libraries/libSimVoleon.dylib $(MAC_FRAMEWORK_NAME) && cd -; \
	cd $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework && ln -sf Versions/Current/$(MAC_FRAMEWORK_NAME) $(MAC_FRAMEWORK_NAME) && install_name_tool -id $(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/$(MAC_FRAMEWORK_NAME) $(MAC_FRAMEWORK_NAME) && cd -;
        # Additional symlink to allow non-framework (-I) includes.
	cd $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Resources/include && rm -f $(MAC_FRAMEWORK_NAME) && ln -s ../../Headers $(MAC_FRAMEWORK_NAME) && cd -;
else
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
endif

uninstall-local:
	@basename="simvoleon@SIMVOLEON_MAJOR_VERSION@@SUFFIX@"; \
	for file in \
	  "$(DESTDIR)$(bindir)/$$basename.dll" \
	  "$(DESTDIR)$(bindir)/$$basename.pdb" \
	  "$(DESTDIR)$(libdir)/$$basename.lib" \
	  "$(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/$(MAKE_FRAMEWORK_NAME)"; do \
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

# **************************************************************************
