

FIND_PACKAGE( OpenCV )

IF(OpenCV_FOUND)
  MESSAGE(STATUS "OpenCV Found.")
  MESSAGE(STATUS "OpenCV dir: ${OpenCV_DIR}")
  MESSAGE(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
  MESSAGE(STATUS "OpenCV libs: ${OpenCV_LIBS}")
  INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
  SET(src opencv.c)
  SET(luasrc init.lua)
  ADD_TORCH_PACKAGE(camopencv "${src}" "${luasrc}" "Image Processing")
  TARGET_LINK_LIBRARIES(camopencv luaT TH ${OpenCV_LIBS})
ENDIF(OpenCV_FOUND)
