

FIND_PACKAGE( OpenCV )

IF(OpenCV_FOUND)
  SET(src opencv.c)
  SET(luasrc init.lua)
  ADD_TORCH_PACKAGE(camopencv "${src}" "${luasrc}" "Image Processing")
  TARGET_LINK_LIBRARIES(camopencv luaT TH ${OpenCV_LIBS})
ENDIF(OpenCV_FOUND)
