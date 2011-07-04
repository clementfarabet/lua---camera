FIND_PATH(DC1394_INCLUDE_DIR dc1394/control.h
  /opt/local/include
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(DC1394_LIBRARY dc1394 /opt/local/lib /usr/local/lib /usr/lib)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DC1394 DEFAULT_MSG DC1394_INCLUDE_DIR DC1394_LIBRARY)
SET(DC1394_LIBRARIES ${DC1394_LIBRARY})

IF(APPLE)
  FIND_PACKAGE(IOKit)
  FIND_PACKAGE(CoreFoundation)
  FIND_PACKAGE(CoreServices)

  add_library(dc1394 STATIC IMPORTED)
  set_property(TARGET dc1394 PROPERTY IMPORTED_LOCATION /opt/local/lib/libdc1394.a /usr/local/lib/libdc1394.a)

  add_library(libusb STATIC IMPORTED)
  set_property(TARGET libusb PROPERTY IMPORTED_LOCATION /usr/local/lib/libusb-1.0.a)

  SET(DC1394_LIBRARIES 
        dc1394
        libusb
        ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY} ${CORESERVICES_LIBRARY})


ENDIF(APPLE)
