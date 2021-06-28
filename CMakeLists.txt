cmake_minimum_required(VERSION 3.2)
project (FFTWater)

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
  list(APPEND fftwater_includes ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
endif()


if(BUILD_GUI)
  find_package(GLEW)
  find_package(OpenGL REQUIRED)
  find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  list(APPEND fftwater_includes ${CMAKE_SOURCE_DIR}/src ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${CMAKE_BINARY_DIR})  
endif(BUILD_GUI)

include_directories(${fftwater_includes})

add_library(OceanSurface SHARED
  ${CMAKE_SOURCE_DIR}/OceanSurface/Vec2f.cpp
  ${CMAKE_SOURCE_DIR}/OceanSurface/OceanSurface.cpp
)

if(NOT WIN32)
  target_link_libraries(OceanSurface ${FFTW3_LIBRARY})
else()
  target_link_libraries(OceanSurface PRIVATE FFTW3::fftw3)
endif()



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

