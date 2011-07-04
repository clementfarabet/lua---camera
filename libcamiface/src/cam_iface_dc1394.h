/* internal structures for dc1394 implementation */

#ifdef MEGA_BACKEND
  #define BACKEND_METHOD(m) dc1394_##m
#else
  #define BACKEND_METHOD(m) m
#endif

#include "cam_iface_static_functions.h"
