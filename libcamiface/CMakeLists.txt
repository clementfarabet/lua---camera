# Copyright (c) 2004-2009, California Institute of Technology. All
# rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:

#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.

#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules )

##  global setup
if (APPLE)
   set (CMAKE_C_FLAGS "-m32")
   set (CMAKE_CXX_FLAGS "-m32")
endif (APPLE)

project(libcamiface)

# version
set(V_MAJOR 0)
set(V_MINOR 5)
set(V_PATCH 9)

include_directories( ${CMAKE_SOURCE_DIR}/include )

FIND_PACKAGE(PkgConfig)

IF(UNIX)
  SET(CMAKE_SKIP_BUILD_RPATH FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF(UNIX)

# Backend: mega will always be built -----
set(all_backends mega)
# Backend: libdc1394 ---------------------------------

# Method A: Check for libdc1394 using pkg-config (preferred)
#pkg_check_modules(DC1394 libdc1394-2)

# Method B: Check for libdc1394 ourselves without pkg-config (e.g. on Mac)

# IF(NOT DC1394_FOUND)
#   FIND_PACKAGE(DC1394)
#   MESSAGE(${DC1394_INCLUDE_DIR})
#   MESSAGE(${DC1394_LIBRARY})
# ENDIF(NOT DC1394_FOUND)


IF(DC1394_FOUND)
  set(all_backends ${all_backends} dc1394)
ENDIF(DC1394_FOUND)

# Backend: Prosilica --------------------------------

FIND_PACKAGE(ProsilicaGigE)

IF(PROSILICA_GIGE_FOUND)
  set(all_backends ${all_backends} prosilica_gige)
ENDIF(PROSILICA_GIGE_FOUND)

# Backend: PGR FlyCapture----------------------------

FIND_PACKAGE(FlyCapture)

IF(FLYCAPTURE_FOUND)
  set(all_backends ${all_backends} pgr_flycap)
ENDIF(FLYCAPTURE_FOUND)

# Backend: QuickTime --------------------------------

FIND_PACKAGE(QuickTime)
FIND_PACKAGE(Carbon)

IF(QUICKTIME_FOUND)
  set(all_backends ${all_backends} quicktime)
ENDIF(QUICKTIME_FOUND)

# Build targets -------------------------------------

# create config.h file
#configure_file(src/camiface_config.h.in.cmake ${CMAKE_BINARY_DIR}/src/camiface_config.h)

add_subdirectory(src)
add_subdirectory(demo)

# Install header ------------------------------------

INSTALL(FILES include/cam_iface.h
  DESTINATION include)

# Install linux udev file ---------------------------

IF(UNIX)
  IF(NOT APPLE)
    IF(EXISTS /etc/udev/rules.d)
      INSTALL(FILES 99-pt-grey-firefly-mv-usb.rules
        DESTINATION /etc/udev/rules.d)
    ENDIF(EXISTS /etc/udev/rules.d)
  ENDIF(NOT APPLE)
ENDIF(UNIX)

# Create pkg-config file

SET(PKG_CONFIG_VERSION "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir      ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir  ${CMAKE_INSTALL_PREFIX}/include)

configure_file(libcamiface.pc.in libcamiface.pc)
IF(UNIX)
  IF(NOT APPLE)
    set(PC_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/")
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/libcamiface.pc"
	    DESTINATION ${PC_DIR})
  ENDIF(NOT APPLE)
ENDIF(UNIX)

# CPack for binary and source releases --------------

# For binary releases

SET(CPACK_PACKAGE_NAME "libcamiface")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libcamiface - camera interface library")
SET(CPACK_PACKAGE_VENDOR "Motmot authors")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.rst")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")

SET(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${V_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "libcamiface ${V_MAJOR}.${V_MINOR}.${V_PATCH}")
SET(CPACK_PACKAGE_EXECUTABLES "frame_grabber")

SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/installer_readme.html")

# For source releases (except we use git archive to make these)

SET(CPACK_SOURCE_PACKAGE_FILE_NAME
    "libcamiface-${V_MAJOR}.${V_MINOR}.${V_PATCH}")
SET(CPACK_SOURCE_GENERATOR "TGZ")

INCLUDE(CPack)
