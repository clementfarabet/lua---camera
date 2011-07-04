/*

Copyright (c) 2004-2009, California Institute of Technology. All
rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */
#include "cam_iface.h"
#include "cam_iface_internal.h"
#include <stdio.h>
#include <stdlib.h>

struct backend_info_t {
  char* name;
  int started;
  int cam_start_idx;
  int cam_stop_idx;
  int (*have_error)(void);
  void (*clear_error)(void);
  const char* (*get_error_string)(void);
  void (*startup)(void);
  void (*shutdown)(void);
  int (*get_num_cameras)(void);
  void (*get_num_modes)(int,int*);
  void (*get_camera_info)(int,Camwire_id*);
  void (*get_mode_string)(int,int,char*,int);
  cam_iface_constructor_func_t (*get_constructor_func)(int);
};

/* globals -- allocate space */
int mega_num_cameras = 0;

cam_iface_thread_local int cam_iface_error = 0;
#define CAM_IFACE_MAX_ERROR_LEN 255
cam_iface_thread_local char cam_iface_error_string[CAM_IFACE_MAX_ERROR_LEN]  = {0x00}; //...

#include "mega_backend_info.h"
struct backend_info_t backend_info[NUM_BACKENDS];
static int backends_started = 0;

#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,            \
           "%s (%d): %s\n",__FILE__,__LINE__,(m));

#define CHECK_CI_ERR()                                          \
  if (this_backend_info->have_error()) {                        \
    cam_iface_error = this_backend_info->have_error();          \
    cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,  \
             "mega backend error (%s %d): %s",__FILE__,        \
             __LINE__,this_backend_info->get_error_string());   \
    return;                                                     \
  }

#define CHECK_CI_ERRV()                                         \
if (this_backend_info->have_error()) {                          \
cam_iface_error = this_backend_info->have_error();              \
cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,  \
"mega backend error (%s %d): %s",__FILE__,                     \
__LINE__,this_backend_info->get_error_string());                \
return NULL;                                         \
}

const char* cam_iface_get_api_version() {
  return CAM_IFACE_API_VERSION;
}

int cam_iface_have_error() {
  int i;
  int err;
  struct backend_info_t* this_backend_info;

  if (cam_iface_error) {
    return cam_iface_error;
  }

  if (backends_started) {
    err = 0;
    for (i=0; i<NUM_BACKENDS; i++) {
      this_backend_info = &(backend_info[i]);
      if (this_backend_info->started) {
        err = this_backend_info->have_error();
        if (err) return err;
      }
    }
  }
  return 0;
}

void cam_iface_clear_error() {
  int i;
  struct backend_info_t* this_backend_info;

  cam_iface_error=0;

  if (backends_started) {
    for (i=0; i<NUM_BACKENDS; i++) {
      this_backend_info = &(backend_info[i]);
      if (this_backend_info->started) {
        this_backend_info->clear_error();
      }
    }
  }
}

const char * cam_iface_get_error_string() {
  struct backend_info_t* this_backend_info;
  int i,err;

  if (cam_iface_error) {
    return cam_iface_error_string;
  }

  if (backends_started) {
    err = 0;
    for (i=0; i<NUM_BACKENDS; i++) {
      this_backend_info = &(backend_info[i]);
      if (this_backend_info->started) {
        err = this_backend_info->have_error();
        if (err) return this_backend_info->get_error_string();
      }
    }
  }
  return "";
}

const char *cam_iface_get_driver_name(void) {
  return "mega";
}

int cam_iface_get_num_cameras(void) {
  return mega_num_cameras;
}

void cam_iface_startup(void) {
  struct backend_info_t* this_backend_info;
  int i, next_num_cameras;

  mega_num_cameras = 0;

  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    this_backend_info->name = backend_names[i];
    this_backend_info->started = 0;

    if (getenv("MEGA_BACKEND_DEBUG")!=NULL) {
      fprintf(stderr,"%s: %d: this_backend_info = %p\n",
              __FILE__,__LINE__,this_backend_info);
      fprintf(stderr,"%s: %d: this_backend_info->name = %s\n",
              __FILE__,__LINE__,this_backend_info->name);
      fprintf(stderr,"%s: %d: this_backend_info->started = %d\n",
              __FILE__,__LINE__,this_backend_info->started);
    }

    if (!strcmp(backend_names[i],"staticdc1394")) {
#ifdef MEGA_BACKEND_DC1394
#include "cam_iface_dc1394.h"
      this_backend_info->have_error = dc1394_cam_iface_have_error;
      this_backend_info->clear_error = dc1394_cam_iface_clear_error;
      this_backend_info->get_error_string = dc1394_cam_iface_get_error_string;
      this_backend_info->startup = dc1394_cam_iface_startup;
      this_backend_info->shutdown = dc1394_cam_iface_shutdown;
      this_backend_info->get_num_cameras = dc1394_cam_iface_get_num_cameras;
      this_backend_info->get_num_modes = dc1394_cam_iface_get_num_modes;
      this_backend_info->get_camera_info = dc1394_cam_iface_get_camera_info;
      this_backend_info->get_mode_string = dc1394_cam_iface_get_mode_string;
      this_backend_info->get_constructor_func = dc1394_cam_iface_get_constructor_func;
#else
      fprintf(stderr,"ERROR: don't know backend %s\n",backend_names[i]);
      exit(1);
#endif
    } else if (!strcmp(backend_names[i],"staticprosilica_gige")) {
#ifdef MEGA_BACKEND_PROSILICA_GIGE
#include "cam_iface_prosilica_gige.h"
      this_backend_info->have_error = prosilica_gige_cam_iface_have_error;
      this_backend_info->clear_error = prosilica_gige_cam_iface_clear_error;
      this_backend_info->get_error_string = prosilica_gige_cam_iface_get_error_string;
      this_backend_info->startup = prosilica_gige_cam_iface_startup;
      this_backend_info->shutdown = prosilica_gige_cam_iface_shutdown;
      this_backend_info->get_num_cameras = prosilica_gige_cam_iface_get_num_cameras;
      this_backend_info->get_num_modes = prosilica_gige_cam_iface_get_num_modes;
      this_backend_info->get_camera_info = prosilica_gige_cam_iface_get_camera_info;
      this_backend_info->get_mode_string = prosilica_gige_cam_iface_get_mode_string;
      this_backend_info->get_constructor_func = prosilica_gige_cam_iface_get_constructor_func;
#else
      fprintf(stderr,"ERROR: don't know backend %s\n",backend_names[i]);
      exit(1);
#endif
    } else if (!strcmp(backend_names[i],"staticpgr_flycap")) {
#ifdef MEGA_BACKEND_FLYCAPTURE
#include "cam_iface_pgr_flycap.h"
      this_backend_info->have_error = pgr_flycapture_cam_iface_have_error;
      this_backend_info->clear_error = pgr_flycapture_cam_iface_clear_error;
      this_backend_info->get_error_string = pgr_flycapture_cam_iface_get_error_string;
      this_backend_info->startup = pgr_flycapture_cam_iface_startup;
      this_backend_info->shutdown = pgr_flycapture_cam_iface_shutdown;
      this_backend_info->get_num_cameras = pgr_flycapture_cam_iface_get_num_cameras;
      this_backend_info->get_num_modes = pgr_flycapture_cam_iface_get_num_modes;
      this_backend_info->get_camera_info = pgr_flycapture_cam_iface_get_camera_info;
      this_backend_info->get_mode_string = pgr_flycapture_cam_iface_get_mode_string;
      this_backend_info->get_constructor_func = pgr_flycapture_cam_iface_get_constructor_func;
#else
      fprintf(stderr,"ERROR: don't know backend %s\n",backend_names[i]);
      exit(1);
#endif
    } else if (!strcmp(backend_names[i],"staticquicktime")) {
#ifdef MEGA_BACKEND_QUICKTIME
#include "cam_iface_quicktime.h"
      this_backend_info->have_error = quicktime_cam_iface_have_error;
      this_backend_info->clear_error = quicktime_cam_iface_clear_error;
      this_backend_info->get_error_string = quicktime_cam_iface_get_error_string;
      this_backend_info->startup = quicktime_cam_iface_startup;
      this_backend_info->shutdown = quicktime_cam_iface_shutdown;
      this_backend_info->get_num_cameras = quicktime_cam_iface_get_num_cameras;
      this_backend_info->get_num_modes = quicktime_cam_iface_get_num_modes;
      this_backend_info->get_camera_info = quicktime_cam_iface_get_camera_info;
      this_backend_info->get_mode_string = quicktime_cam_iface_get_mode_string;
      this_backend_info->get_constructor_func = quicktime_cam_iface_get_constructor_func;
#else
      fprintf(stderr,"ERROR: don't know backend %s\n",backend_names[i]);
      exit(1);
#endif
    } else {
      fprintf(stderr,"ERROR: don't know unidentified backend %s\n",backend_names[i]);
      exit(1);
    }

    if (getenv("MEGA_BACKEND_DEBUG")!=NULL) {
      fprintf(stderr,"%s: %d: this_backend_info = %p\n",
              __FILE__,__LINE__,this_backend_info);
      fprintf(stderr,"%s: %d: this_backend_info->name = %s\n",
              __FILE__,__LINE__,this_backend_info->name);
      fprintf(stderr,"%s: %d: this_backend_info->started = %d\n",
              __FILE__,__LINE__,this_backend_info->started);
      fprintf(stderr,"%s: %d: this_backend_info->have_error = %p\n",
              __FILE__,__LINE__,this_backend_info->have_error);
    }

    /* -------- XXXXXXXXX ------- */
    //    fprintf(stderr,"be:%s",(MEGA_BACKENDS)[i]);
    //exit(1);

    this_backend_info->clear_error();
    CHECK_CI_ERR();

    this_backend_info->startup();
    if (this_backend_info->have_error()) {
      if (getenv("MEGA_BACKEND_DEBUG")!=NULL) {
        fprintf(stderr,"%s: %d: %s backend startup() had error '%s'\n",
                __FILE__,__LINE__,
                this_backend_info->name,
                this_backend_info->get_error_string());
      }
      continue; //  backend startup had error
    }

    this_backend_info->started = 1;

    next_num_cameras = mega_num_cameras + this_backend_info->get_num_cameras();
    this_backend_info->cam_start_idx = mega_num_cameras;
    this_backend_info->cam_stop_idx = next_num_cameras;
    mega_num_cameras = next_num_cameras;
  }
  backends_started = 1;
}

void cam_iface_shutdown(void) {
  int i;
  struct backend_info_t* this_backend_info;
  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if (this_backend_info->started) {
      this_backend_info->shutdown();
      this_backend_info->started = 0;
    }
  }
}

#define CHECK_DEVICE_NUMBER(device_number)                              \
  if (((device_number) < 0) || ((device_number) >= mega_num_cameras))   \
    {                                                                   \
      cam_iface_error = -1;                                             \
      CAM_IFACE_ERROR_FORMAT("device number out of range");             \
      return;                                                           \
    }

#define CHECK_DEVICE_NUMBERV(device_number)                              \
  if (((device_number) < 0) || ((device_number) >= mega_num_cameras))   \
    {                                                                   \
      cam_iface_error = -1;                                             \
      CAM_IFACE_ERROR_FORMAT("device number out of range");             \
      return NULL;                                                      \
    }

CamContext* CCmega_construct( int device_number, int NumImageBuffers,
                               int mode_number) {
  int i;
  struct backend_info_t* this_backend_info;
  CamContext* result;
  cam_iface_constructor_func_t construct;
  int did_attempt_constructor;

  CHECK_DEVICE_NUMBERV(device_number)

  result = NULL;
  did_attempt_constructor = 0;

  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if ( (this_backend_info->cam_start_idx <= device_number) &&
         (device_number < this_backend_info->cam_stop_idx) ) {

      construct = this_backend_info->get_constructor_func( device_number-this_backend_info->cam_start_idx );
      did_attempt_constructor = 1;
      result = construct( device_number-this_backend_info->cam_start_idx,
                          NumImageBuffers, mode_number );
      CHECK_CI_ERRV();
    }
  }
  if (did_attempt_constructor == 0) {
    cam_iface_error = -1;
    CAM_IFACE_ERROR_FORMAT("internal error: no attempt to construct camera");
    return NULL;
  }

  if (result==NULL) {
    cam_iface_error = -1;
    CAM_IFACE_ERROR_FORMAT("failed to construct camera instance");
    return NULL;
  }
  return result;
}

cam_iface_constructor_func_t cam_iface_get_constructor_func(int device_number) {
  return CCmega_construct;
}

void cam_iface_get_mode_string(int device_number,
                               int mode_number,
                               char* mode_string, //output parameter
                               int mode_string_maxlen) {
  int i;
  struct backend_info_t* this_backend_info;

  CHECK_DEVICE_NUMBER(device_number)
  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if ( (this_backend_info->cam_start_idx <= device_number) &&
         (device_number < this_backend_info->cam_stop_idx) ) {
      this_backend_info->get_mode_string(device_number-this_backend_info->cam_start_idx,
                                         mode_number, mode_string, mode_string_maxlen);
      CHECK_CI_ERR();
    }
  }
}


void cam_iface_get_num_modes(int device_number,int* num_modes) {
  int i;
  struct backend_info_t* this_backend_info;

  CHECK_DEVICE_NUMBER(device_number)
  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if ( (this_backend_info->cam_start_idx <= device_number) &&
         (device_number < this_backend_info->cam_stop_idx) ) {
      this_backend_info->get_num_modes(device_number-this_backend_info->cam_start_idx,num_modes);
      CHECK_CI_ERR();
    }
  }
}

void cam_iface_get_camera_info(int device_number,
                               Camwire_id *out_camid) { //output parameter
  int i;
  struct backend_info_t* this_backend_info;

  CHECK_DEVICE_NUMBER(device_number)
  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if ( (this_backend_info->cam_start_idx <= device_number) &&
         (device_number < this_backend_info->cam_stop_idx) ) {
      this_backend_info->get_camera_info(device_number-this_backend_info->cam_start_idx,
                                         out_camid);
      CHECK_CI_ERR();
    }
  }
}
