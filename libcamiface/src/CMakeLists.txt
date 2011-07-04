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

set(common_SRCS
    cam_iface_common.c
    )

set(CAM_IFACE_VERSION "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(CAM_IFACE_SOVERSION 0)

# dc1394 backend ------------------

IF(DC1394_FOUND)

  set(dc1394_SRCS ${common_SRCS}
      cam_iface_dc1394.c
      )
  set(mega_SRCS ${mega_SRCS}
      cam_iface_dc1394.c
     )

  include_directories(${DC1394_INCLUDE_DIR})
  LINK_DIRECTORIES (${DC1394_LIBRARY})

  ADD_LIBRARY(cam_iface_dc1394 SHARED ${dc1394_SRCS})
  TARGET_LINK_LIBRARIES(cam_iface_dc1394 ${DC1394_LIBRARIES})
  set_target_properties(cam_iface_dc1394 PROPERTIES VERSION ${CAM_IFACE_VERSION}
    SOVERSION ${CAM_IFACE_SOVERSION})

  ADD_LIBRARY(cam_iface_dc1394-static STATIC ${dc1394_SRCS})
  set_target_properties(cam_iface_dc1394-static PROPERTIES OUTPUT_NAME "cam_iface_dc1394")

  SET(mega_LINK_LIBS ${mega_LINK_LIBS} ${DC1394_LIBRARIES})
  SET(mega_DEFINE
      ${mega_DEFINE}
      -DMEGA_BACKEND_DC1394
      )
  SET_TARGET_PROPERTIES(cam_iface_dc1394 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  SET_TARGET_PROPERTIES(cam_iface_dc1394-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

  INSTALL(TARGETS cam_iface_dc1394 cam_iface_dc1394-static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

ENDIF(DC1394_FOUND)

# prosilica_gige backend ------------------

IF(PROSILICA_GIGE_FOUND)

  set(prosilica_gige_SRCS ${common_SRCS}
      cam_iface_prosilica_gige.cpp
      )
  set(mega_SRCS ${mega_SRCS}
      cam_iface_prosilica_gige.cpp
     )

  # list of each command-line define
  IF(WIN32)
    set(prosilica_gige_DEFINE
        -D_WINDOWS
        )
  ELSE(WIN32)
    IF(UNIX AND NOT APPLE)
      # Linux
      set(prosilica_gige_DEFINE
          -D_LINUX
          )
      IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )
        set(prosilica_gige_DEFINE -D_x64 ${prosilica_gige_DEFINE})
      ELSE("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )
        set(prosilica_gige_DEFINE -D_x86 ${prosilica_gige_DEFINE})
      ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )
    ELSE(UNIX AND NOT APPLE)
      set(prosilica_gige_DEFINE -D_OSX ${prosilica_gige_DEFINE})
      IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386" )
        set(prosilica_gige_DEFINE -D_x86 ${prosilica_gige_DEFINE})
      ELSE("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386" )
        MESSAGE(FATAL_ERROR "Prosilica GigE not supported on this platform")
      ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386" )
    ENDIF(UNIX AND NOT APPLE)
  ENDIF(WIN32)

  # concatenate (join) strings to make single argument
  FOREACH(element ${prosilica_gige_DEFINE})
    set(prosilica_gige_DEFINES "${prosilica_gige_DEFINES} ${element}")
  ENDFOREACH(element)

  SET_SOURCE_FILES_PROPERTIES(
      cam_iface_prosilica_gige.cpp
      PROPERTIES COMPILE_FLAGS ${prosilica_gige_DEFINES})

  include_directories(${PROSILICA_GIGE_INCLUDE_DIRS})
  ADD_LIBRARY(cam_iface_prosilica_gige SHARED ${prosilica_gige_SRCS})

  TARGET_LINK_LIBRARIES(cam_iface_prosilica_gige ${PROSILICA_GIGE_LIBRARIES})
  set_target_properties(cam_iface_prosilica_gige PROPERTIES
    VERSION ${CAM_IFACE_VERSION}
    SOVERSION ${CAM_IFACE_SOVERSION}
    DEFINE_SYMBOL cam_iface_EXPORTS
    )

  SET_TARGET_PROPERTIES(cam_iface_prosilica_gige PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
  )

  SET(CI_PROSIL_TARGETS cam_iface_prosilica_gige)

  if(NOT WIN32)
  # Trouble building static lib on Windows, so disabled for now...
  ADD_LIBRARY(cam_iface_prosilica_gige-static STATIC ${prosilica_gige_SRCS})
  set_target_properties(cam_iface_prosilica_gige-static PROPERTIES
    OUTPUT_NAME "cam_iface_prosilica_gige"
    )

  SET_TARGET_PROPERTIES(cam_iface_prosilica_gige-static PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
  )
  SET(CI_PROSIL_TARGETS ${CI_PROSIL_TARGETS} cam_iface_prosilica_gige-static)
  endif(NOT WIN32)

  SET(mega_LINK_LIBS ${mega_LINK_LIBS} ${PROSILICA_GIGE_LIBRARIES})
  SET(mega_DEFINE
      ${mega_DEFINE}
      -DMEGA_BACKEND_PROSILICA_GIGE
      )
  INSTALL(TARGETS ${CI_PROSIL_TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  IF(WIN32)
    # install PvAPI.dll on Windows
    GET_FILENAME_COMPONENT(PROSIL_LIB_DIR ${PROSILICA_GIGE_LIBRARIES} PATH)
    SET(PROSIL_DLL_DIR ${PROSIL_LIB_DIR}/../bin-pc)
    get_filename_component(PROSIL_DLL_DIR ${PROSIL_DLL_DIR} ABSOLUTE)
    FILE(GLOB PROSIL_DLLS ${PROSIL_DLL_DIR}/*.dll)
    INSTALL(FILES ${PROSIL_DLLS}
      DESTINATION bin
      )
  ENDIF(WIN32)

ENDIF(PROSILICA_GIGE_FOUND)

# flycapture backend ------------------

IF(FLYCAPTURE_FOUND)

  set(flycapture_SRCS ${common_SRCS}
      cam_iface_pgr_flycap.cpp
      )
  set(mega_SRCS ${mega_SRCS}
      cam_iface_pgr_flycap.cpp
     )

  include_directories(${FLYCAPTURE_INCLUDE_DIRS})
  ADD_LIBRARY(cam_iface_pgr_flycap SHARED ${flycapture_SRCS})

  TARGET_LINK_LIBRARIES(cam_iface_pgr_flycap ${FLYCAPTURE_LIBRARIES})
  set_target_properties(cam_iface_pgr_flycap PROPERTIES
    VERSION ${CAM_IFACE_VERSION}
    SOVERSION ${CAM_IFACE_SOVERSION}
    DEFINE_SYMBOL cam_iface_EXPORTS
    )

  if(WIN32)
    set_target_properties(cam_iface_pgr_flycap PROPERTIES
      COMPILE_FLAGS -D_CRT_SECURE_NO_WARNINGS=1
    )
  endif(WIN32)

  SET_TARGET_PROPERTIES(cam_iface_pgr_flycap PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
  )

  SET(CI_FLYCAP_TARGETS cam_iface_pgr_flycap)

  ## if(NOT WIN32)
  ## # Trouble building static lib on Windows, so disabled for now...
  ## ADD_LIBRARY(cam_iface_pgr_flycap-static STATIC ${flycapture_SRCS})
  ## set_target_properties(cam_iface_pgr_flycap-static PROPERTIES
  ##   OUTPUT_NAME "cam_iface_pgr_flycap"
  ##   )

  ## SET_TARGET_PROPERTIES(cam_iface_pgr_flycap-static PROPERTIES
  ##   CLEAN_DIRECT_OUTPUT 1
  ## )
  ## SET(CI_FLYCAP_TARGETS ${CI_FLYCAP_TARGETS} cam_iface_pgr_flycap-static)
  ## endif(NOT WIN32)

  SET(mega_LINK_LIBS ${mega_LINK_LIBS} ${FLYCAPTURE_LIBRARIES})
  SET(mega_DEFINE
      ${mega_DEFINE}
      -DMEGA_BACKEND_FLYCAPTURE
      )
  INSTALL(TARGETS ${CI_FLYCAP_TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  ## IF(WIN32)
  ##   # install PvAPI.dll on Windows
  ##   GET_FILENAME_COMPONENT(PROSIL_LIB_DIR ${FLYCAPTURE_LIBRARIES} PATH)
  ##   SET(PROSIL_DLL_DIR ${PROSIL_LIB_DIR}/../bin-pc)
  ##   get_filename_component(PROSIL_DLL_DIR ${PROSIL_DLL_DIR} ABSOLUTE)
  ##   FILE(GLOB PROSIL_DLLS ${PROSIL_DLL_DIR}/*.dll)
  ##   INSTALL(FILES ${PROSIL_DLLS}
  ##     DESTINATION bin
  ##     )
  ## ENDIF(WIN32)

ENDIF(FLYCAPTURE_FOUND)

# quicktime backend ------------------

IF(QUICKTIME_FOUND)

IF(NOT CARBON_FOUND)
  MESSAGE(FATAL_ERROR "QuickTime backend depends on Carbon, but Carbon not found")
ENDIF(NOT CARBON_FOUND)

  set(quicktime_SRCS ${common_SRCS}
      cam_iface_quicktime.c
      )
  set(mega_SRCS ${mega_SRCS}
      cam_iface_quicktime.c
     )

  ADD_LIBRARY(cam_iface_quicktime SHARED ${quicktime_SRCS})
  TARGET_LINK_LIBRARIES(cam_iface_quicktime ${QUICKTIME_LIBRARY} ${CARBON_LIBRARY})
  SET(mega_LINK_LIBS ${mega_LINK_LIBS} ${QUICKTIME_LIBRARY} ${CARBON_LIBRARY})
  SET(mega_DEFINE
      ${mega_DEFINE}
      -DMEGA_BACKEND_QUICKTIME
      )
  set_target_properties(cam_iface_quicktime PROPERTIES VERSION ${CAM_IFACE_VERSION}
    SOVERSION ${CAM_IFACE_SOVERSION})

  ADD_LIBRARY(cam_iface_quicktime-static STATIC ${quicktime_SRCS})
  set_target_properties(cam_iface_quicktime-static PROPERTIES OUTPUT_NAME "cam_iface_quicktime")

  SET_TARGET_PROPERTIES(cam_iface_quicktime PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  SET_TARGET_PROPERTIES(cam_iface_quicktime-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

  INSTALL(TARGETS cam_iface_quicktime cam_iface_quicktime-static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

ENDIF(QUICKTIME_FOUND)




# mega backend -----------------------------


# get list of all backends to wrap into mega backend
FOREACH(BACKEND ${all_backends})
    IF(${BACKEND} STREQUAL "mega")
      # don't include mega backend in list of backends for mega
    ELSE(${BACKEND} STREQUAL "mega")
        set(INDIVIDUAL_MEGA_BACKENDS ${INDIVIDUAL_MEGA_BACKENDS} "static${BACKEND}")
    ENDIF(${BACKEND} STREQUAL "mega")
ENDFOREACH(BACKEND)

FOREACH(BACKEND ${INDIVIDUAL_MEGA_BACKENDS})
  IF(DEFINED MEGA_BACKENDS)
    set(MEGA_BACKENDS "${MEGA_BACKENDS},\"${BACKEND}\"")
  ELSE(DEFINED MEGA_BACKENDS)
    set(MEGA_BACKENDS "\"${BACKEND}\"")
  ENDIF(DEFINED MEGA_BACKENDS)
ENDFOREACH(BACKEND)


list(LENGTH INDIVIDUAL_MEGA_BACKENDS NUM_MEGA_BACKENDS)
MESSAGE(STATUS "mega backend includes ${NUM_MEGA_BACKENDS} other backends: (${MEGA_BACKENDS})")


# list of each command-line define
set(mega_DEFINE
    ${mega_DEFINE}
    -DMEGA_BACKEND
   )

FILE(WRITE "mega_backend_info.h"
     "#define NUM_BACKENDS ${NUM_MEGA_BACKENDS}\n"
     "char *backend_names[NUM_BACKENDS] = {${MEGA_BACKENDS}};\n")

  if(WIN32)
    set(mega_DEFINE
      ${mega_DEFINE}
      -D_CRT_SECURE_NO_WARNINGS=1
    )
  endif(WIN32)

# concatenate (join) strings to make single argument
FOREACH(element ${mega_DEFINE})
  set(mega_DEFINES "${mega_DEFINES} ${element}")
ENDFOREACH(element ${mega_DEFINE})

SET_SOURCE_FILES_PROPERTIES(
    cam_iface_mega.c
    PROPERTIES COMPILE_FLAGS ${mega_DEFINES})
MESSAGE(STATUS "mega DEFINES: " ${mega_DEFINES})

set(mega_SRCS ${mega_SRCS} ${common_SRCS}
    cam_iface_mega.c
    )

ADD_LIBRARY(cam_iface_mega SHARED ${mega_SRCS})
ADD_LIBRARY(cam_iface_mega-static STATIC ${mega_SRCS})
set_target_properties(cam_iface_mega-static PROPERTIES OUTPUT_NAME "cam_iface_megastatic")

SET_TARGET_PROPERTIES(cam_iface_mega PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(cam_iface_mega-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

IF(WIN32)
  SET(MEGA_COMPILE_FLAGS "-Dcam_iface_EXPORTS -DMEGA_BACKEND")
ELSE(WIN32)
  SET(MEGA_COMPILE_FLAGS "-DMEGA_BACKEND")
ENDIF(WIN32)

SET_TARGET_PROPERTIES(cam_iface_mega PROPERTIES
  VERSION ${CAM_IFACE_VERSION}
  SOVERSION ${CAM_IFACE_SOVERSION}
  COMPILE_FLAGS ${MEGA_COMPILE_FLAGS}
  )
SET_TARGET_PROPERTIES(cam_iface_mega-static PROPERTIES
  VERSION ${CAM_IFACE_VERSION}
  SOVERSION ${CAM_IFACE_SOVERSION}
  COMPILE_FLAGS ${MEGA_COMPILE_FLAGS}
  )

# add the static libraries into the linker sources for the mega backend
TARGET_LINK_LIBRARIES(cam_iface_mega ${mega_LINK_LIBS})
TARGET_LINK_LIBRARIES(cam_iface_mega-static ${mega_LINK_LIBS})


INSTALL(TARGETS cam_iface_mega cam_iface_mega-static
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
