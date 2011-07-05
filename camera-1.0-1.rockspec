
package = "camera"
version = "1.0-1"

source = {
   url = "camera-1.0-1.tgz"
}

description = {
   summary = "A camera interface for Torch7.",
   detailed = [[
         A simple camera interface for Torch7. 
         Extends the 'image' package by creating 
         an image.Camera() class. Works on MacOSX
         and Linux.
   ]],
   homepage = "",
   license = "MIT/X11" -- or whatever you like
}

dependencies = {
   "lua >= 5.1",
   "torch",
   "xlua",
   "sys",
   "image"
}

build = {
   type = "cmake",

   cmake = [[
         cmake_minimum_required(VERSION 2.8)

         set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

         # infer path for Torch7
         string (REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" TORCH_PREFIX "${CMAKE_INSTALL_PREFIX}" )
         message (STATUS "Found Torch7, installed in: " ${TORCH_PREFIX})

         find_package (Torch REQUIRED)

         set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

         if (UNIX AND NOT APPLE)
             include_directories (${TORCH_INCLUDE_DIR})
             add_library (v4l SHARED video4linux/v4l.c)
             target_link_libraries (v4l ${TORCH_LIBRARIES})
             install_targets (/lib v4l)
             add_subdirectory (video4linux)
         endif (UNIX AND NOT APPLE)

         if (APPLE)
             include_directories (${TORCH_INCLUDE_DIR})
             add_library (camiface SHARED camiface/camiface.c)
             target_link_libraries (camiface ${TORCH_LIBRARIES})
             install_targets (/lib camiface)
             add_subdirectory (camiface)
         endif (APPLE)

         if (NOT UNIX)
             message (ERROR "This package only builds on Unix platforms")
         endif (NOT UNIX)

         install_files(/lua/camera init.lua)

         if (APPLE)
             # finally, build+install libcamiface, outside of Luarocks
             # this is a bit of a hack of course, but works ok
             string (REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" PREFIX "${CMAKE_INSTALL_PREFIX}" )
             message (STATUS "Installing libcamiface to: " ${PREFIX})
             execute_process(COMMAND mkdir -p scratch)
             execute_process(COMMAND cmake ../libcamiface -DCMAKE_INSTALL_PREFIX=${PREFIX} WORKING_DIRECTORY scratch)
             execute_process(COMMAND make install WORKING_DIRECTORY scratch)
         endif (APPLE)
   ]],

   variables = {
      CMAKE_INSTALL_PREFIX = "$(PREFIX)"
   }
}
