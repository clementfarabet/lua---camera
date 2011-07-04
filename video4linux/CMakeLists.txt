
IF (UNIX AND NOT APPLE)
   SET(src v4l.c) 
   SET(luasrc init.lua) 
   ADD_TORCH_PACKAGE(v4l "${src}" "${luasrc}")
   TARGET_LINK_LIBRARIES(v4l luaT TH)
ELSE (UNIX AND NOT APPLE)
   MESSAGE("WARNING: video4linux wrapper bypassed (only works on Linux)")
ENDIF (UNIX AND NOT APPLE)
