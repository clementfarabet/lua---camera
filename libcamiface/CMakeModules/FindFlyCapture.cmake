# Get default values for FLYCAPTURE_ROOT and FLYCAPTURE_LAYOUT

IF(WIN32)
  IF(EXISTS "C:\\Program Files\\Point Grey Research\\FlyCapture2")
    SET(FLYCAPTURE_ROOT "C:\\Program Files\\Point Grey Research\\FlyCapture2")
  ELSE(EXISTS "C:\\Program Files\\Point Grey Research\\FlyCapture2")
    IF(EXISTS "C:\\Program Files (x86)\\Point Grey Research\\FlyCapture2")
      # Maybe we're on 64bit but have 32bit SDK installed.
      SET(FLYCAPTURE_ROOT "C:\\Program Files (x86)\\Point Grey Research\\FlyCapture2")
    ELSE(EXISTS "C:\\Program Files (x86)\\Point Grey Research\\FlyCapture2")
      # If not, default to "C:\Program Files".
      SET(FLYCAPTURE_ROOT "C:\\Program Files\\Point Grey Research\\FlyCapture2")
    ENDIF(EXISTS "C:\\Program Files (x86)\\Point Grey Research\\FlyCapture2")
  ENDIF(EXISTS "C:\\Program Files\\Point Grey Research\\FlyCapture2")
ELSE(WIN32)
  # Assume headers and libraries are installed in FHS locations
  SET(FLYCAPTURE_ROOT "/usr")
ENDIF(WIN32)

# defaults for FLYCAPTURE_LAYOUT
IF(WIN32)
  SET(FLYCAPTURE_LAYOUT "flycapture")
ELSE(WIN32)
  # Assume headers and libraries are installed in FHS locations
  SET(FLYCAPTURE_LAYOUT "FHS")
ENDIF(WIN32)

#############################################
# Environment variable overrides

SET(FLYCAPTURE_TEST_INCLUDE_PATHS "$ENV{FLYCAPTURE_TEST_INCLUDE_PATHS}")
IF(FLYCAPTURE_TEST_INCLUDE_PATHS STREQUAL "")
  # not set with env var, use defaults
  SET(FLYCAPTURE_TEST_INCLUDE_PATHS ${FLYCAPTURE_ROOT}/include)
ENDIF(FLYCAPTURE_TEST_INCLUDE_PATHS STREQUAL "")

SET(FLYCAPTURE_TEST_LIB_PATHS "$ENV{FLYCAPTURE_TEST_LIB_PATHS}")
IF(FLYCAPTURE_TEST_LIB_PATHS STREQUAL "")
  # not set with env var, use defaults
  IF(FLYCAPTURE_LAYOUT STREQUAL "flycapture")
    IF(EXISTS ${FLYCAPTURE_ROOT}/lib64)
      SET(FLYCAPTURE_TEST_LIB_PATHS ${FLYCAPTURE_ROOT}/lib64)
      MESSAGE("building FlyCapture2 64-bit backend")
    ELSE(EXISTS ${FLYCAPTURE_ROOT}/lib64)
      SET(FLYCAPTURE_TEST_LIB_PATHS ${FLYCAPTURE_ROOT}/lib)
      MESSAGE("building FlyCapture2 32-bit backend")
    ENDIF(EXISTS ${FLYCAPTURE_ROOT}/lib64)
  ELSE(FLYCAPTURE_LAYOUT STREQUAL "flycapture")
    IF(NOT FLYCAPTURE_LAYOUT STREQUAL "FHS")
      MESSAGE(FATAL_ERROR "unknown FLYCAPTURE_LAYOUT " ${FLYCAPTURE_LAYOUT})
    ENDIF(NOT FLYCAPTURE_LAYOUT STREQUAL "FHS")
    SET(FLYCAPTURE_TEST_LIB_PATHS ${FLYCAPTURE_ROOT}/lib)
  ENDIF(FLYCAPTURE_LAYOUT STREQUAL "flycapture")
ENDIF(FLYCAPTURE_TEST_LIB_PATHS STREQUAL "")

#############################################
# No more environment variable settings

IF(NOT "$ENV{FLYCAPTURE_CMAKE_DEBUG}" STREQUAL "")
  MESSAGE("FLYCAPTURE_TEST_INCLUDE_PATHS " ${FLYCAPTURE_TEST_INCLUDE_PATHS})
  MESSAGE("FLYCAPTURE_TEST_LIB_PATHS " ${FLYCAPTURE_TEST_LIB_PATHS})
ENDIF(NOT "$ENV{FLYCAPTURE_CMAKE_DEBUG}" STREQUAL "")

FIND_PATH(FLYCAPTURE_INCLUDE_PATH FlyCapture2.h
  ${FLYCAPTURE_TEST_INCLUDE_PATHS}
)

FIND_LIBRARY(FLYCAPTURE_LIBRARY FlyCapture2 ${FLYCAPTURE_TEST_LIB_PATHS})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FLYCAPTURE DEFAULT_MSG
                                             FLYCAPTURE_INCLUDE_PATH
                                             FLYCAPTURE_LIBRARY)
SET(FLYCAPTURE_LIBRARIES ${FLYCAPTURE_LIBRARY})
SET(FLYCAPTURE_INCLUDE_DIRS ${FLYCAPTURE_INCLUDE_PATH})
