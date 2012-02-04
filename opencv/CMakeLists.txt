

FIND_PACKAGE( OpenCV REQUIRED )

SET(src opencv.c)
SET(luasrc init.lua)
ADD_TORCH_PACKAGE(camopencv "${src}" "${luasrc}" "Image Processing")
TARGET_LINK_LIBRARIES(camopencv luaT TH ${OpenCV_LIBS})
