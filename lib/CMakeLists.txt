configure_file(config.h.cmake.in config.h)
configure_file(VolumeViz/C/basic.h.cmake.in VolumeViz/C/basic.h)

macro(strfy _input _postfix)
  set(COIN_INPUT_FILE_NAME ${_input})
  get_filename_component(_basename "${_input}" NAME_WE)
  get_filename_component(_suffix "${_input}" EXT)
  get_filename_component(_path "${_input}" PATH)
  string(SUBSTRING ${_suffix} 1 -1 _suffix)
  string(TOUPPER "${_basename}_${_suffix}_H" COIN_HEADER_DEF)
  string(TOUPPER ${_basename} COIN_TEXTVAR_NAME)
  set(COIN_TEXTVAR_NAME "${COIN_TEXTVAR_NAME}${_postfix}")
  #message(STATUS "Parse: ${CMAKE_CURRENT_SOURCE_DIR}/${_input}")
  FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/${_input} f0)
  STRING(REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f0}")
  STRING(REGEX REPLACE "\"" "\\\\\"" f2 "${f1}")
  STRING(REGEX REPLACE "\r?\n" "\\\\n\"\n  \"" COIN_STR_SOURCE_CODE "${f2}")
  configure_file(strfytemplate.cmake.in "${_path}/${_basename}.h")
endmacro()

# Note misc/ must be built after nodes/gradients/.
file(GLOB_RECURSE GRADIENTS LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/VolumeViz/nodes/gradients/" *.gg)
foreach(_input_file ${GRADIENTS})
  strfy(VolumeViz/nodes/gradients/${_input_file} "_gradient")
endforeach()

file(GLOB_RECURSE HDRS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE BIN_HDRS ${CMAKE_CURRENT_BINARY_DIR}/*.h)

set(SRCS
  VolumeViz/Coin/gl/CoinGLPerformance.cpp 
  VolumeViz/caches/GLTextureCache.cpp
  VolumeViz/details/ObliqueSliceDetail.cpp
  VolumeViz/details/OrthoSliceDetail.cpp
  VolumeViz/details/VolumeDetail.cpp
  VolumeViz/details/VolumeRenderDetail.cpp
  VolumeViz/details/VolumeSkinDetail.cpp
  VolumeViz/elements/CompressedTexturesElement.cpp
  VolumeViz/elements/GLInterpolationElement.cpp
  VolumeViz/elements/LightingElement.cpp
  VolumeViz/elements/PageSizeElement.cpp
  VolumeViz/elements/PalettedTexturesElement.cpp
  VolumeViz/elements/StorageHintElement.cpp
  VolumeViz/elements/TransferFunctionElement.cpp
  VolumeViz/elements/VoxelBlockElement.cpp
  VolumeViz/nodes/CvrFaceSetRenderP.cpp
  VolumeViz/nodes/CvrIndexedFaceSetRenderP.cpp
  VolumeViz/nodes/CvrIndexedSetRenderBaseP.cpp
  VolumeViz/nodes/CvrIndexedTriangleStripSetRenderP.cpp
  VolumeViz/nodes/CvrNonIndexedSetRenderBaseP.cpp
  VolumeViz/nodes/CvrTriangleStripSetRenderP.cpp
  VolumeViz/nodes/ObliqueSlice.cpp
  VolumeViz/nodes/OrthoSlice.cpp
  # VolumeViz/nodes/ROI.cpp                                 # not yet implemented?
  VolumeViz/nodes/TransferFunction.cpp
  VolumeViz/nodes/VolumeData.cpp
  VolumeViz/nodes/VolumeFaceSet.cpp
  VolumeViz/nodes/VolumeIndexedFaceSet.cpp
  VolumeViz/nodes/VolumeIndexedTriangleStripSet.cpp
  VolumeViz/nodes/volumeraypickintersection.cpp
  VolumeViz/nodes/VolumeRender.cpp
  VolumeViz/nodes/VolumeRendering.cpp
  VolumeViz/nodes/VolumeSkin.cpp
  VolumeViz/nodes/VolumeTriangleStripSet.cpp
  VolumeViz/misc/CentralDifferenceGradient.cpp
  VolumeViz/misc/CLUT.cpp
  VolumeViz/misc/GIMPGradient.cpp
  VolumeViz/misc/GlobalRenderLock.cpp
  VolumeViz/misc/Gradient.cpp
  VolumeViz/misc/ResourceManager.cpp
  VolumeViz/misc/Util.cpp
  VolumeViz/misc/VoxelChunk.cpp
  VolumeViz/readers/VolumeReader.cpp
  VolumeViz/readers/VRMemReader.cpp
  VolumeViz/readers/VRVolFileReader.cpp
  VolumeViz/render/2D/2DTexPage.cpp
  VolumeViz/render/2D/2DTexSubPage.cpp
  VolumeViz/render/2D/PageHandler.cpp
  VolumeViz/render/3D/3DTexCube.cpp
  VolumeViz/render/3D/3DTexSubCube.cpp
  VolumeViz/render/3D/CubeHandler.cpp
  VolumeViz/render/common/Cvr2DPaletteTexture.cpp
  VolumeViz/render/common/Cvr2DRGBATexture.cpp
  VolumeViz/render/common/Cvr3DPaletteGradientTexture.cpp
  VolumeViz/render/common/Cvr3DPaletteTexture.cpp
  VolumeViz/render/common/Cvr3DRGBATexture.cpp
  VolumeViz/render/common/CvrPaletteTexture.cpp
  VolumeViz/render/common/CvrRGBATexture.cpp
  VolumeViz/render/common/CvrTextureObject.cpp
  VolumeViz/render/Pointset/PointRendering.cpp
)

if(WIN32)
  if(MINGW)
    set(SIMVOLEON_DEFAULT_SHARED_POSTFIX "")
    set(SIMVOLEON_DEFAULT_STATIC_POSTFIX "")
  else()
    # On Windows the major version number is part of the library name
    set(SIMVOLEON_DEFAULT_SHARED_POSTFIX ${PROJECT_VERSION_MAJOR})
    set(SIMVOLEON_DEFAULT_STATIC_POSTFIX ${PROJECT_VERSION_MAJOR}s)
  endif()
  if(SIMVOLEON_BUILD_SHARED_LIBS)
    set(SIMVOLEON_DEFAULT_POSTFIX ${SIMVOLEON_DEFAULT_SHARED_POSTFIX})
  else()
    set(SIMVOLEON_DEFAULT_POSTFIX ${SIMVOLEON_DEFAULT_STATIC_POSTFIX})
  endif()
  set(CMAKE_RELEASE_POSTFIX ${SIMVOLEON_DEFAULT_POSTFIX})
  set(CMAKE_MINSIZEREL_POSTFIX ${SIMVOLEON_DEFAULT_POSTFIX})
  set(CMAKE_RELWITHDEBINFO_POSTFIX ${SIMVOLEON_DEFAULT_POSTFIX})
  set(CMAKE_DEBUG_POSTFIX ${SIMVOLEON_DEFAULT_POSTFIX}d)
elseif(APPLE)
else()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(SIMVOLEON_BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED ${HDRS} ${BIN_HDRS} ${SRCS})
else()
  add_library(${PROJECT_NAME} STATIC ${HDRS} ${BIN_HDRS} ${SRCS})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${SIMVOLEON_VERSION} SOVERSION ${SIMVOLEON_SO_VERSION})

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CONFIG_H SIMVOLEON_INTERNAL SIMVOLEON_DEBUG=$<CONFIG:Debug>)

if(WIN32)
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
  endif()
  if(SIMVOLEON_BUILD_SHARED_LIBS)
    configure_file(${PROJECT_NAME}.rc.cmake.in ${PROJECT_NAME}.rc)
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SIMVOLEON_MAKE_DLL INTERFACE SIMVOLEON_DLL)
  else()
    target_compile_definitions(${PROJECT_NAME} INTERFACE SIMVOLEON_NOT_DLL)
    if(MSVC)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")                       # Override default /Zi to embed
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")     # debugging info in the .lib.
    endif()
  endif()
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} Coin::Coin ${OPENGL_LIBRARIES})

# Add a target to generate API documentation with Doxygen
if(SIMVOLEON_BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  set(DOXYGEN_TAGFILES "${Coin_DOC_DIR}/html/Coin.tag=${Coin_DOC_DIR}/html")

# ############################################################################
# Setup documentation options
# ############################################################################
  set(GENERATE_HTMLHELP NO)
  set(DOXYGEN_GENERATE_MAN NO)
  set(GENERATE_QHP NO)
  set(GENERATE_TREEVIEW YES)
  set(HHC_PROGRAM)
  if(WIN32)
    if(SIMVOLEON_BUILD_DOC_CHM)
      find_program(HHC_PROGRAM NAMES hhc.exe PATHS "C:/Program Files/HTML Help Workshop" "C:/Program Files (x86)/HTML Help Workshop" DOC "HTML Help Compiler program")
      if(NOT HHC_PROGRAM)
        message(FATAL_ERROR "Missing program HTML Help Compiler")
      else()
        set(GENERATE_HTMLHELP YES)
        set(GENERATE_TREEVIEW NO)
        mark_as_advanced(HHC_PROGRAM)
      endif()
    endif()
  else()
    if(SIMVOLEON_BUILD_DOC_MAN)
      set(DOXYGEN_GENERATE_MAN YES)
    endif()
  endif()
  if(SIMVOLEON_BUILD_DOC_QTHELP)
    find_program(QHG_LOCATION NAMES qhelpgenerator qhelpgenerator-qt5 DOC "Qt qhelpgenerator")
    if(NOT QHG_LOCATION)
      message(FATAL_ERROR "Missing program Qt qhelpgenerator")
    else()
      set(GENERATE_QHP YES)
      mark_as_advanced(QHG_LOCATION)
    endif()
  endif()

  set(DOXYGEN_INTERNAL_DOCS NO)
  set(DOXYGEN_EXTRACT_PRIVATE NO)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
  set(DOXYGEN_EXCLUDE)

  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
  set(DOXYGEN_OUTPUT "${CMAKE_BINARY_DIR}/html/index.html")
  configure_file("${PROJECT_SOURCE_DIR}/docs/${PROJECT_NAME_LOWER}.doxygen.cmake.in" ${DOXYFILE} @ONLY)

# ############################################################################
# Setup documentation targets
# ############################################################################
  add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E echo_append "Generating API documentation with Doxygen "
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS ${DOXYFILE}
  )
  add_custom_target(documentation ALL DEPENDS ${DOXYGEN_OUTPUT})

# ############################################################################
# Install built documentation files
# ############################################################################
  install(DIRECTORY "${CMAKE_BINARY_DIR}/html" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation REGEX ".*\\.(chm|qch)" EXCLUDE)
  if(GENERATE_HTMLHELP)
    install(FILES "${CMAKE_BINARY_DIR}/html/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.chm" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation)
  endif()
  if(GENERATE_QHP)
    install(FILES "${CMAKE_BINARY_DIR}/html/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.qch" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT documentation)
  endif()
  if(DOXYGEN_GENERATE_MAN)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/man/man3" DESTINATION ${CMAKE_INSTALL_MANDIR} COMPONENT documentation)
    message(STATUS "CMAKE_INSTALL_MANDIR ${CMAKE_INSTALL_MANDIR}")
  endif()
endif()

# ############################################################################
# Install headers
# ############################################################################

set(INST_C_HDRS
  "${CMAKE_CURRENT_BINARY_DIR}/VolumeViz/C/basic.h"
)

set(INST_ELEMENTS_HDRS
  VolumeViz/elements/CvrPageSizeElement.h
  VolumeViz/elements/CvrVoxelBlockElement.h
  VolumeViz/elements/CvrGLInterpolationElement.h
  VolumeViz/elements/CvrLightingElement.h
  VolumeViz/elements/CvrPalettedTexturesElement.h
  VolumeViz/elements/CvrStorageHintElement.h
  VolumeViz/elements/SoTransferFunctionElement.h
  VolumeViz/elements/CvrCompressedTexturesElement.h
)

set(INST_NODES_HDRS
  VolumeViz/nodes/SoVolumeFaceSet.h
  VolumeViz/nodes/SoTransferFunction.h
  VolumeViz/nodes/SoVolumeRender.h
  VolumeViz/nodes/SoVolumeIndexedTriangleStripSet.h
  VolumeViz/nodes/SoVolumeIndexedFaceSet.h
  VolumeViz/nodes/SoVolumeSkin.h
  # VolumeViz/nodes/SoROI.h                             # not yet implemented?!
  VolumeViz/nodes/SoVolumeTriangleStripSet.h
  VolumeViz/nodes/SoObliqueSlice.h
  VolumeViz/nodes/volumeraypickintersection.h
  VolumeViz/nodes/SoOrthoSlice.h
  VolumeViz/nodes/SoVolumeData.h
  VolumeViz/nodes/SoVolumeRendering.h
)

set(INST_MISC_HDRS
  VolumeViz/misc/CvrGlobalRenderLock.h
  VolumeViz/misc/CvrResourceManager.h
  VolumeViz/misc/CvrCentralDifferenceGradient.h
  VolumeViz/misc/Gradient.h
  VolumeViz/misc/CvrVoxelChunk.h
  VolumeViz/misc/CvrUtil.h
  VolumeViz/misc/CvrCLUT.h
  VolumeViz/misc/CvrGradient.h
  VolumeViz/misc/CvrGIMPGradient.h
)

set(INST_DETAILS_HDRS
  VolumeViz/details/SoVolumeDetail.h
  VolumeViz/details/SoVolumeSkinDetail.h
  VolumeViz/details/SoVolumeRenderDetail.h
  VolumeViz/details/SoOrthoSliceDetail.h
  VolumeViz/details/SoObliqueSliceDetail.h
)

set(INST_RENDER_2D_HDRS
  VolumeViz/render/2D/Cvr2DTexPage.h
  VolumeViz/render/2D/CvrPageHandler.h
  VolumeViz/render/2D/texmemfullimg.h
  VolumeViz/render/2D/Cvr2DTexSubPage.h
)

set(INST_RENDER_POINTSET_HDRS
  VolumeViz/render/Pointset/PointRendering.h
)

set(INST_RENDER_COMMON_HDRS
  VolumeViz/render/common/CvrRGBATexture.h
  VolumeViz/render/common/Cvr3DPaletteTexture.h
  VolumeViz/render/common/Cvr2DPaletteTexture.h
  VolumeViz/render/common/CvrPaletteTexture.h
  VolumeViz/render/common/Cvr2DRGBATexture.h
  VolumeViz/render/common/CvrTextureObject.h
  VolumeViz/render/common/Cvr3DRGBATexture.h
  VolumeViz/render/common/Cvr3DPaletteGradientTexture.h
)

set(INST_RENDER_3D_HDRS
  VolumeViz/render/3D/CvrCubeHandler.h
  VolumeViz/render/3D/Cvr3DTexSubCube.h
  VolumeViz/render/3D/Cvr3DTexCube.h
)

set(INST_READERS_HDRS
  VolumeViz/readers/SoVRVolFileReader.h
  VolumeViz/readers/SoVRMemReader.h
  VolumeViz/readers/SoVolumeReader.h
)

set(INST_CACHES_HDRS
  VolumeViz/caches/CvrGLTextureCache.h
)

install(FILES ${INST_C_HDRS}               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/C               COMPONENT development)
install(FILES ${INST_ELEMENTS_HDRS}        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/elements        COMPONENT development)
install(FILES ${INST_NODES_HDRS}           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/nodes           COMPONENT development)
install(FILES ${INST_MISC_HDRS}            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/misc            COMPONENT development)
install(FILES ${INST_DETAILS_HDRS}         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/details         COMPONENT development)
install(FILES ${INST_RENDER_2D_HDRS}       DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/render/2D       COMPONENT development)
install(FILES ${INST_RENDER_POINTSET_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/render/Pointset COMPONENT development)
install(FILES ${INST_RENDER_COMMON_HDRS}   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/render/common   COMPONENT development)
install(FILES ${INST_RENDER_3D_HDRS}       DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/render/3D       COMPONENT development)
install(FILES ${INST_READERS_HDRS}         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/readers         COMPONENT development)
install(FILES ${INST_CACHES_HDRS}          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VolumeViz/caches          COMPONENT development)

# ############################################################################
# Install targets
# ############################################################################

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime NAMELINK_SKIP
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
)

if(SIMVOLEON_BUILD_SHARED_LIBS)
  install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development NAMELINK_ONLY)
  if(MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} CONFIGURATIONS Debug RelWithDebInfo COMPONENT debug)
  endif()
endif()

# ############################################################################
# Install pkgconfig file and CMake config package
# ############################################################################

if(UNIX OR MINGW)
  configure_file("${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pc.cmake.in" ${PROJECT_NAME}.pc @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

configure_package_config_file("${CMAKE_SOURCE_DIR}/${PROJECT_NAME_LOWER}-config.cmake.in" ${PROJECT_NAME_LOWER}-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY ExactVersion
)

# To make the component usable not only from the install directory but also from the build directory
export(
  TARGETS ${PROJECT_NAME}
  FILE ${PROJECT_NAME_LOWER}-export.cmake
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
  COMPONENT development
)

install(EXPORT ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}::
  FILE ${PROJECT_NAME_LOWER}-export.cmake
  COMPONENT development
)
