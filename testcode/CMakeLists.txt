add_subdirectory(tabula)
configure_file(doc-example.cpp.in doc-example.cpp @ONLY)
executable(doc-example SOURCES "${CMAKE_CURRENT_BINARY_DIR}/doc-example.cpp" LIBS SIMVoleon ${EXAMPLE_LINK_LIB})
