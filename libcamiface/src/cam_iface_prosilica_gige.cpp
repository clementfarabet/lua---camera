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
// from Prosilica GigE SDK/examples/Stream/StdAfx.h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// from Prosilica GigE SDK/examples/Stream/Stream.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef _WINDOWS
#define u_int8_t unsigned char
#endif

#if defined(_LINUX) || defined(_QNX) || defined(__APPLE__)
#include <unistd.h>
#include <time.h>
#include <signal.h>
#endif

#include "PvApi.h"

extern "C" {
#include "cam_iface.h"
#include "cam_iface_internal.h"

#ifdef _WINDOWS
#define _STDCALL __stdcall
#else
#define _STDCALL
#endif

#if 1
#define DPRINTF(...)
#else
#define DPRINTF(...) fprintf(stderr,__VA_ARGS__)
#endif

#if defined(_LINUX) || defined(_QNX) || defined(__APPLE__)
void Sleep(unsigned int time)
{
    struct timespec t,r;

    t.tv_sec    = time / 1000;
    t.tv_nsec   = (time % 1000) * 1000000;

    while(nanosleep(&t,&r)==-1)
        t = r;
}
#endif


#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <time.h>

struct CCprosil; // forward declaration

// keep functable in sync across backends
typedef struct {
  cam_iface_constructor_func_t construct;
  void (*destruct)(struct CamContext*);

  void (*CCprosil)(struct CCprosil*,int,int,int);
  void (*close)(struct CCprosil*);
  void (*start_camera)(struct CCprosil*);
  void (*stop_camera)(struct CCprosil*);
  void (*get_num_camera_properties)(struct CCprosil*,int*);
  void (*get_camera_property_info)(struct CCprosil*,
                                   int,
                                   CameraPropertyInfo*);
  void (*get_camera_property)(struct CCprosil*,int,long*,int*);
  void (*set_camera_property)(struct CCprosil*,int,long,int);
  void (*grab_next_frame_blocking)(struct CCprosil*,
                                   unsigned char*,
                                   float);
  void (*grab_next_frame_blocking_with_stride)(struct CCprosil*,
                                               unsigned char*,
                                               intptr_t,
                                               float);
  void (*point_next_frame_blocking)(struct CCprosil*,unsigned char**,float);
  void (*unpoint_frame)(struct CCprosil*);
  void (*get_last_timestamp)(struct CCprosil*,double*);
  void (*get_last_framenumber)(struct CCprosil*,unsigned long*);
  void (*get_num_trigger_modes)(struct CCprosil*,int*);
  void (*get_trigger_mode_string)(struct CCprosil*,int,char*,int);
  void (*get_trigger_mode_number)(struct CCprosil*,int*);
  void (*set_trigger_mode_number)(struct CCprosil*,int);
  void (*get_frame_roi)(struct CCprosil*,int*,int*,int*,int*);
  void (*set_frame_roi)(struct CCprosil*,int,int,int,int);
  void (*get_max_frame_size)(struct CCprosil*,int*,int*);
  void (*get_buffer_size)(struct CCprosil*,int*);
  void (*get_framerate)(struct CCprosil*,float*);
  void (*set_framerate)(struct CCprosil*,float);
  void (*get_num_framebuffers)(struct CCprosil*,int*);
  void (*set_num_framebuffers)(struct CCprosil*,int);
} CCprosil_functable;

typedef struct CCprosil {
  CamContext inherited;
} CCprosil;


// forward declarations
CCprosil* CCprosil_construct( int device_number, int NumImageBuffers,
                              int mode_number);
void delete_CCprosil(struct CCprosil*);

void CCprosil_CCprosil(struct CCprosil*,int,int,int);
void CCprosil_close(struct CCprosil*);
void CCprosil_start_camera(struct CCprosil*);
void CCprosil_stop_camera(struct CCprosil*);
void CCprosil_get_num_camera_properties(struct CCprosil*,int*);
void CCprosil_get_camera_property_info(struct CCprosil*,
                              int,
                              CameraPropertyInfo*);
void CCprosil_get_camera_property(struct CCprosil*,int,long*,int*);
void CCprosil_set_camera_property(struct CCprosil*,int,long,int);
void CCprosil_grab_next_frame_blocking(struct CCprosil*,
                              unsigned char*,
                              float);
void CCprosil_grab_next_frame_blocking_with_stride(struct CCprosil*,
                                          unsigned char*,
                                          intptr_t,
                                          float);
void CCprosil_point_next_frame_blocking(struct CCprosil*,unsigned char**,float);
void CCprosil_unpoint_frame(struct CCprosil*);
void CCprosil_get_last_timestamp(struct CCprosil*,double*);
void CCprosil_get_last_framenumber(struct CCprosil*,unsigned long*);
void CCprosil_get_num_trigger_modes(struct CCprosil*,int*);
void CCprosil_get_trigger_mode_string(struct CCprosil*,int,char*,int);
void CCprosil_get_trigger_mode_number(struct CCprosil*,int*);
void CCprosil_set_trigger_mode_number(struct CCprosil*,int);
void CCprosil_get_frame_roi(struct CCprosil*,int*,int*,int*,int*);
void CCprosil_set_frame_roi(struct CCprosil*,int,int,int,int);
void CCprosil_get_max_frame_size(struct CCprosil*,int*,int*);
void CCprosil_get_buffer_size(struct CCprosil*,int*);
void CCprosil_get_framerate(struct CCprosil*,float*);
void CCprosil_set_framerate(struct CCprosil*,float);
void CCprosil_get_num_framebuffers(struct CCprosil*,int*);
void CCprosil_set_num_framebuffers(struct CCprosil*,int);

CCprosil_functable CCprosil_vmt = {
  (cam_iface_constructor_func_t)CCprosil_construct,
  (void (*)(CamContext*))delete_CCprosil,
  CCprosil_CCprosil,
  CCprosil_close,
  CCprosil_start_camera,
  CCprosil_stop_camera,
  CCprosil_get_num_camera_properties,
  CCprosil_get_camera_property_info,
  CCprosil_get_camera_property,
  CCprosil_set_camera_property,
  CCprosil_grab_next_frame_blocking,
  CCprosil_grab_next_frame_blocking_with_stride,
  CCprosil_point_next_frame_blocking,
  CCprosil_unpoint_frame,
  CCprosil_get_last_timestamp,
  CCprosil_get_last_framenumber,
  CCprosil_get_num_trigger_modes,
  CCprosil_get_trigger_mode_string,
  CCprosil_get_trigger_mode_number,
  CCprosil_set_trigger_mode_number,
  CCprosil_get_frame_roi,
  CCprosil_set_frame_roi,
  CCprosil_get_max_frame_size,
  CCprosil_get_buffer_size,
  CCprosil_get_framerate,
  CCprosil_set_framerate,
  CCprosil_get_num_framebuffers,
  CCprosil_set_num_framebuffers
};


// If the following is defined, we get time from the host computer clock.
//#define CIPROSIL_TIME_HOST

double ciprosil_floattime() {
#ifdef _WIN32
#if _MSC_VER == 1310
  struct _timeb t;
  _ftime(&t);
  return (double)t.time + (double)t.millitm * (double)0.001;
#else
  struct _timeb t;
  if (_ftime_s(&t)==0) {
    return (double)t.time + (double)t.millitm * (double)0.001;
  }
  else {
    return 0.0;
  }
#endif
#else
  struct timeval t;
  if (gettimeofday(&t, (struct timezone *)NULL) == 0)
    return (double)t.tv_sec + t.tv_usec*0.000001;
  else
    return 0.0;
#endif
}

#ifdef MEGA_BACKEND
  #define BACKEND_GLOBAL(m) prosilica_gige_##m
#else
  #define BACKEND_GLOBAL(m) m
#endif

/* globals -- allocate space */
  u_int64_t BACKEND_GLOBAL(prev_ts_uint64); //tmp

cam_iface_thread_local int BACKEND_GLOBAL(cam_iface_error)=0;
#define CAM_IFACE_MAX_ERROR_LEN 255
cam_iface_thread_local char BACKEND_GLOBAL(cam_iface_error_string)[CAM_IFACE_MAX_ERROR_LEN];
cam_iface_thread_local char BACKEND_GLOBAL(cam_iface_backend_string)[CAM_IFACE_MAX_ERROR_LEN];

#define PV_MAX_ENUM_LEN 32

/* global variables */
#define PV_MAX_NUM_CAMERAS 1
#define PV_MAX_NUM_BUFFERS 80
static int BACKEND_GLOBAL(num_cameras) = 0;
static tPvCameraInfo BACKEND_GLOBAL(camera_list)[PV_MAX_NUM_CAMERAS];

// circular buffer that takes advantage of 8-bit rollover
tPvFrame* BACKEND_GLOBAL(frames_ready_list_cam0)[256];
u_int8_t BACKEND_GLOBAL(frames_ready_cam0_write_idx)=0;
u_int8_t BACKEND_GLOBAL(frames_ready_cam0_read_idx)=0;
u_int8_t BACKEND_GLOBAL(frames_ready_cam0_num)=0;

typedef struct cam_iface_backend_extras cam_iface_backend_extras;
struct cam_iface_backend_extras {
  int num_buffers;
  int buf_size; // current buffer size (number of bytes)
  unsigned long malloced_buf_size; // maximum buffer size (number of bytes)
  int current_height;
  intptr_t current_width;
  int max_height;
  int max_width;
  tPvFrame** frames;
  int frame_number_currently_waiting_for;
  long last_framecount; // same type as Prosilica's FrameCount in struct tPvFrame
  long frame_epoch;
  double frame_epoch_start;
#ifndef CIPROSIL_TIME_HOST
  u_int64_t last_timestamp;
  double timestamp_tick;
#else
  double last_timestamp;
#endif // #ifndef CIPROSIL_TIME_HOST
  int exposure_mode_number;
};

#define PV_NUM_ATTR 2
const char *BACKEND_GLOBAL(pv_attr_strings)[PV_NUM_ATTR] = {
  "gain",
  "shutter" // exposure
};
#define PV_ATTR_GAIN 0
#define PV_ATTR_SHUTTER 1

  // from PvApi.h
#define PV_ERROR_NUM 22
const char *BACKEND_GLOBAL(pv_error_strings)[PV_ERROR_NUM] = {
  "No error",
  "Unexpected camera fault",
  "Unexpected fault in PvApi or driver",
  "Camera handle is invalid",
  "Bad parameter to API call",
  "Sequence of API calls is incorrect",
  "Camera or attribute not found",
  "Camera cannot be opened in the specified mode",
  "Camera was unplugged",
  "Setup is invalid (an attribute is invalid)",
  "System/network resources or memory not available",
  "1394 bandwidth not available",
  "Too many frames on queue",
  "Frame buffer is too small",
  "Frame cancelled by user",
  "The data for the frame was lost",
  "Some data in the frame is missing",
  "Timeout during wait",
  "Attribute value is out of the expected range",
  "Attribute is not this type (wrong access function)",
  "Attribute write forbidden at this time",
  "Attribute is not available at this time"
};

#ifdef MEGA_BACKEND
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(prosilica_gige_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,    \
                     "%s (%d): %s\n",__FILE__,__LINE__,(m));
#else
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,    \
                     "%s (%d): %s\n",__FILE__,__LINE__,(m));
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_THROW_ERROR(m)                        \
  {                                                     \
    prosilica_gige_cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return;                                             \
  }
#else
#define CAM_IFACE_THROW_ERROR(m)                        \
  {                                                     \
    cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return;                                             \
  }
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_THROW_ERRORV(m)                       \
  {                                                     \
    prosilica_gige_cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return NULL;                                                \
  }
#else
#define CAM_IFACE_THROW_ERRORV(m)                       \
  {                                                     \
    cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return NULL;                                                \
  }
#endif

#define CHECK_CC(m)                                                     \
  if (!(m)) {                                                           \
    CAM_IFACE_THROW_ERROR("no CamContext specified (NULL argument)");   \
  }

#define NOT_IMPLEMENTED CAM_IFACE_THROW_ERROR("not yet implemented");

#ifdef MEGA_BACKEND
#define CAM_IFACE_CHECK_DEVICE_NUMBER(m)                                \
  if ( ((m)<0) | ((m)>=prosilica_gige_num_cameras) ) {                                 \
    prosilica_gige_cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return;                                                             \
  }
#else
#define CAM_IFACE_CHECK_DEVICE_NUMBER(m)                                \
  if ( ((m)<0) | ((m)>=num_cameras) ) {                                 \
    cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return;                                                             \
  }
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_CHECK_DEVICE_NUMBERV(m)                               \
  if ( ((m)<0) | ((m)>=prosilica_gige_num_cameras) ) {                                 \
    prosilica_gige_cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return NULL;                                                        \
  }
#else
#define CAM_IFACE_CHECK_DEVICE_NUMBERV(m)                               \
  if ( ((m)<0) | ((m)>=num_cameras) ) {                                 \
    cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return NULL;                                                        \
  }
#endif

#ifdef MEGA_BACKEND
#define CIPVCHK(err) {                                                  \
  tPvErr m = err;                                                       \
  if (m!=ePvErrSuccess) {                                               \
    prosilica_gige_cam_iface_error = CAM_IFACE_GENERIC_ERROR;                          \
    if (m==ePvErrTimeout) {                                             \
      prosilica_gige_cam_iface_error = CAM_IFACE_FRAME_TIMEOUT;                        \
    }                                                                   \
    if (m<PV_ERROR_NUM) {                                               \
      cam_iface_snprintf(prosilica_gige_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: %s\n",__FILE__,__LINE__, \
                         m,                                             \
                         prosilica_gige_pv_error_strings[m]);                          \
    } else {                                                            \
      cam_iface_snprintf(prosilica_gige_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: (unknown error)\n", \
                         __FILE__,__LINE__,                             \
                         m);                                            \
    }                                                                   \
    return;                                                             \
  }                                                                     \
  }
#else
#define CIPVCHK(err) {                                                  \
  tPvErr m = err;                                                       \
  if (m!=ePvErrSuccess) {                                               \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;                          \
    if (m==ePvErrTimeout) {                                             \
      cam_iface_error = CAM_IFACE_FRAME_TIMEOUT;                        \
    }                                                                   \
    if (m<PV_ERROR_NUM) {                                               \
      cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: %s\n",__FILE__,__LINE__, \
                         m,                                             \
                         pv_error_strings[m]);                          \
    } else {                                                            \
      cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: (unknown error)\n", \
                         __FILE__,__LINE__,                             \
                         m);                                            \
    }                                                                   \
    return;                                                             \
  }                                                                     \
  }
#endif

#ifdef MEGA_BACKEND
#define CIPVCHKV(err) {                                                 \
  tPvErr m = err;                                                       \
  if (m!=ePvErrSuccess) {                                               \
    prosilica_gige_cam_iface_error = CAM_IFACE_GENERIC_ERROR;                          \
    if (m==ePvErrTimeout) {                                             \
      prosilica_gige_cam_iface_error = CAM_IFACE_FRAME_TIMEOUT;                        \
    }                                                                   \
    if (m<PV_ERROR_NUM) {                                               \
      cam_iface_snprintf(prosilica_gige_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: %s\n",__FILE__,__LINE__, \
                         m,                                             \
                         prosilica_gige_pv_error_strings[m]);                          \
    } else {                                                            \
      cam_iface_snprintf(prosilica_gige_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: (unknown error)\n", \
                         __FILE__,__LINE__,                             \
                         m);                                            \
    }                                                                   \
    return NULL;                                                        \
  }                                                                     \
}
#else
#define CIPVCHKV(err) {                                                 \
  tPvErr m = err;                                                       \
  if (m!=ePvErrSuccess) {                                               \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;                          \
    if (m==ePvErrTimeout) {                                             \
      cam_iface_error = CAM_IFACE_FRAME_TIMEOUT;                        \
    }                                                                   \
    if (m<PV_ERROR_NUM) {                                               \
      cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: %s\n",__FILE__,__LINE__, \
                         m,                                             \
                         pv_error_strings[m]);                          \
    } else {                                                            \
      cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): Prosilica GigE err %d: (unknown error)\n", \
                         __FILE__,__LINE__,                             \
                         m);                                            \
    }                                                                   \
    return NULL;                                                        \
  }                                                                     \
}
#endif

#ifdef MEGA_BACKEND
#define INTERNAL_CHK() {                                                \
    if (prosilica_gige_cam_iface_error) {                                              \
      return;                                                           \
    }                                                                   \
  }
#else
#define INTERNAL_CHK() {                                                \
    if (cam_iface_error) {                                              \
      return;                                                           \
    }                                                                   \
  }
#endif

#ifdef MEGA_BACKEND
#define INTERNAL_CHKV() {                                               \
    if (prosilica_gige_cam_iface_error) {                                              \
      return void;                                                      \
    }                                                                   \
  }
#else
#define INTERNAL_CHKV() {                                               \
    if (cam_iface_error) {                                              \
      return void;                                                      \
    }                                                                   \
  }
#endif

void _internal_start_streaming( CCprosil * ccntxt,
                                tPvHandle* handle_ptr,
                                cam_iface_backend_extras* backend_extras ) {
  // modeled after CFinderWindow::OnStart() in
  // in Prosilica's examples/SampleViewer/src/FinderWindow.cpp
  unsigned long lCapturing;
  tPvHandle iHandle = *handle_ptr;
  tPvUint32 iBytesPerFrame;

  CIPVCHK(PvCaptureQuery(iHandle,&lCapturing));
  if(lCapturing) {
    CAM_IFACE_THROW_ERROR("camera not in IDLE mode");
  }
  CIPVCHK(PvCaptureStart(iHandle));
  PvAttrUint32Get(iHandle,"TotalBytesPerFrame",&iBytesPerFrame);
  if(!iBytesPerFrame) {
    CAM_IFACE_THROW_ERROR("incorrect frame size");
  }
  backend_extras->buf_size = iBytesPerFrame;
  if ((backend_extras->malloced_buf_size) < iBytesPerFrame) {
    CAM_IFACE_THROW_ERROR("buffer is larger than allocated memory");
  }
  for (int i=0; i<backend_extras->num_buffers; i++) {
    backend_extras->frames[i]->ImageBufferSize = backend_extras->buf_size;
  }

  for (int i=0; i<backend_extras->num_buffers; i++) {
    CIPVCHK(PvCaptureQueueFrame(*handle_ptr,backend_extras->frames[i],NULL));
    BACKEND_GLOBAL(frames_ready_list_cam0)[BACKEND_GLOBAL(frames_ready_cam0_write_idx)] = backend_extras->frames[i];
    BACKEND_GLOBAL(frames_ready_cam0_write_idx)++;
    BACKEND_GLOBAL(frames_ready_cam0_num)++;
  }

  CIPVCHK(PvCommandRun(*handle_ptr,"AcquisitionStart"));
}

void _internal_stop_streaming( CCprosil * ccntxt,
                               tPvHandle* handle_ptr,
                               cam_iface_backend_extras* backend_extras ) {
  // modeled after CFinderWindow::OnStop() in
  // in Prosilica's examples/SampleViewer/src/FinderWindow.cpp
  unsigned long lCapturing;
  tPvHandle iHandle = *handle_ptr;

  CIPVCHK(PvCaptureQuery(iHandle,&lCapturing));
  if(lCapturing) {
    CIPVCHK(PvCommandRun(*handle_ptr,"AcquisitionStop"));

    // According to the PvAPI manual, this should follow the
    // PvCaptureQueueClear() call, but this order is what their sample
    // code does.
    CIPVCHK(PvCaptureEnd(iHandle));

    // Unintelligible comment from Prosilica code:
    // then dequeue all the frames still in the queue (we
    // will ignore any error as the capture was stopped anyway)
    CIPVCHK(PvCaptureQueueClear(iHandle));
  } else {
    // Comment from Prosilica code:
    // then dequeue all the frame still in the queue
    // in case there is any left in it and that the camera
    // was unplugged (we will ignore any error as the
    // capture was stopped anyway)
    CIPVCHK(PvCaptureQueueClear(iHandle));
  }
}

#include "cam_iface_prosilica_gige.h"

const char *BACKEND_METHOD(cam_iface_get_driver_name)() {
  unsigned long major, minor;
  PvVersion(&major,&minor);
  cam_iface_snprintf(BACKEND_GLOBAL(cam_iface_backend_string),CAM_IFACE_MAX_ERROR_LEN,
                     "prosilica_gige (%lu.%lu)",major,minor);
  return BACKEND_GLOBAL(cam_iface_backend_string);
}

void BACKEND_METHOD(cam_iface_clear_error)() {
  BACKEND_GLOBAL(cam_iface_error) = 0;
}

int BACKEND_METHOD(cam_iface_have_error)() {
  return BACKEND_GLOBAL(cam_iface_error);
}

const char * BACKEND_METHOD(cam_iface_get_error_string)() {
  return BACKEND_GLOBAL(cam_iface_error_string);
}

const char* BACKEND_METHOD(cam_iface_get_api_version)() {
  return CAM_IFACE_API_VERSION;
}

void BACKEND_METHOD(cam_iface_startup)() {
  unsigned long major, minor;

  PvVersion(&major,&minor);
#if defined(CAMIFACE_PROSIL_MAJOR)
  if (major!=CAMIFACE_PROSIL_MAJOR) {
    CAM_IFACE_THROW_ERROR("Prosilica library version mismatch");
  }
  if (minor!=CAMIFACE_PROSIL_MINOR) {
    CAM_IFACE_THROW_ERROR("Prosilica library version mismatch");
  }
  DPRINTF("libcamiface compiled with and loaded PvAPI version %d.%d\n",
          major,minor);
#else
  DPRINTF("libcamiface loaded PvAPI version %ld.%ld\n",
          major,minor);
#endif

  CIPVCHK(PvInitialize());

  for (int i=0;i<4;i++) {
    if (PvCameraCount()) { // wait for a camera for 4*250 msec = 1 sec
      break;
    }
    Sleep(250);
  }

  // get list of reachable cameras
  unsigned long     ul_nc, numCamerasAvail;
  ul_nc = PvCameraList(BACKEND_GLOBAL(camera_list), PV_MAX_NUM_CAMERAS, &numCamerasAvail);

  if (ul_nc != numCamerasAvail) {
    CAM_IFACE_THROW_ERROR("more cameras available than PV_MAX_NUM_CAMERAS");
  }

  if (ul_nc < PV_MAX_NUM_CAMERAS) {
    DPRINTF("trying unreachable cameras...\n");
    ul_nc += PvCameraListUnreachable(&BACKEND_GLOBAL(camera_list)[ul_nc],
                                     PV_MAX_NUM_CAMERAS-ul_nc,
                                     NULL);
  }

  BACKEND_GLOBAL(num_cameras) = (int)ul_nc; // cast to integer
}

void BACKEND_METHOD(cam_iface_shutdown)() {
  PvUnInitialize();
}

int BACKEND_METHOD(cam_iface_get_num_cameras)() {
  return BACKEND_GLOBAL(num_cameras);
}

void BACKEND_METHOD(cam_iface_get_camera_info)(int device_number, Camwire_id *out_camid) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  if (out_camid==NULL) { CAM_IFACE_THROW_ERROR("return structure NULL"); }

  cam_iface_snprintf(out_camid->vendor, CAMWIRE_ID_MAX_CHARS, "Prosilica");
  cam_iface_snprintf(out_camid->model, CAMWIRE_ID_MAX_CHARS, "%s", BACKEND_GLOBAL(camera_list)[device_number].DisplayName);
  cam_iface_snprintf(out_camid->chip, CAMWIRE_ID_MAX_CHARS, "%llXh", (long long unsigned int)BACKEND_GLOBAL(camera_list)[device_number].UniqueId);
}


void BACKEND_METHOD(cam_iface_get_num_modes)(int device_number, int *num_modes) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  *num_modes = 1; // only one mode
}

void BACKEND_METHOD(cam_iface_get_mode_string)(int device_number,
                               int mode_number,
                               char* mode_string,
                               int mode_string_maxlen) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  cam_iface_snprintf(mode_string, mode_string_maxlen, "(Prosilica GigE default mode)");
}

cam_iface_constructor_func_t BACKEND_METHOD(cam_iface_get_constructor_func)(int device_number) {
  return (CamContext* (*)(int, int, int))CCprosil_construct;
}

CCprosil* CCprosil_construct( int device_number, int NumImageBuffers,
                                 int mode_number) {
  CCprosil *ccntxt = NULL;
  ccntxt = new CCprosil; // C++ equivalent to malloc
  memset(ccntxt,0,sizeof(CCprosil));
  CCprosil_CCprosil( ccntxt, device_number,NumImageBuffers,
                     mode_number);
  return ccntxt;
}

void CCprosil_CCprosil( CCprosil * ccntxt, int device_number, int NumImageBuffers,
                        int mode_number) {

  // call parent
  CamContext_CamContext((CamContext*)ccntxt,device_number,NumImageBuffers,mode_number); // XXX cast error?
  ccntxt->inherited.vmt = (CamContext_functable*)&CCprosil_vmt;

  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  if (mode_number!=0) { CAM_IFACE_THROW_ERROR("mode number not 0"); }

  ccntxt->inherited.device_number = device_number;
  ccntxt->inherited.backend_extras = new cam_iface_backend_extras;
  memset(ccntxt->inherited.backend_extras,0,sizeof(cam_iface_backend_extras));

  tPvHandle* handle_ptr = new tPvHandle; // C++ equivalent to malloc
  CIPVCHK(PvCameraOpen(BACKEND_GLOBAL(camera_list)[device_number].UniqueId,
                        ePvAccessMaster,
                        handle_ptr ));
  ccntxt->inherited.cam = (void*)handle_ptr; // save pointer

  // Check firmware version
  unsigned long FirmwareVerMajor = 0;
  unsigned long FirmwareVerMinor = 0;
  unsigned long FirmwareVerBuild = 0;

  CIPVCHK(PvAttrUint32Get(*handle_ptr,"FirmwareVerMajor",&FirmwareVerMajor));
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"FirmwareVerMinor",&FirmwareVerMinor));
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"FirmwareVerBuild",&FirmwareVerBuild));

  // DPRINTF("firmware %d %d %d\n",FirmwareVerMajor,FirmwareVerMinor,FirmwareVerBuild);

  if ( ! ((FirmwareVerMajor >= 1) && (FirmwareVerMinor >= 24) ) ) {
    CAM_IFACE_THROW_ERROR("firmware too old - see http://www.prosilica.com/support/gige/ge_download.html");
  }

  const char *attr_names[] = {
    "Width",
    "ExposureValue",
    "FrameStartTriggerMode",
    "RegionX",
    "FrameRate",
    "PacketSize"
  };
  const int attr_names_size = sizeof(attr_names)/sizeof(const char *);

  tPvAttributeInfo attrInfo;
  for (int i=0;i<attr_names_size;i++) {
    DPRINTF("%s\n",attr_names[i]);
    CIPVCHK(PvAttrInfo(*handle_ptr,attr_names[i],&attrInfo));
    DPRINTF("     impact: %s\n",attrInfo.Impact);
    DPRINTF("     category: %s\n",attrInfo.Category);
    if (attrInfo.Flags & ePvFlagRead) {
      DPRINTF("       Read access is permitted\n");
    }
    if (attrInfo.Flags & ePvFlagWrite) {
      DPRINTF("       Write access is permitted\n");
    }
    if (attrInfo.Flags & ePvFlagVolatile) {
      DPRINTF("       The camera may change the value any time\n");
    }
    if (attrInfo.Flags & ePvFlagConst) {
      DPRINTF("       Value is read only and never changes\n");
    }
  }

  // code to adjust packet size, taken from SampleViewer -- JP May 2009.
  if(!BACKEND_METHOD(cam_iface_have_error)()){
        DPRINTF("Setting PacketSize automatically...\n");
    tPvUint32 lMaxSize = 8228;
    // get the last packet size set on the camera
    CIPVCHK(PvAttrUint32Get(*handle_ptr,"PacketSize",&lMaxSize));
    // adjust the packet size according to the current network capacity
    CIPVCHK(PvCaptureAdjustPacketSize(*handle_ptr,lMaxSize));
  }

  /*
  if (NumImageBuffers!=5) {
    DPRINTF("forcing num_buffers to 5 for performance reasons\n"); // seems to work well - ADS 20061204
    NumImageBuffers = 5;
  }
  */

  // hard-coded mono8 for now...
  ccntxt->inherited.depth = 8;
  ccntxt->inherited.coding = CAM_IFACE_MONO8;

  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  unsigned long FrameSize = 0;
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"TotalBytesPerFrame",&FrameSize));
  backend_extras->buf_size = FrameSize; // XXX should check for int overflow...

#ifndef CIPROSIL_TIME_HOST
  tPvUint32 tsf;
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"TimeStampFrequency",&tsf));
  backend_extras->timestamp_tick = 1.0/((double)tsf);
  DPRINTF("tsf %lu = dt %g\n",tsf,backend_extras->timestamp_tick);

#endif // #ifndef CIPROSIL_TIME_HOST

  CCprosil_set_trigger_mode_number( ccntxt, 0 ); // set to freerun

  /*
  char buf[PV_MAX_ENUM_LEN];
  unsigned long enum_size;
  CIPVCHK(PvAttrEnumGet(*handle_ptr,"ExposureMode",buf,PV_MAX_ENUM_LEN,&enum_size));
  fprintf(stderr,"ExposureMode enum: %s\n",buf);

  if (strncmp(buf,"FreeRun",enum_size)==0) {
    backend_extras->exposure_mode_number=0;
  } else if (strncmp(buf,"Manual",enum_size)==0) {
    backend_extras->exposure_mode_number=1;
  } else {
    fprintf(stderr,"ExposureMode enum: %s\n",buf);
    CAM_IFACE_THROW_ERROR("unknown ExposureMode enum");
  }
  */

  tPvUint32 MinWidth,MaxWidth,Width;
  CIPVCHK(PvAttrRangeUint32(*handle_ptr,"Width",&MinWidth,&MaxWidth));
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"Width",&Width));
  backend_extras->current_width = Width;
  backend_extras->max_width = MaxWidth;  // XXX should check for int overflow...
  backend_extras->frame_epoch = 0;
  backend_extras->frame_epoch_start = ciprosil_floattime();
  backend_extras->last_framecount = 0;

  tPvUint32 MinHeight,MaxHeight,Height;
  CIPVCHK(PvAttrRangeUint32(*handle_ptr,"Height",&MinHeight,&MaxHeight));
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"Height",&Height));
  backend_extras->current_height = Height;  // XXX should check for int overflow...
  backend_extras->max_height = MaxHeight;  // XXX should check for int overflow...

  backend_extras->malloced_buf_size = MaxWidth*MaxHeight;

  if (NumImageBuffers>PV_MAX_NUM_BUFFERS) {
    CAM_IFACE_THROW_ERROR("requested too many buffers");
  }

  // allocate image buffers
  backend_extras->num_buffers = NumImageBuffers;
  backend_extras->frames = (tPvFrame**)malloc( NumImageBuffers*sizeof(tPvFrame*) );
  if (backend_extras->frames == NULL) {CAM_IFACE_THROW_ERROR("could not alloc frames");}

  for (int i=0; i<NumImageBuffers; i++) {
    backend_extras->frames[i] = NULL;
  }
  for (int i=0; i<NumImageBuffers; i++) {
    backend_extras->frames[i] = new tPvFrame;
    if (backend_extras->frames[i] == NULL) {CAM_IFACE_THROW_ERROR("could not alloc frames");}
    backend_extras->frames[i]->ImageBuffer = malloc(backend_extras->malloced_buf_size);
    if (backend_extras->frames[i]->ImageBuffer == NULL) {CAM_IFACE_THROW_ERROR("could not alloc buffers");}
    backend_extras->frames[i]->ImageBufferSize = backend_extras->buf_size;
    backend_extras->frames[i]->AncillaryBuffer = NULL;
    backend_extras->frames[i]->AncillaryBufferSize = 0;
    backend_extras->frames[i]->Context[0] = (void*)i;
  }
  backend_extras->frame_number_currently_waiting_for=0; // first frame first
}

void delete_CCprosil(CCprosil *ccntxt) {
  CCprosil_close(ccntxt);
  delete ccntxt;
  ccntxt = (CCprosil*)NULL;
}

void CCprosil_close(CCprosil *ccntxt) {
  if (!ccntxt) {CAM_IFACE_THROW_ERROR("no CCprosil specified (NULL argument)");}
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;

  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  _internal_stop_streaming(ccntxt,handle_ptr,backend_extras);INTERNAL_CHK();

  if (backend_extras!=NULL) {
    if (backend_extras->frames!=NULL) {

      //CIPVCHK(PvCaptureQueueClear(*handle_ptr));
      CIPVCHK(PvCaptureEnd(*handle_ptr));

      for (int i=0; i<(backend_extras->num_buffers); i++) {
        if (backend_extras->frames[i] != NULL) {
          if (backend_extras->frames[i]->ImageBuffer != NULL) {
            free(backend_extras->frames[i]->ImageBuffer);
            backend_extras->frames[i]->ImageBuffer = (void*)NULL;
          }
          delete backend_extras->frames[i];
        }
      }
      free(backend_extras->frames);
    }
    delete backend_extras;
    ccntxt->inherited.backend_extras = (void*)NULL;
  }

  delete handle_ptr;
  ccntxt->inherited.cam = (void*)NULL;
}

void CCprosil_start_camera( CCprosil *ccntxt ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  _internal_start_streaming(ccntxt,handle_ptr,backend_extras);INTERNAL_CHK();
}

void CCprosil_stop_camera( CCprosil *ccntxt ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  _internal_stop_streaming(ccntxt,handle_ptr,backend_extras);INTERNAL_CHK();
}

void CCprosil_get_num_camera_properties(CCprosil *ccntxt,
                                          int* num_properties) {
  CHECK_CC(ccntxt);
  /*
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  tPvAttrListPtr attr_list;
  unsigned long num_props_pv=0;
  CIPVCHK(PvAttrList(*handle_ptr,&attr_list,&num_props_pv));
  for (int i=0;i<num_props_pv;i++) {
    DPRINTF("attr: %d %s\n",i,attr_list[i]);
  }
  */
  *num_properties = PV_NUM_ATTR;
}

void CCprosil_get_camera_property_info(CCprosil *ccntxt,
                                         int property_number,
                                         CameraPropertyInfo *info) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;

  if (info==NULL) {
    CAM_IFACE_THROW_ERROR("no info argument specified (NULL argument)");
  }

  info->is_present = 1;

  info->min_value = 0;
  //info->max_value = min(MAX_LONG,MAX_UINT32);
  info->max_value = 0x7FFFFFFF;

  info->has_auto_mode = 1;
  info->has_manual_mode = 1;

  info->is_scaled_quantity = 0;

  info->original_value = 0;

  info->available = 1;
  info->readout_capable = 1;
  info->on_off_capable = 0;

  info->absolute_capable = 0;
  info->absolute_control_mode = 0;
  info->absolute_min_value = 0.0;
  info->absolute_max_value = 0.0;

  tPvUint32 mymin,mymax;

  switch (property_number) {
  case PV_ATTR_GAIN:
    info->name = "gain";
    info->has_auto_mode = 0;
    CIPVCHK(PvAttrRangeUint32(*handle_ptr,"GainValue",&mymin,&mymax));
    info->min_value = mymin;
    info->max_value = mymax;
    break;
  case PV_ATTR_SHUTTER:
    info->name = "shutter";
    CIPVCHK(PvAttrRangeUint32(*handle_ptr,"ExposureValue",&mymin,&mymax));
    info->min_value = mymin;
    /// XXX HACK!!!
    //info->max_value = mymax;
    info->max_value = 50000;
    DPRINTF("WARNING: artificially setting max_value of shutter to 50000 in %s, %d\n",__FILE__,__LINE__);
    info->is_scaled_quantity = 1;
    info->scaled_unit_name = "usec";
    info->scale_offset = 0;
    info->scale_gain = 1e-3; // convert from microsecond to millisecond
    break;
  default:
    CAM_IFACE_THROW_ERROR("invalid property number");
    break;
  }

  return;
}

void CCprosil_get_camera_property(CCprosil *ccntxt,
                                    int property_number,
                                    long* Value,
                                    int* Auto ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;

  tPvUint32 value;
  char buf[PV_MAX_ENUM_LEN];
  unsigned long enum_size;

  switch (property_number) {
  case PV_ATTR_GAIN:
    CIPVCHK(PvAttrEnumGet(*handle_ptr,"GainMode",buf,PV_MAX_ENUM_LEN,&enum_size));
    CIPVCHK(PvAttrUint32Get(*handle_ptr,"GainValue",&value));
    *Value = value;
    if (strncmp(buf,"Manual",enum_size)==0) {
      *Auto = 0;
    } else if (strncmp(buf,"Auto",enum_size)==0) {
      *Auto = 1;
    } else {
      CAM_IFACE_THROW_ERROR("unknown enum");
    }
    break;
  case PV_ATTR_SHUTTER:
    CIPVCHK(PvAttrEnumGet(*handle_ptr,"ExposureMode",buf,PV_MAX_ENUM_LEN,&enum_size));
    CIPVCHK(PvAttrUint32Get(*handle_ptr,"ExposureValue",&value));
    *Value = value;
    if (strncmp(buf,"Manual",enum_size)==0) {
      *Auto = 0;
    } else if (strncmp(buf,"Auto",enum_size)==0) {
      *Auto = 1;
    } else {
      CAM_IFACE_THROW_ERROR("unknown enum");
    }
    break;
  default:
    CAM_IFACE_THROW_ERROR("invalid property number");
    break;
  }
  return;
}

void CCprosil_set_camera_property(CCprosil *ccntxt,
                                    int property_number,
                                    long Value,
                                    int Auto ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  const char* mode_str=NULL;
  tPvUint32 value = Value;

  switch (property_number) {
  case PV_ATTR_GAIN:
    if (Auto!=0) {
      CAM_IFACE_THROW_ERROR("auto gain not available");
    }
    CIPVCHK(PvAttrUint32Set(*handle_ptr,"GainValue",value));
    break;
  case PV_ATTR_SHUTTER:
    if (Auto==0) {
      mode_str = "Manual";
    } else {
      mode_str = "Auto";
    }
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"ExposureMode",mode_str));
    CIPVCHK(PvAttrUint32Set(*handle_ptr,"ExposureValue",value));
    break;
  default:
    CAM_IFACE_THROW_ERROR("invalid property number");
    break;
  }
  return;
}

void CCprosil_grab_next_frame_blocking( CCprosil *ccntxt, unsigned char *out_bytes, float timeout) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  CCprosil_grab_next_frame_blocking_with_stride(ccntxt,out_bytes,
                                                  backend_extras->current_width,
                                                  timeout);
}

void CCprosil_grab_next_frame_blocking_with_stride( CCprosil *ccntxt,
                                                      unsigned char *out_bytes,
                                                      intptr_t stride0,
                                                      float timeout ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  tPvFrame* frame;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  unsigned long pvTimeout;
  double now;
  bool recent_rollover;

  if (timeout < 0)
    pvTimeout = PVINFINITE;
  else
    pvTimeout = (unsigned long)ceilf(timeout*1000.0f); // convert to msec

  frame = BACKEND_GLOBAL(frames_ready_list_cam0)[BACKEND_GLOBAL(frames_ready_cam0_read_idx)];
  if (frame==NULL) CAM_IFACE_THROW_ERROR("internal cam_iface error: frame not allocated");

  CIPVCHK(PvCaptureWaitForFrameDone(*handle_ptr,frame,pvTimeout));

  BACKEND_GLOBAL(frames_ready_cam0_read_idx)++;
  BACKEND_GLOBAL(frames_ready_cam0_num)--;

  size_t wb = frame->Width;
  int height = frame->Height;

  for (int row=0;row<height;row++) {
    memcpy((void*)(out_bytes+row*stride0), //dest
           (const void*)( ((intptr_t)(frame->ImageBuffer)) + row*wb),//src
           wb);//size
  }
  if (getenv("PROSILICA_BACKEND_DEBUG")!=NULL) {
    fprintf(stderr,"frame->FrameCount %lu\n",frame->FrameCount);
  }

  now = ciprosil_floattime();
  recent_rollover = (now - (backend_extras->frame_epoch_start) <= 30.0);

  // FrameCount can rollover, but we don't want that
  if ( ((backend_extras->last_framecount-1)%(long)(0xFFFF)) > (long)0xFF00) {
    if ( ((long)frame->FrameCount) < (long)0xFF00 ) {
      if (!recent_rollover) {
        // wait 30 seconds before allowing rollover again
        backend_extras->frame_epoch++;
        backend_extras->frame_epoch_start = now;
      }
    }
  }

  if (recent_rollover) {
    if ( ((long)frame->FrameCount) > (long)0xFF00 ) {
      // These frames are probably coming in from before rollover.
      backend_extras->last_framecount = ((backend_extras->frame_epoch-1)*(long)0xFFFF)+
        ((long)frame->FrameCount);
    } else {
      backend_extras->last_framecount = ((backend_extras->frame_epoch)*(long)0xFFFF)+
        ((long)frame->FrameCount);
    }
  } else {
    backend_extras->last_framecount = ((backend_extras->frame_epoch)*(long)0xFFFF)+
      ((long)frame->FrameCount);
  }

  if (getenv("PROSILICA_BACKEND_DEBUG")!=NULL) {
    fprintf(stderr,"backend_extras->last_framecount %ld\n",backend_extras->last_framecount);
  }
#ifndef CIPROSIL_TIME_HOST
  u_int64_t ts_uint64;
  ts_uint64 = (((u_int64_t)(frame->TimestampHi))<<32) + (frame->TimestampLo);
  int64_t dif64; //tmp
  dif64=ts_uint64-BACKEND_GLOBAL(prev_ts_uint64);
  BACKEND_GLOBAL(prev_ts_uint64) = ts_uint64;

  DPRINTF("got it                         (ts %llu)    (diff %lld)!\n",ts_uint64,dif64);
  backend_extras->last_timestamp = ts_uint64;
#else // #ifndef CIPROSIL_TIME_HOST
  backend_extras->last_timestamp = ciprosil_floattime();
#endif // #ifndef CIPROSIL_TIME_HOST

  tPvErr oldstatus = frame->Status;

  //if (requeue_int==0) {
    // re-queue frame buffer
    CIPVCHK(PvCaptureQueueFrame(*handle_ptr,frame,NULL));
    //    printf("queued frame %d\n",int(frame->Context[0]));
    BACKEND_GLOBAL(frames_ready_list_cam0)[BACKEND_GLOBAL(frames_ready_cam0_write_idx)] = frame;
    BACKEND_GLOBAL(frames_ready_cam0_write_idx)++;
    BACKEND_GLOBAL(frames_ready_cam0_num)++;
    //}

  if(oldstatus == ePvErrDataMissing) {
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_FRAME_DATA_MISSING_ERROR;
    CAM_IFACE_ERROR_FORMAT("frame data missing");
    return;
  }

  if(oldstatus == ePvErrDataLost) {
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_FRAME_DATA_LOST_ERROR;
    CAM_IFACE_ERROR_FORMAT("frame data lost");
    return;
  }
}

void CCprosil_point_next_frame_blocking( CCprosil *ccntxt, unsigned char **buf_ptr,
                                           float timeout){
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}
void CCprosil_unpoint_frame( CCprosil *ccntxt){
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}

void CCprosil_get_last_timestamp( CCprosil *ccntxt, double* timestamp ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

#ifndef CIPROSIL_TIME_HOST
  *timestamp = (double)(backend_extras->last_timestamp) * backend_extras->timestamp_tick;
#else // #ifndef CIPROSIL_TIME_HOST
  *timestamp = backend_extras->last_timestamp;
#endif // #ifndef CIPROSIL_TIME_HOST

}

void CCprosil_get_last_framenumber( CCprosil *ccntxt, unsigned long* framenumber ){
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *framenumber = (unsigned long)(backend_extras->last_framecount);
  if (getenv("PROSILICA_BACKEND_DEBUG")!=NULL) {
    fprintf(stderr,"*framenumber %lu\n",*framenumber);
  }
}

void CCprosil_get_num_trigger_modes( CCprosil *ccntxt,
                                       int *num_exposure_modes ) {
  CHECK_CC(ccntxt);
  *num_exposure_modes = 5;
}

void CCprosil_get_trigger_mode_string( CCprosil *ccntxt,
                                         int exposure_mode_number,
                                         char* exposure_mode_string, //output parameter
                                         int exposure_mode_string_maxlen) {
  CHECK_CC(ccntxt);
  switch (exposure_mode_number) {
  case 0:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"FreeRun");
    break;
  case 1:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"SyncIn1");
    break;
  case 2:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"SyncIn2");
    break;
  case 3:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"SyncIn3");
    break;
  case 4:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"SyncIn4");
    break;
  default:
    BACKEND_GLOBAL(cam_iface_error) = -1;
    CAM_IFACE_ERROR_FORMAT("exposure_mode_number invalid");
    return;
  }
}

void CCprosil_get_trigger_mode_number( CCprosil *ccntxt,
                                         int *exposure_mode_number ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *exposure_mode_number = (backend_extras->exposure_mode_number);
}

void CCprosil_set_trigger_mode_number( CCprosil *ccntxt,
                                         int exposure_mode_number ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  switch (exposure_mode_number) {
  case 0:
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"FrameStartTriggerMode","Freerun"));
    break;
  case 1:
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"FrameStartTriggerMode","SyncIn1"));
    break;
  case 2:
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"FrameStartTriggerMode","SyncIn2"));
    break;
  case 3:
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"FrameStartTriggerMode","SyncIn3"));
    break;
  case 4:
    CIPVCHK(PvAttrEnumSet(*handle_ptr,"FrameStartTriggerMode","SyncIn4"));
    break;
  default:
    CAM_IFACE_THROW_ERROR("exposure_mode_number invalid");
    break;
  }
  backend_extras->exposure_mode_number = exposure_mode_number;
}

void CCprosil_get_frame_roi( CCprosil *ccntxt,
                             int *left, int *top, int* width, int* height ) {
  tPvUint32 l,t;

  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"RegionX",&l));
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"RegionY",&t));

  *left = l;  // XXX cast error?
  *top = t;   // XXX cast error?
  *width = backend_extras->current_width;
  *height = backend_extras->current_height;
}

void CCprosil_set_frame_roi( CCprosil *ccntxt,
                             int left, int top, int width, int height ) {
  // XXX not yet done
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  tPvUint32 l,t;
  tPvUint32 w,h;
  w=width;// XXX should check for int overflow...
  h=height;

  l=left;// XXX should check for int overflow...
  t=top;
  _internal_stop_streaming( ccntxt, handle_ptr, backend_extras );INTERNAL_CHK();
  CIPVCHK(PvAttrUint32Set(*handle_ptr,"Width",w));
  backend_extras->current_width = width;
  CIPVCHK(PvAttrUint32Set(*handle_ptr,"Height",h));
  backend_extras->current_height = height;

  unsigned long FrameSize = 0;
  CIPVCHK(PvAttrUint32Get(*handle_ptr,"TotalBytesPerFrame",&FrameSize));
  backend_extras->buf_size = FrameSize; // XXX should check for int overflow...

  for (int i=0; i<backend_extras->num_buffers; i++) {
    backend_extras->frames[i]->ImageBufferSize = backend_extras->buf_size;
  }

  CIPVCHK(PvAttrUint32Set(*handle_ptr,"RegionX",l));
  CIPVCHK(PvAttrUint32Set(*handle_ptr,"RegionY",t));
  _internal_start_streaming( ccntxt, handle_ptr, backend_extras );INTERNAL_CHK();
}

void CCprosil_get_buffer_size( CCprosil *ccntxt,
                                 int *size) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *size = backend_extras->buf_size;
}

void CCprosil_get_framerate( CCprosil *ccntxt,
                               float *framerate ) {
  CHECK_CC(ccntxt);
  tPvHandle* handle_ptr = (tPvHandle*)ccntxt->inherited.cam;
  CIPVCHK(PvAttrFloat32Get(*handle_ptr,"FrameRate",framerate));
}

void CCprosil_set_framerate( CCprosil *ccntxt,
                               float framerate ) {
  CHECK_CC(ccntxt);
  CAM_IFACE_THROW_ERROR("frame rate is not settable");
}

void CCprosil_get_max_frame_size( CCprosil *ccntxt,
                                    int *width, int *height ){
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *width = backend_extras->max_width;
  *height = backend_extras->max_height;
}

void CCprosil_get_num_framebuffers( CCprosil *ccntxt,
                                      int *num_framebuffers ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *num_framebuffers = backend_extras->num_buffers;
}

void CCprosil_set_num_framebuffers( CCprosil *ccntxt,
                                      int num_framebuffers ) {
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}

} // closes: extern "C"
