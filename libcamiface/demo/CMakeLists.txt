# Copyright (c) 2004-2009, California Institute of Technology. All
# rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:

#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.

#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# demo programs with no compile/link needs beyond libcamiface

set(SRCS
    #multi-cam-autoset
    #multi-cam
    #simple
    #save_fmf
    frame_grabber
    )

IF(UNIX AND NOT APPLE)
  set(USR_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/libcamiface/")
  SET(SHADER_SOURCE_DIR ${USR_SHARE_DIR})
ELSE(UNIX AND NOT APPLE)
  # current directory
  message(STATUS "unknown shader source directory: setting to current directory")
  SET(SHADER_SOURCE_DIR "")
ENDIF(UNIX AND NOT APPLE)

set(all_demo_backends ${all_backends})

FOREACH (SRC ${SRCS})
  FOREACH (BACKEND ${all_demo_backends})

    ADD_EXECUTABLE (${SRC}-${BACKEND} ${SRC}.c)
    TARGET_LINK_LIBRARIES (${SRC}-${BACKEND} cam_iface_${BACKEND})

    INSTALL (TARGETS ${SRC}-${BACKEND}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )
  ENDFOREACH (BACKEND)
ENDFOREACH (SRC ${SRCS})
