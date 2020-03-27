cmake_minimum_required(VERSION 2.8.9)
project (FFTWater)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

find_package(FFTW3)
find_package(GLEW)
find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src
 ${CMAKE_SOURCE_DIR}/OceanSurface
 ${Qt5Core_INCLUDE_DIRS}
 ${Qt5Gui_INCLUDE_DIRS}
 ${Qt5Widgets_INCLUDE_DIRS}
 ${Qt5OpenGL_INCLUDE_DIRS}
 ${FFTW3_INCLUDE_DIR}
 ${GLEW_INCLUDE_DIR}
 ${CMAKE_BINARY_DIR})

add_executable(FFTWater
  ${CMAKE_SOURCE_DIR}/OceanSurface/Vec2f.cpp
  ${CMAKE_SOURCE_DIR}/OceanSurface/OceanSurface.cpp
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
  ${FFTW3_LIBRARY}
  ${GLEW_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY})

file(COPY ${CMAKE_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR})

message("Qt5 core libraries " ${Qt5Core_LIBRARIES})
message("Qt5 core libraries " ${Qt5Core_INCLUDE_DIRS})
message("Opengl lib " ${OPENGL_gl_LIBRARY})
message("Opengl glu lib " ${OPENGL_glu_LIBRARY})
message("QT5 Ui sources " ${SOURCES})
