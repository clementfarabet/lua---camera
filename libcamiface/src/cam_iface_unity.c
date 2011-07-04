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
#include <dlfcn.h>
#include <stdlib.h>

#ifndef UNITY_BACKEND_DIR
# error "must define UNITY_BACKEND_DIR"
#endif

#ifndef UNITY_BACKEND_PREFIX
# error "must define UNITY_BACKEND_PREFIX"
#endif

#ifndef UNITY_BACKEND_SUFFIX
# error "must define UNITY_BACKEND_SUFFIX"
#endif


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
int unity_num_cameras = 0;

cam_iface_thread_local int cam_iface_error = 0;
#define CAM_IFACE_MAX_ERROR_LEN 255
cam_iface_thread_local char cam_iface_error_string[CAM_IFACE_MAX_ERROR_LEN]  = {0x00}; //...

char *backend_names[NUM_BACKENDS] = UNITY_BACKENDS;
struct backend_info_t backend_info[NUM_BACKENDS] = {};
static int backends_started = 0;

#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,            \
           "%s (%d): %s\n",__FILE__,__LINE__,(m));

#define CHECK_CI_ERR()                                          \
  if (this_backend_info->have_error()) {                        \
    cam_iface_error = this_backend_info->have_error();          \
    cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,  \
             "unity backend error (%s %d): %s",__FILE__,        \
             __LINE__,this_backend_info->get_error_string());   \
    return;                                                     \
  }

#define CHECK_CI_ERRV()                                         \
if (this_backend_info->have_error()) {                          \
cam_iface_error = this_backend_info->have_error();              \
cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,  \
"unity backend error (%s %d): %s",__FILE__,                     \
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

#define LOAD_DLSYM(var,name) {                                  \
    (var) = dlsym(libhandle,(name));                            \
    if ((var)==NULL) {                                          \
      cam_iface_error = -1;                                     \
      CAM_IFACE_ERROR_FORMAT("dlsym() error");                  \
      return;                                                   \
    }                                                           \
  }

const char *cam_iface_get_driver_name(void) {
  return "unity";
}

int cam_iface_get_num_cameras(void) {
  return unity_num_cameras;
}

void cam_iface_startup(void) {
  int* libhandle;
  struct backend_info_t* this_backend_info;
  char *full_backend_name;
  int i, j, next_num_cameras;
  char *envvar;
  int try_this_name;

  unity_num_cameras = 0;

  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    this_backend_info->name = backend_names[i];
    this_backend_info->started = 0;

    if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
      fprintf(stderr,"%s: %d: this_backend_info = %p\n",
              __FILE__,__LINE__,this_backend_info);
      fprintf(stderr,"%s: %d: this_backend_info->name = %s\n",
              __FILE__,__LINE__,this_backend_info->name);
      fprintf(stderr,"%s: %d: this_backend_info->started = %d\n",
              __FILE__,__LINE__,this_backend_info->started);
    }

    for (j=0; j<3; j++) {

      full_backend_name = (char*)malloc( 256*sizeof(char) );
      try_this_name = 1;
      switch (j) {
      case 0:
        /* Check environment variables first */
        envvar = getenv("UNITY_BACKEND_DIR");
        if (envvar != NULL) {
          cam_iface_snprintf(full_backend_name,256,"%s/" UNITY_BACKEND_PREFIX "cam_iface_%s" UNITY_BACKEND_SUFFIX ,
                   envvar,backend_names[i]);
        } else {
          try_this_name = 0;
		}
        break;
      case 1:
        /* Check pwd next */
        cam_iface_snprintf(full_backend_name,256,"./" UNITY_BACKEND_PREFIX "cam_iface_%s" UNITY_BACKEND_SUFFIX ,backend_names[i]);
        break;
      case 2:
        /* Next check system-install prefix */
        cam_iface_snprintf(full_backend_name,256,UNITY_BACKEND_DIR UNITY_BACKEND_PREFIX "cam_iface_%s" UNITY_BACKEND_SUFFIX ,backend_names[i]);
        break;
      default:
        cam_iface_error=-1;
        CAM_IFACE_ERROR_FORMAT("error in switch statement");
        return;
      }

      if (!try_this_name) {
        free(full_backend_name);
        continue;
      }

      // RTLD_GLOBAL needed for embedded Python to work. (For examples, see pythoncall.c
      // and pymplug.c.)

      if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
        fprintf(stderr,"%s: %d: attempting to open: %s\n",__FILE__,__LINE__,full_backend_name);
      }
      libhandle = dlopen(full_backend_name, RTLD_NOW | RTLD_GLOBAL );
      if (libhandle==NULL) {
        if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
          fprintf(stderr,"%s: %d: %s failed.\n",__FILE__,__LINE__,full_backend_name);
        }
        this_backend_info->cam_start_idx = unity_num_cameras;
        this_backend_info->cam_stop_idx = unity_num_cameras;
        free(full_backend_name);
        continue;
      } else {
        if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
          fprintf(stderr,"%s: %d: %s OK, libhandle = %p\n",
                  __FILE__,__LINE__,
                  full_backend_name,libhandle);
        }
        free(full_backend_name);
        break; // found backend, stop searching
      }
    }

    if (libhandle==NULL) {
      continue; //  no backend loaded
    }

    if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
      fprintf(stderr,"%s: %d: Loading symbols from libhandle %p\n",
              __FILE__,__LINE__,libhandle);
    }

    LOAD_DLSYM(this_backend_info->have_error,"cam_iface_have_error");
    LOAD_DLSYM(this_backend_info->clear_error,"cam_iface_clear_error");
    LOAD_DLSYM(this_backend_info->get_error_string,"cam_iface_get_error_string");
    LOAD_DLSYM(this_backend_info->startup,"cam_iface_startup");
    LOAD_DLSYM(this_backend_info->shutdown,"cam_iface_shutdown");
    LOAD_DLSYM(this_backend_info->get_num_cameras,"cam_iface_get_num_cameras");
    LOAD_DLSYM(this_backend_info->get_num_modes,"cam_iface_get_num_modes");
    LOAD_DLSYM(this_backend_info->get_camera_info,"cam_iface_get_camera_info");

    LOAD_DLSYM(this_backend_info->get_mode_string,"cam_iface_get_mode_string");
    LOAD_DLSYM(this_backend_info->get_constructor_func,"cam_iface_get_constructor_func");

    if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
      fprintf(stderr,"%s: %d: this_backend_info = %p\n",
              __FILE__,__LINE__,this_backend_info);
      fprintf(stderr,"%s: %d: this_backend_info->name = %s\n",
              __FILE__,__LINE__,this_backend_info->name);
      fprintf(stderr,"%s: %d: this_backend_info->started = %d\n",
              __FILE__,__LINE__,this_backend_info->started);
      fprintf(stderr,"%s: %d: this_backend_info->have_error = %p\n",
              __FILE__,__LINE__,this_backend_info->have_error);
    }

    this_backend_info->clear_error();
    CHECK_CI_ERR();

    this_backend_info->startup();
    if (this_backend_info->have_error()) {
      if (getenv("UNITY_BACKEND_DEBUG")!=NULL) {
        fprintf(stderr,"%s: %d: %s backend startup() had error '%s'\n",
                __FILE__,__LINE__,
                this_backend_info->name,
                this_backend_info->get_error_string());
      }
      continue; //  backend startup had error
    }

    this_backend_info->started = 1;

    next_num_cameras = unity_num_cameras + this_backend_info->get_num_cameras();
    this_backend_info->cam_start_idx = unity_num_cameras;
    this_backend_info->cam_stop_idx = next_num_cameras;
    unity_num_cameras = next_num_cameras;
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

CamContext* CCunity_construct( int device_number, int NumImageBuffers,
                               int mode_number) {
  int i;
  struct backend_info_t* this_backend_info;
  CamContext* result;
  cam_iface_constructor_func_t construct;

  for (i=0; i<NUM_BACKENDS; i++) {
    this_backend_info = &(backend_info[i]);
    if ( (this_backend_info->cam_start_idx <= device_number) &&
         (device_number < this_backend_info->cam_stop_idx) ) {

      construct = this_backend_info->get_constructor_func( device_number-this_backend_info->cam_start_idx );
      result = construct( device_number-this_backend_info->cam_start_idx,
                          NumImageBuffers, mode_number );
      CHECK_CI_ERRV();
    }
  }
  return result;
}

cam_iface_constructor_func_t cam_iface_get_constructor_func(int device_number) {
  return CCunity_construct;
}

void cam_iface_get_mode_string(int device_number,
                               int mode_number,
                               char* mode_string, //output parameter
                               int mode_string_maxlen) {
  int i;
  struct backend_info_t* this_backend_info;
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
