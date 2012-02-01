
SET(src v4l.c)
SET(luasrc init.lua)
ADD_TORCH_PACKAGE(v4l "${src}" "${luasrc}" "Image Processing")
TARGET_LINK_LIBRARIES(v4l luaT TH)
