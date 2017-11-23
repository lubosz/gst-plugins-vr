CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(gst-plugins-vr)

include (FindPkgConfig)

pkg_check_modules (GRAPHENE REQUIRED graphene-1.0)
pkg_check_modules (GST REQUIRED gstreamer-1.0)
pkg_check_modules (GST_GL REQUIRED gstreamer-gl-1.0)
pkg_check_modules (GST_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules (GLIB REQUIRED glib-2.0)
pkg_check_modules (GIO REQUIRED gio-2.0)
pkg_check_modules (OPENHMD REQUIRED openhmd)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

add_definitions(-DVERSION="0.1.0")
add_definitions(-DPACKAGE="gst-plugins-vr")
add_definitions(-DPACKAGE_NAME="GStreamer Plugins VR")
add_definitions(-DGST_PACKAGE_ORIGIN="c")

include_directories(
  ${GRAPHENE_INCLUDE_DIRS}
  ${GST_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GST_GL_INCLUDE_DIRS}
  ${GST_VIDEO_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/gst-libs/
  ${CMAKE_SOURCE_DIR}/build
)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/gpu)

execute_process(
	WORKING_DIRECTORY ${SHADER_DIR}
	COMMAND glib-compile-resources --target=shaders.c --sourcedir=. --generate-source shaders.gresource.xml)

file(GLOB_RECURSE GST_3D_C ${CMAKE_SOURCE_DIR}/gst-libs/gst/3d/*.c)
file(GLOB_RECURSE GST_3D_H ${CMAKE_SOURCE_DIR}/gst-libs/gst/3d/*.h)

add_library(gst3d-1.0 SHARED ${SOURCES} ${GST_3D_C} ${GST_3D_H})
add_library(gst3d-1.0_static STATIC ${SOURCES} ${GST_3D_C} ${GST_3D_H})

target_link_libraries(gst3d-1.0
  ${GRAPHENE_LIBRARIES}
  ${GST_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GST_GL_LIBRARIES}
  ${GST_VIDEO_LIBRARIES}
  ${GIO_LIBRARIES}
  ${OPENHMD_LIBRARIES}
)

file(GLOB_RECURSE GST_VR_C ${CMAKE_SOURCE_DIR}/gst/vr/*.c)
file(GLOB_RECURSE GST_VR_H ${CMAKE_SOURCE_DIR}/gst/vr/*.h)
add_library(gstvrplugins SHARED ${GST_VR_C} ${GST_VR_H} ${CMAKE_SOURCE_DIR}/gpu/shaders.c)
add_library(gstvrplugins_static STATIC ${GST_VR_C} ${GST_VR_H} ${CMAKE_SOURCE_DIR}/gpu/shaders.c)
target_link_libraries(gstvrplugins gst3d-1.0)


install (TARGETS gstvrplugins gstvrplugins_static
  LIBRARY DESTINATION lib/gstreamer-1.0
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib/gstreamer-1.0)
install (TARGETS gst3d-1.0 gst3d-1.0_static
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib)
