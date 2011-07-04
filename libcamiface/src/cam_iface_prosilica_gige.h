/* internal structures for prosilica_gige implementation */

#ifdef MEGA_BACKEND
  #define BACKEND_METHOD(m) prosilica_gige_##m
#else
  #define BACKEND_METHOD(m) m
#endif

#include "cam_iface_static_functions.h"
