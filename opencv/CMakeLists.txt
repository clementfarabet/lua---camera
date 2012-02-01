

FIND_PACKAGE( OpenCV REQUIRED )

SET(src opencv.c)
SET(luasrc init.lua)
ADD_TORCH_PACKAGE(opencv "${src}" "${luasrc}" "Image Processing")
TARGET_LINK_LIBRARIES(opencv luaT TH ${OpenCV_LIBS})
