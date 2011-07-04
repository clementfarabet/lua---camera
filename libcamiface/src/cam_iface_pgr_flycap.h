/* internal structures for pgr_flycapture implementation */

#ifdef MEGA_BACKEND
  #define BACKEND_METHOD(m) pgr_flycapture_##m
#else
  #define BACKEND_METHOD(m) m
#endif

#include "cam_iface_static_functions.h"

