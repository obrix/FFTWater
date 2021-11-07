cmake_minimum_required(VERSION 3.2)
project (FFTWater)

set(FFTWater_MAJOR_VERSION 0)
set(FFTWater_MINOR_VERSION 1)
set(FFTWater_PATCH_VERSION 0)
set(FFTWater_VERSION
  ${FFTWater_MAJOR_VERSION}.${FFTWater_MINOR_VERSION}.${FFTWater_PATCH_VERSION})


# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/FFTWater)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
set(var INSTALL_${p}_DIR)
if(NOT IS_ABSOLUTE "${${var}}")
  set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
endif()
endforeach()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

option(BUILD_GUI "Build the GUI." OFF)

set(fftwater_includes ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/OceanSurface) 

if(NOT WIN32)
  find_package(FFTW3)
  list(APPEND fftwater_includes ${FFTW3_INCLUDE_DIR})
else()
  find_package(FFTW3 CONFIG REQUIRED)
endif()


if(BUILD_GUI)
  find_package(GLEW)
  find_package(OpenGL REQUIRED)
  find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  list(APPEND fftwater_includes ${CMAKE_SOURCE_DIR}/src ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${CMAKE_BINARY_DIR})  
endif(BUILD_GUI)

include_directories(${fftwater_includes})

# Ocean Water library
add_library(OceanSurface SHARED
  ${CMAKE_SOURCE_DIR}/OceanSurface/Vec2f.cpp
  ${CMAKE_SOURCE_DIR}/OceanSurface/OceanSurface.cpp
)

set_target_properties(OceanSurface PROPERTIES
  PUBLIC_HEADER "OceanSurface/Vec2f.h;OceanSurface/OceanSurface.h;OceanSurface/OceanSurfaceEXPORT.h")

if(NOT WIN32)
  target_link_libraries(OceanSurface ${FFTW3_LIBRARY})
else()
  target_link_libraries(OceanSurface PRIVATE FFTW3::fftw3)
endif()

install(TARGETS OceanSurface
  EXPORT FFTWaterTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/OceanSurface"
    COMPONENT dev)

#Package fftw3.dll with OceanSurface.dll 
get_target_property(FFTW3_lib FFTW3::fftw3 IMPORTED_LOCATION_DEBUG)
install(FILES ${FFTW3_lib} DESTINATION bin)

if(BUILD_GUI) 
  add_executable(FFTWater
    ${CMAKE_SOURCE_DIR}/src/DialogParameters.ui
    ${CMAKE_SOURCE_DIR}/src/waterfftvisu.ui
    ${CMAKE_SOURCE_DIR}/src/waterfftvisu.qrc
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/functions.cpp
    ${CMAKE_SOURCE_DIR}/src/camera.cpp
    ${CMAKE_SOURCE_DIR}/src/waterfftvisu.cpp)

  target_link_libraries(FFTWater
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5OpenGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    OceanSurface)

  file(COPY ${CMAKE_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR})
endif(BUILD_GUI)

# Add all targets to the build-tree export set
export(TARGETS OceanSurface
  FILE "${PROJECT_BINARY_DIR}/FFTWaterTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE FFTWater)

# Create the FooBarConfig.cmake and FooBarConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")

# ... for the install tree
set(CONF_INCLUDE_DIRS "\${FFTWater_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(FFTWaterConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FFTWaterConfig.cmake" @ONLY)

configure_file(FFTWaterConfigVersion.cmake.in
"${PROJECT_BINARY_DIR}/FFTWaterConfigVersion.cmake" @ONLY)


# Install the FFTWaterConfig.cmake and FFTWaterConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FFTWaterConfig.cmake"
  "${PROJECT_BINARY_DIR}/FFTWaterConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT FFTWaterTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)


