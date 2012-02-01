CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
FIND_PACKAGE(Torch REQUIRED)

if (UNIX AND NOT APPLE)
    add_subdirectory (video4linux)
endif (UNIX AND NOT APPLE)

if (APPLE)
    add_subdirectory (opencv)
    add_subdirectory (macos)
endif (APPLE)

if (NOT UNIX)
    message (ERROR "This package only builds on Unix platforms")
endif (NOT UNIX)

install_files(/lua/camera init.lua)

SET(src)
SET(luasrc init.lua)
ADD_TORCH_PACKAGE(camera "${src}" "${luasrc}" "Image Processing")
