/*

Copyright (c) 2004-2010, California Institute of Technology. All
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

// This #if statement from FlyCapture2Test/stdafx.h
#if defined(WIN32) || defined(WIN64)

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#include <Windows.h>
#define u_int8_t unsigned char

#include <stdio.h>
#include <tchar.h>

#elif defined(MACOSX)

#else

#include <unistd.h>
#include <time.h>
 //#include <signal.h>
#include <stdio.h>

#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "FlyCapture2.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef MEGA_BACKEND
  #define BACKEND_GLOBAL(m) pgr_flycapture_##m
#else
  #define BACKEND_GLOBAL(m) m
#endif

/* global variables */
static int BACKEND_GLOBAL(num_cameras) = 0;
static FlyCapture2::BusManager* BACKEND_GLOBAL(busMgr_ptr);

class CamMode {
public:
  std::string descr;
  FlyCapture2::VideoMode videomode; // VIDEOMODE_FORMAT7 IFF format 7
  FlyCapture2::FrameRate framerate; // not used IFF format 7
  FlyCapture2::Format7ImageSettings fmt7ImageSettings;
  FlyCapture2::Format7PacketInfo fmt7PacketInfo;
  int format;
};

std::string pixfmt2string(FlyCapture2::PixelFormat pixfmt) {
  switch (pixfmt) {
  case FlyCapture2::PIXEL_FORMAT_MONO8: return std::string("MONO8");
  case FlyCapture2::PIXEL_FORMAT_411YUV8: return std::string("411YUV8");
  case FlyCapture2::PIXEL_FORMAT_422YUV8: return std::string("422YUV8");
  case FlyCapture2::PIXEL_FORMAT_444YUV8: return std::string("444YUV8");
  case FlyCapture2::PIXEL_FORMAT_RGB8: return std::string("RGB8");
  case FlyCapture2::PIXEL_FORMAT_MONO16: return std::string("MONO16");
  case FlyCapture2::PIXEL_FORMAT_RGB16: return std::string("RGB16");
  case FlyCapture2::PIXEL_FORMAT_S_MONO16: return std::string("MONO16");
  case FlyCapture2::PIXEL_FORMAT_S_RGB16: return std::string("RGB16");
  case FlyCapture2::PIXEL_FORMAT_RAW8: return std::string("RAW8");
  case FlyCapture2::PIXEL_FORMAT_RAW16: return std::string("RAW16");
  case FlyCapture2::PIXEL_FORMAT_MONO12: return std::string("MONO12");
  case FlyCapture2::PIXEL_FORMAT_RAW12: return std::string("RAW12");
  case FlyCapture2::PIXEL_FORMAT_BGR: return std::string("BGR");
  case FlyCapture2::PIXEL_FORMAT_BGRU: return std::string("BGRU");
    //case FlyCapture2::PIXEL_FORMAT_RGB: return std::string("RGB");
  case FlyCapture2::PIXEL_FORMAT_RGBU: return std::string("RGBU");
  default: return std::string("UNKNOWN");
  }
}

int get_mode_list(int device_number, std::vector<CamMode> &result ) {

  FlyCapture2::Camera *cam = new FlyCapture2::Camera;
  FlyCapture2::PGRGuid guid;
  FlyCapture2::Error err;
  CamMode mode;
  FlyCapture2::Format7Info fmt7Info;
  bool supported;

  //FlyCapture2::Mode k_fmt7Mode;
  //FlyCapture2::PixelFormat k_fmt7PixFmt;
  std::ostringstream oss;
  std::vector<FlyCapture2::Mode> fmt7modes;
  fmt7modes.push_back(FlyCapture2::MODE_0);
  fmt7modes.push_back(FlyCapture2::MODE_1);
  fmt7modes.push_back(FlyCapture2::MODE_2);
  fmt7modes.push_back(FlyCapture2::MODE_3);
  fmt7modes.push_back(FlyCapture2::MODE_4);
  fmt7modes.push_back(FlyCapture2::MODE_5);
  fmt7modes.push_back(FlyCapture2::MODE_6);
  fmt7modes.push_back(FlyCapture2::MODE_7);
  fmt7modes.push_back(FlyCapture2::MODE_8);
  fmt7modes.push_back(FlyCapture2::MODE_9);
  fmt7modes.push_back(FlyCapture2::MODE_10);
  fmt7modes.push_back(FlyCapture2::MODE_11);
  fmt7modes.push_back(FlyCapture2::MODE_12);
  fmt7modes.push_back(FlyCapture2::MODE_13);
  fmt7modes.push_back(FlyCapture2::MODE_14);
  fmt7modes.push_back(FlyCapture2::MODE_15);
  fmt7modes.push_back(FlyCapture2::MODE_16);
  fmt7modes.push_back(FlyCapture2::MODE_17);
  fmt7modes.push_back(FlyCapture2::MODE_18);
  fmt7modes.push_back(FlyCapture2::MODE_19);
  fmt7modes.push_back(FlyCapture2::MODE_20);
  fmt7modes.push_back(FlyCapture2::MODE_21);
  fmt7modes.push_back(FlyCapture2::MODE_22);
  fmt7modes.push_back(FlyCapture2::MODE_23);
  fmt7modes.push_back(FlyCapture2::MODE_24);
  fmt7modes.push_back(FlyCapture2::MODE_25);
  fmt7modes.push_back(FlyCapture2::MODE_26);
  fmt7modes.push_back(FlyCapture2::MODE_27);
  fmt7modes.push_back(FlyCapture2::MODE_28);
  fmt7modes.push_back(FlyCapture2::MODE_29);
  fmt7modes.push_back(FlyCapture2::MODE_30);
  fmt7modes.push_back(FlyCapture2::MODE_31);

  std::vector<FlyCapture2::PixelFormat> pixfmts;
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_MONO8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_411YUV8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_422YUV8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_444YUV8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RGB8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_MONO16);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RGB16);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_S_MONO16);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_S_RGB16);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RAW8);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RAW16);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_MONO12);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RAW12);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_BGR);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_BGRU);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RGB);
  pixfmts.push_back(FlyCapture2::PIXEL_FORMAT_RGBU);

  err = BACKEND_GLOBAL(busMgr_ptr)->GetCameraFromIndex(device_number, &guid);
  // if (err!=FlyCapture2::PGRERROR_OK) {
  //   goto errlabel2;
  // }
  err = cam->Connect(&guid);
  // if (err!=FlyCapture2::PGRERROR_OK) {
  //   goto errlabel2;
  // }

  // FORMAT 7 -- test all modes
  std::vector<FlyCapture2::Mode>::const_iterator fmt7mode;
  for(fmt7mode=fmt7modes.begin(); fmt7mode!=fmt7modes.end(); fmt7mode++) {
    fmt7Info.mode = *fmt7mode;
    err = cam->GetFormat7Info( &fmt7Info, &supported );
    // if (err != FlyCapture2::PGRERROR_OK) goto errlabel1;

    // Do work
    if (supported) {

      std::vector<FlyCapture2::PixelFormat>::const_iterator pixfmt;
      for(pixfmt=pixfmts.begin(); pixfmt!=pixfmts.end(); pixfmt++) {
	if (*pixfmt & fmt7Info.pixelFormatBitField) {

	  std::ostringstream oss = std::ostringstream();
	  oss << "format 7, mode " << *fmt7mode << " (" << pixfmt2string(*pixfmt) << ", " << fmt7Info.maxWidth << "x" << fmt7Info.maxHeight <<")";

	  mode.descr = oss.str();
	  mode.videomode = FlyCapture2::VIDEOMODE_FORMAT7;
	  mode.fmt7ImageSettings.mode = *fmt7mode;
	  mode.fmt7ImageSettings.offsetX = 0;
	  mode.fmt7ImageSettings.offsetY = 0;
	  mode.fmt7ImageSettings.width = fmt7Info.maxWidth;
	  mode.fmt7ImageSettings.height = fmt7Info.maxHeight;
	  mode.fmt7ImageSettings.pixelFormat = *pixfmt;

	  bool valid;
	  err = cam->ValidateFormat7Settings( &mode.fmt7ImageSettings,
					      &valid,
					      &mode.fmt7PacketInfo );
	  if (err == FlyCapture2::PGRERROR_OK) {
	    result.push_back(mode);
	  }
	}
      }
    }
  }

  // not format 7 -- test each videomode and framerate combination

  std::vector<FlyCapture2::VideoMode> videomodes;
  videomodes.push_back(FlyCapture2::VIDEOMODE_160x120YUV444);
  videomodes.push_back(FlyCapture2::VIDEOMODE_320x240YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_640x480YUV411);
  videomodes.push_back(FlyCapture2::VIDEOMODE_640x480YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_640x480RGB);
  videomodes.push_back(FlyCapture2::VIDEOMODE_640x480Y8);
  videomodes.push_back(FlyCapture2::VIDEOMODE_640x480Y16);
  videomodes.push_back(FlyCapture2::VIDEOMODE_800x600YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_800x600RGB);
  videomodes.push_back(FlyCapture2::VIDEOMODE_800x600Y8);
  videomodes.push_back(FlyCapture2::VIDEOMODE_800x600Y16);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1024x768YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1024x768RGB);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1024x768Y8);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1024x768Y16);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1280x960YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1280x960RGB);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1280x960Y8);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1280x960Y16);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1600x1200YUV422);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1600x1200RGB);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1600x1200Y8);
  videomodes.push_back(FlyCapture2::VIDEOMODE_1600x1200Y16);

  std::vector<FlyCapture2::FrameRate> framerates;
  framerates.push_back(FlyCapture2::FRAMERATE_1_875);
  framerates.push_back(FlyCapture2::FRAMERATE_3_75);
  framerates.push_back(FlyCapture2::FRAMERATE_7_5);
  framerates.push_back(FlyCapture2::FRAMERATE_15);
  framerates.push_back(FlyCapture2::FRAMERATE_30);
  framerates.push_back(FlyCapture2::FRAMERATE_60);
  framerates.push_back(FlyCapture2::FRAMERATE_120);
  framerates.push_back(FlyCapture2::FRAMERATE_240);

  std::vector<FlyCapture2::VideoMode>::const_iterator videomode;
  for(videomode=videomodes.begin(); videomode!=videomodes.end(); videomode++) {
    std::vector<FlyCapture2::FrameRate>::const_iterator framerate;
    for(framerate=framerates.begin(); framerate!=framerates.end(); framerate++) {
      err = cam->GetVideoModeAndFrameRateInfo(*videomode, *framerate, &supported);
      if (err == FlyCapture2::PGRERROR_OK) {
	if (supported) {
	  std::ostringstream oss = std::ostringstream();
	  oss << "videomode " << *videomode << " framerate " << *framerate;
	  mode.descr = oss.str();
	  mode.videomode = *videomode;
	  mode.framerate = *framerate;
	}
      }
    }
  }

  // err = cam->Disconnect();
  cam->Disconnect();
  return 0;


 // errlabel1:
  // err = cam->Disconnect();
 // errlabel2:
 //  return 1;
}

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

struct CCflycap; // forward declaration

// keep functable in sync across backends
typedef struct {
  cam_iface_constructor_func_t construct;
  void (*destruct)(struct CamContext*);

  void (*CCflycap)(struct CCflycap*,int,int,int);
  void (*close)(struct CCflycap*);
  void (*start_camera)(struct CCflycap*);
  void (*stop_camera)(struct CCflycap*);
  void (*get_num_camera_properties)(struct CCflycap*,int*);
  void (*get_camera_property_info)(struct CCflycap*,
                                   int,
                                   CameraPropertyInfo*);
  void (*get_camera_property)(struct CCflycap*,int,long*,int*);
  void (*set_camera_property)(struct CCflycap*,int,long,int);
  void (*grab_next_frame_blocking)(struct CCflycap*,
                                   unsigned char*,
                                   float);
  void (*grab_next_frame_blocking_with_stride)(struct CCflycap*,
                                               unsigned char*,
                                               intptr_t,
                                               float);
  void (*point_next_frame_blocking)(struct CCflycap*,unsigned char**,float);
  void (*unpoint_frame)(struct CCflycap*);
  void (*get_last_timestamp)(struct CCflycap*,double*);
  void (*get_last_framenumber)(struct CCflycap*,unsigned long*);
  void (*get_num_trigger_modes)(struct CCflycap*,int*);
  void (*get_trigger_mode_string)(struct CCflycap*,int,char*,int);
  void (*get_trigger_mode_number)(struct CCflycap*,int*);
  void (*set_trigger_mode_number)(struct CCflycap*,int);
  void (*get_frame_roi)(struct CCflycap*,int*,int*,int*,int*);
  void (*set_frame_roi)(struct CCflycap*,int,int,int,int);
  void (*get_max_frame_size)(struct CCflycap*,int*,int*);
  void (*get_buffer_size)(struct CCflycap*,int*);
  void (*get_framerate)(struct CCflycap*,float*);
  void (*set_framerate)(struct CCflycap*,float);
  void (*get_num_framebuffers)(struct CCflycap*,int*);
  void (*set_num_framebuffers)(struct CCflycap*,int);
} CCflycap_functable;

typedef struct CCflycap {
  CamContext inherited;
} CCflycap;


// forward declarations
CCflycap* CCflycap_construct( int device_number, int NumImageBuffers,
                              int mode_number);
void delete_CCflycap(struct CCflycap*);

void CCflycap_CCflycap(struct CCflycap*,int,int,int);
void CCflycap_close(struct CCflycap*);
void CCflycap_start_camera(struct CCflycap*);
void CCflycap_stop_camera(struct CCflycap*);
void CCflycap_get_num_camera_properties(struct CCflycap*,int*);
void CCflycap_get_camera_property_info(struct CCflycap*,
                              int,
                              CameraPropertyInfo*);
void CCflycap_get_camera_property(struct CCflycap*,int,long*,int*);
void CCflycap_set_camera_property(struct CCflycap*,int,long,int);
void CCflycap_grab_next_frame_blocking(struct CCflycap*,
                              unsigned char*,
                              float);
void CCflycap_grab_next_frame_blocking_with_stride(struct CCflycap*,
                                          unsigned char*,
                                          intptr_t,
                                          float);
void CCflycap_point_next_frame_blocking(struct CCflycap*,unsigned char**,float);
void CCflycap_unpoint_frame(struct CCflycap*);
void CCflycap_get_last_timestamp(struct CCflycap*,double*);
void CCflycap_get_last_framenumber(struct CCflycap*,unsigned long*);
void CCflycap_get_num_trigger_modes(struct CCflycap*,int*);
void CCflycap_get_trigger_mode_string(struct CCflycap*,int,char*,int);
void CCflycap_get_trigger_mode_number(struct CCflycap*,int*);
void CCflycap_set_trigger_mode_number(struct CCflycap*,int);
void CCflycap_get_frame_roi(struct CCflycap*,int*,int*,int*,int*);
void CCflycap_set_frame_roi(struct CCflycap*,int,int,int,int);
void CCflycap_get_max_frame_size(struct CCflycap*,int*,int*);
void CCflycap_get_buffer_size(struct CCflycap*,int*);
void CCflycap_get_framerate(struct CCflycap*,float*);
void CCflycap_set_framerate(struct CCflycap*,float);
void CCflycap_get_num_framebuffers(struct CCflycap*,int*);
void CCflycap_set_num_framebuffers(struct CCflycap*,int);

CCflycap_functable CCflycap_vmt = {
  (cam_iface_constructor_func_t)CCflycap_construct,
  (void (*)(CamContext*))delete_CCflycap,
  CCflycap_CCflycap,
  CCflycap_close,
  CCflycap_start_camera,
  CCflycap_stop_camera,
  CCflycap_get_num_camera_properties,
  CCflycap_get_camera_property_info,
  CCflycap_get_camera_property,
  CCflycap_set_camera_property,
  CCflycap_grab_next_frame_blocking,
  CCflycap_grab_next_frame_blocking_with_stride,
  CCflycap_point_next_frame_blocking,
  CCflycap_unpoint_frame,
  CCflycap_get_last_timestamp,
  CCflycap_get_last_framenumber,
  CCflycap_get_num_trigger_modes,
  CCflycap_get_trigger_mode_string,
  CCflycap_get_trigger_mode_number,
  CCflycap_set_trigger_mode_number,
  CCflycap_get_frame_roi,
  CCflycap_set_frame_roi,
  CCflycap_get_max_frame_size,
  CCflycap_get_buffer_size,
  CCflycap_get_framerate,
  CCflycap_set_framerate,
  CCflycap_get_num_framebuffers,
  CCflycap_set_num_framebuffers
};

/* globals -- allocate space */
  u_int64_t BACKEND_GLOBAL(prev_ts_uint64); //tmp

cam_iface_thread_local int BACKEND_GLOBAL(cam_iface_error)=0;
#define CAM_IFACE_MAX_ERROR_LEN 255
cam_iface_thread_local char BACKEND_GLOBAL(cam_iface_error_string)[CAM_IFACE_MAX_ERROR_LEN];
cam_iface_thread_local char BACKEND_GLOBAL(cam_iface_backend_string)[CAM_IFACE_MAX_ERROR_LEN];

typedef struct cam_iface_backend_extras cam_iface_backend_extras;
struct cam_iface_backend_extras {
  unsigned int buf_size; // current buffer size (number of bytes)
  unsigned int current_height;
  unsigned int current_width;
  unsigned int max_height;
  unsigned int max_width;
  double last_timestamp;
  unsigned long last_framecount;
  FlyCapture2::TriggerModeInfo trigger_mode_info;
};

#ifdef MEGA_BACKEND
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(pgr_flycapture_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,    \
                     "%s (%d): %s\n",__FILE__,__LINE__,(m));
#else
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN,    \
                     "%s (%d): %s\n",__FILE__,__LINE__,(m));
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_THROW_ERROR(m)                        \
  {                                                     \
    pgr_flycapture_cam_iface_error = CAM_IFACE_GENERIC_ERROR;	\
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return;                                             \
  }
#else
#define CAM_IFACE_THROW_ERROR(m)                        \
  {                                                     \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;		\
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return;                                             \
  }
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_THROW_ERRORV(m)                       \
  {                                                     \
    pgr_flycapture_cam_iface_error = CAM_IFACE_GENERIC_ERROR; \
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return NULL;					\
  }
#else
#define CAM_IFACE_THROW_ERRORV(m)                       \
  {                                                     \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;		\
    CAM_IFACE_ERROR_FORMAT((m));                        \
    return NULL;					\
  }
#endif

#define CHECK_CC(m)                                                     \
  if (!(m)) {                                                           \
    CAM_IFACE_THROW_ERROR("no CamContext specified (NULL argument)");   \
  }

#define NOT_IMPLEMENTED CAM_IFACE_THROW_ERROR("not yet implemented");

#ifdef MEGA_BACKEND
#define CAM_IFACE_CHECK_DEVICE_NUMBER(m)                                \
  if ( ((m)<0) | ((m)>=pgr_flycapture_num_cameras) ) {			\
    pgr_flycapture_cam_iface_error = CAM_IFACE_GENERIC_ERROR;		\
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return;                                                             \
  }
#else
#define CAM_IFACE_CHECK_DEVICE_NUMBER(m)                                \
  if ( ((m)<0) | ((m)>=num_cameras) ) {                                 \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;				\
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return;                                                             \
  }
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_CHECK_DEVICE_NUMBERV(m)                               \
  if ( ((m)<0) | ((m)>=pgr_flycapture_num_cameras) ) {			\
    pgr_flycapture_cam_iface_error = CAM_IFACE_GENERIC_ERROR;		\
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return NULL;                                                        \
  }
#else
#define CAM_IFACE_CHECK_DEVICE_NUMBERV(m)                               \
  if ( ((m)<0) | ((m)>=num_cameras) ) {                                 \
    cam_iface_error = CAM_IFACE_GENERIC_ERROR;				\
    CAM_IFACE_ERROR_FORMAT("invalid device_number");                    \
    return NULL;                                                        \
  }
#endif

#ifdef MEGA_BACKEND
#define CIPGRCHK(err) {							\
    FlyCapture2::Error _error_def = (err);				\
    if (_error_def.GetType()!=FlyCapture2::PGRERROR_OK) {		\
      pgr_flycapture_cam_iface_error = CAM_IFACE_GENERIC_ERROR;		\
      CAM_IFACE_ERROR_FORMAT("generic error");				\
      cam_iface_snprintf(pgr_flycapture_cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): PGR FlyCapture2 err %d: (%s)\n",	\
                         __FILE__,__LINE__,                             \
                         (int)_error_def.GetType(),			\
                         _error_def.GetDescription());			\
      return;								\
    }                                                                   \
}
#else
#define CIPGRCHK(err) {							\
    FlyCapture2::Error _error_def = (err);				\
    if (_error_def.GetType()!=FlyCapture2::PGRERROR_OK) {		\
      cam_iface_error = CAM_IFACE_GENERIC_ERROR;			\
      CAM_IFACE_ERROR_FORMAT("generic error");				\
      cam_iface_snprintf(cam_iface_error_string,CAM_IFACE_MAX_ERROR_LEN, \
                         "%s (%d): PGR FlyCapture2 err %d: (%s)\n",	\
                         __FILE__,__LINE__,                             \
                         (int)_error_def.GetType(),			\
                         _error_def.GetDescription());			\
      return;								\
    }                                                                   \
}
#endif

#ifdef MEGA_BACKEND
#define INTERNAL_CHK() {                                                \
    if (pgr_flycapture_cam_iface_error) {                                              \
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
    if (pgr_flycapture_cam_iface_error) {                                              \
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

#include "cam_iface_pgr_flycap.h"

const char *BACKEND_METHOD(cam_iface_get_driver_name)() {
  FlyCapture2::FC2Version fc2Version;
  FlyCapture2::Utilities::GetLibraryVersion( &fc2Version );
  cam_iface_snprintf(BACKEND_GLOBAL(cam_iface_backend_string),CAM_IFACE_MAX_ERROR_LEN,
	  "FlyCapture2 library version: %d.%d.%d.%d\n",
	  fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );
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
  FlyCapture2::Error error;
  unsigned int numCameras;

  BACKEND_GLOBAL(busMgr_ptr)  = new FlyCapture2::BusManager;
  error = BACKEND_GLOBAL(busMgr_ptr)->GetNumOfCameras(&numCameras);
  BACKEND_GLOBAL(num_cameras) = (int)numCameras;
}

void BACKEND_METHOD(cam_iface_shutdown)() {
  delete BACKEND_GLOBAL(busMgr_ptr);
}

int BACKEND_METHOD(cam_iface_get_num_cameras)() {
  return BACKEND_GLOBAL(num_cameras);
}

void BACKEND_METHOD(cam_iface_get_camera_info)(int device_number, Camwire_id *out_camid) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  if (out_camid==NULL) { CAM_IFACE_THROW_ERROR("return structure NULL"); }

  FlyCapture2::PGRGuid guid;
  CIPGRCHK( BACKEND_GLOBAL(busMgr_ptr)->GetCameraFromIndex(device_number, &guid));

  FlyCapture2::CameraInfo camInfo;
  FlyCapture2::Camera *cam = new FlyCapture2::Camera;
  CIPGRCHK(cam->Connect(&guid));
  CIPGRCHK(cam->GetCameraInfo(&camInfo));
  CIPGRCHK(cam->Disconnect());

  cam_iface_snprintf(out_camid->vendor, CAMWIRE_ID_MAX_CHARS, camInfo.vendorName);
  cam_iface_snprintf(out_camid->model, CAMWIRE_ID_MAX_CHARS, camInfo.modelName);
  cam_iface_snprintf(out_camid->chip, CAMWIRE_ID_MAX_CHARS, "GUID %x %x %x %x",
		     guid.value[0],
		     guid.value[1],
		     guid.value[2],
		     guid.value[3]);
}

void BACKEND_METHOD(cam_iface_get_num_modes)(int device_number, int *num_modes) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  std::vector<CamMode> result;
  int myerr = get_mode_list(device_number, result);
  if (myerr) {
    {CAM_IFACE_THROW_ERROR("problem getting mode list for camera"); }
  }
  *num_modes = result.size();
}

void BACKEND_METHOD(cam_iface_get_mode_string)(int device_number,
					       int mode_number,
					       char* mode_string,
					       int mode_string_maxlen) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  std::vector<CamMode> result;
  int myerr = get_mode_list(device_number, result);
  if (myerr) {
    {CAM_IFACE_THROW_ERROR("problem getting mode list for camera"); }
  }
  cam_iface_snprintf(mode_string, mode_string_maxlen, result[mode_number].descr.c_str());
}

cam_iface_constructor_func_t BACKEND_METHOD(cam_iface_get_constructor_func)(int device_number) {
  return (CamContext* (*)(int, int, int))CCflycap_construct;
}

CCflycap* CCflycap_construct( int device_number, int NumImageBuffers,
                                 int mode_number) {
  CCflycap *ccntxt = NULL;
  ccntxt = new CCflycap; // C++ equivalent to malloc
  memset(ccntxt,0,sizeof(CCflycap));
  CCflycap_CCflycap( ccntxt, device_number,NumImageBuffers,
                     mode_number);
  return ccntxt;
}

void CCflycap_CCflycap( CCflycap * ccntxt, int device_number, int NumImageBuffers,
                        int mode_number) {

  // call parent
  CamContext_CamContext((CamContext*)ccntxt,device_number,NumImageBuffers,mode_number); // XXX cast error?
  ccntxt->inherited.vmt = (CamContext_functable*)&CCflycap_vmt;

  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  std::vector<CamMode> result;
  int myerr = get_mode_list(device_number, result);

  ccntxt->inherited.device_number = device_number;
  ccntxt->inherited.backend_extras = new cam_iface_backend_extras;
  memset(ccntxt->inherited.backend_extras,0,sizeof(cam_iface_backend_extras));

  FlyCapture2::Camera *cam = new FlyCapture2::Camera;
  FlyCapture2::PGRGuid guid;
  CIPGRCHK( BACKEND_GLOBAL(busMgr_ptr)->GetCameraFromIndex(device_number, &guid));
  CIPGRCHK(cam->Connect(&guid));

  FlyCapture2::FC2Config cfg;
  CIPGRCHK(cam->GetConfiguration(&cfg));
  cfg.numBuffers = NumImageBuffers;
  CIPGRCHK(cam->SetConfiguration(&cfg));

  // Set the settings to the camera
  CamMode target_mode = result[mode_number];

  if (target_mode.videomode == FlyCapture2::VIDEOMODE_FORMAT7) {
    CIPGRCHK(cam->SetFormat7Configuration(&target_mode.fmt7ImageSettings,
					  target_mode.fmt7PacketInfo.recommendedBytesPerPacket ));
  } else {
    CIPGRCHK(cam->SetVideoModeAndFrameRate(target_mode.videomode, target_mode.framerate));
  }

  ccntxt->inherited.cam = (void*)cam;

  // XXX move this to start camera and query camera for settings

  CIPGRCHK(cam->StartCapture());


  // Retrieve an image to get width, height. XXX change to query later.
  FlyCapture2::Image rawImage;
  CIPGRCHK( cam->RetrieveBuffer( &rawImage ));

  cam_iface_backend_extras *extras = (cam_iface_backend_extras *)ccntxt->inherited.backend_extras;

  ccntxt->inherited.depth = rawImage.GetBitsPerPixel();
  extras->buf_size = rawImage.GetDataSize();
  extras->current_height = rawImage.GetRows();
  extras->current_width = rawImage.GetCols();
  extras->max_height = rawImage.GetRows();
  extras->max_width = rawImage.GetCols();
  CIPGRCHK( cam->GetTriggerModeInfo( &extras->trigger_mode_info ));

  switch (rawImage.GetPixelFormat()) {
  case FlyCapture2::PIXEL_FORMAT_MONO8:
    ccntxt->inherited.coding = CAM_IFACE_MONO8;
    if (rawImage.GetBayerTileFormat()!=FlyCapture2::NONE) {
      NOT_IMPLEMENTED;
    }
    break;
  case FlyCapture2::PIXEL_FORMAT_RAW8:
    switch (rawImage.GetBayerTileFormat()) {
    case FlyCapture2::NONE:
      ccntxt->inherited.coding = CAM_IFACE_RAW8;
      break;
    case FlyCapture2::RGGB:
      ccntxt->inherited.coding = CAM_IFACE_MONO8_BAYER_RGGB;
      break;
    case FlyCapture2::GRBG:
      ccntxt->inherited.coding = CAM_IFACE_MONO8_BAYER_GRBG;
      break;
    case FlyCapture2::GBRG:
      ccntxt->inherited.coding = CAM_IFACE_MONO8_BAYER_GBRG;
      break;
    case FlyCapture2::BGGR:
      ccntxt->inherited.coding = CAM_IFACE_MONO8_BAYER_BGGR;
      break;
    default:
      NOT_IMPLEMENTED;
    }
  }
  CIPGRCHK(cam->StopCapture());

}

void delete_CCflycap(CCflycap *ccntxt) {
  CCflycap_close(ccntxt);
  delete ccntxt;
  ccntxt = (CCflycap*)NULL;
}

void CCflycap_close(CCflycap *ccntxt) {
  if (!ccntxt) {CAM_IFACE_THROW_ERROR("no CCflycap specified (NULL argument)");}
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  CIPGRCHK(cam->StopCapture());
  CIPGRCHK(cam->Disconnect());

  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);


  if (backend_extras!=NULL) {
    delete backend_extras;
    ccntxt->inherited.backend_extras = (void*)NULL;
  }

  delete cam;
  ccntxt->inherited.cam = (void*)NULL;
}

void CCflycap_start_camera( CCflycap *ccntxt ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  CIPGRCHK(cam->StartCapture());
}

void CCflycap_stop_camera( CCflycap *ccntxt ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  CIPGRCHK(cam->StopCapture());
}

FlyCapture2::PropertyType propno2prop(int n) {
  FlyCapture2::PropertyType result;
  switch(n) {
  case 0: result  = FlyCapture2::BRIGHTNESS; break;
  case 1: result  = FlyCapture2::AUTO_EXPOSURE; break;
  case 2: result  = FlyCapture2::SHARPNESS; break;
  case 3: result  = FlyCapture2::WHITE_BALANCE; break;
  case 4: result  = FlyCapture2::HUE; break;
  case 5: result  = FlyCapture2::SATURATION; break;
  case 6: result  = FlyCapture2::GAMMA; break;
  case 7: result  = FlyCapture2::IRIS; break;
  case 8: result  = FlyCapture2::FOCUS; break;
  case 9: result  = FlyCapture2::ZOOM; break;
  case 10: result = FlyCapture2::PAN; break;
  case 11: result = FlyCapture2::TILT; break;
  case 12: result = FlyCapture2::SHUTTER; break;
  case 13: result = FlyCapture2::GAIN; break;
  case 14: result = FlyCapture2::TRIGGER_MODE; break;
  case 15: result = FlyCapture2::TRIGGER_DELAY; break;
  case 16: result = FlyCapture2::FRAME_RATE; break;
  case 17: result = FlyCapture2::TEMPERATURE; break;
  default:
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("property invalid");
    break;
  }
  return result;
}

#define NUM_CAM_PROPS 18

void CCflycap_get_num_camera_properties(CCflycap *ccntxt,
					int* num_properties) {
  CHECK_CC(ccntxt);
  *num_properties = NUM_CAM_PROPS;
}

void CCflycap_get_camera_property_info(CCflycap *ccntxt,
				       int property_number,
				       CameraPropertyInfo *info) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;

  if (info==NULL) {
    CAM_IFACE_THROW_ERROR("no info argument specified (NULL argument)");
  }

  if ((property_number<0) || (property_number>=NUM_CAM_PROPS)) {
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("property invalid");
  }

  FlyCapture2::Property prop;
  FlyCapture2::PropertyInfo propinfo;

  prop.type = propno2prop(property_number);
  propinfo.type = prop.type;

  switch(prop.type) {
  case FlyCapture2::BRIGHTNESS: info->name = "brightness"; break;
  case FlyCapture2::AUTO_EXPOSURE: info->name = "auto exposure"; break;
  case FlyCapture2::SHARPNESS: info->name = "sharpness"; break;
  case FlyCapture2::WHITE_BALANCE: info->name = "white balance"; break;
  case FlyCapture2::HUE: info->name = "hue"; break;
  case FlyCapture2::SATURATION: info->name = "saturation"; break;
  case FlyCapture2::GAMMA: info->name = "gamma"; break;
  case FlyCapture2::IRIS: info->name = "iris"; break;
  case FlyCapture2::FOCUS: info->name = "focus"; break;
  case FlyCapture2::ZOOM: info->name = "zoom"; break;
  case FlyCapture2::PAN: info->name = "pan"; break;
  case FlyCapture2::TILT: info->name = "tilt"; break;
  case FlyCapture2::SHUTTER: info->name = "shutter"; break;
  case FlyCapture2::GAIN: info->name = "gain"; break;
  case FlyCapture2::TRIGGER_MODE: info->name = "trigger mode"; break;
  case FlyCapture2::TRIGGER_DELAY: info->name = "trigger delay"; break;
  case FlyCapture2::FRAME_RATE: info->name = "frame rate"; break;
  case FlyCapture2::TEMPERATURE: info->name = "temperature"; break;
  }

  FlyCapture2::Error error = cam->GetProperty( &prop );
  if (error.GetType()==FlyCapture2::PGRERROR_PROPERTY_FAILED) {
    info->is_present = 0;
    return;
  }
  CIPGRCHK(error);

  CIPGRCHK(cam->GetPropertyInfo( &propinfo ));

  info->is_present = prop.present;

  info->min_value = propinfo.min;
  info->max_value = propinfo.max;

  info->has_auto_mode = propinfo.autoSupported;
  info->has_manual_mode = propinfo.manualSupported;

  info->is_scaled_quantity = 0;

  info->original_value = 0; // XXX FIXME

  info->available = prop.present;
  info->readout_capable = propinfo.readOutSupported;
  info->on_off_capable = propinfo.onOffSupported;

  info->absolute_capable = propinfo.absValSupported;
  info->absolute_control_mode = 0;
  info->absolute_min_value = propinfo.absMin;
  info->absolute_max_value = propinfo.absMax;

  return;
}

void CCflycap_get_camera_property(CCflycap *ccntxt,
				  int property_number,
				  long* Value,
				  int* Auto ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;

  if ((property_number<0) || (property_number>=NUM_CAM_PROPS)) {
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("property invalid");
  }

  FlyCapture2::Property prop;
  FlyCapture2::PropertyInfo propinfo;
  prop.type = propno2prop(property_number);
  CIPGRCHK(cam->GetProperty( &prop ));

  *Value = prop.valueA;
  *Auto = prop.autoManualMode;
}

void CCflycap_set_camera_property(CCflycap *ccntxt,
				  int property_number,
				  long Value,
				  int Auto ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;

  if ((property_number<0) || (property_number>=NUM_CAM_PROPS)) {
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("property invalid");
  }

  FlyCapture2::Property prop;
  FlyCapture2::PropertyInfo propinfo;
  prop.type = propno2prop(property_number);
  CIPGRCHK(cam->GetProperty( &prop ));

  prop.valueA = Value;
  prop.autoManualMode = Auto;
  CIPGRCHK(cam->SetProperty( &prop ));
  return;
}

void CCflycap_grab_next_frame_blocking( CCflycap *ccntxt, unsigned char *out_bytes, float timeout) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  CCflycap_grab_next_frame_blocking_with_stride(ccntxt,out_bytes,
						backend_extras->current_width,
						timeout);
}

void CCflycap_grab_next_frame_blocking_with_stride( CCflycap *ccntxt,
						    unsigned char *out_bytes,
						    intptr_t stride0,
						    float timeout ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  // The main frame grabbing code goes here.

  FlyCapture2::Image rawImage;
  CIPGRCHK(cam->RetrieveBuffer( &rawImage ));

  if (stride0 < (intptr_t)rawImage.GetStride()) {
    CAM_IFACE_THROW_ERROR("stride too small for image");
  }

  if (stride0==(intptr_t)rawImage.GetStride()) {
    // same stride
    memcpy((void*)out_bytes, /*dest*/
	   (const void*)rawImage.GetData(),/*src*/
	   rawImage.GetDataSize());/*src*/

  } else {
    // different strides
    for (int row=0; row<(int)rawImage.GetRows(); row++) {
      memcpy((void*)(out_bytes+row*stride0), /*dest*/
	     (const void*)(rawImage.GetData() + row*rawImage.GetStride()),/*src*/
	     rawImage.GetStride());/*size*/
    }
  }
}

void CCflycap_point_next_frame_blocking( CCflycap *ccntxt, unsigned char **buf_ptr,
                                           float timeout){
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}
void CCflycap_unpoint_frame( CCflycap *ccntxt){
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}

void CCflycap_get_last_timestamp( CCflycap *ccntxt, double* timestamp ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *timestamp = backend_extras->last_timestamp;
}

void CCflycap_get_last_framenumber( CCflycap *ccntxt, unsigned long* framenumber ){
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *framenumber = backend_extras->last_framecount;
}

void CCflycap_get_num_trigger_modes( CCflycap *ccntxt,
				     int *num_exposure_modes ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *num_exposure_modes = 1;

  if (backend_extras->trigger_mode_info.present) {
    if (backend_extras->trigger_mode_info.onOffSupported) {
      if (backend_extras->trigger_mode_info.polaritySupported) {
	*num_exposure_modes = 3;
      } else {
	*num_exposure_modes = 2;
      }
    }
  }
}

void CCflycap_get_trigger_mode_string( CCflycap *ccntxt,
				       int exposure_mode_number,
				       char* exposure_mode_string, //output parameter
				       int exposure_mode_string_maxlen) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  switch (exposure_mode_number) {
  case 0:
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"free running");
    break;
  case 1:
    if (!((backend_extras->trigger_mode_info.present) &&
	  (backend_extras->trigger_mode_info.onOffSupported))) {
      BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
      CAM_IFACE_ERROR_FORMAT("exposure_mode_number invalid");
      return;
    }

    if (backend_extras->trigger_mode_info.polaritySupported) {
      cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"externally triggered (positive polarity)");
    } else {
      cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"externally triggered");
    }
    break;
  case 2:
    if (!((backend_extras->trigger_mode_info.present) &&
	  (backend_extras->trigger_mode_info.onOffSupported) &&
	  (backend_extras->trigger_mode_info.polaritySupported))) {
      BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
      CAM_IFACE_ERROR_FORMAT("exposure_mode_number invalid");
      return;
    }
    cam_iface_snprintf(exposure_mode_string,exposure_mode_string_maxlen,"externally triggered (negative polarity)");
    break;
  default:
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("exposure_mode_number invalid");
    return;
  }
}

void CCflycap_get_trigger_mode_number( CCflycap *ccntxt,
				       int *exposure_mode_number ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  FlyCapture2::TriggerMode trigger_mode;

  if (!(backend_extras->trigger_mode_info.present)) {
    *exposure_mode_number = 0;
    return;
  }

  CIPGRCHK(((FlyCapture2::Camera *)(ccntxt->inherited.cam))->GetTriggerMode( &trigger_mode ));

  if (!(trigger_mode.onOff)) {
    *exposure_mode_number = 0;
    return;
  }

  if (backend_extras->trigger_mode_info.polaritySupported) {
    if (trigger_mode.polarity) {
      *exposure_mode_number = 1;
      return;
    } else {
      *exposure_mode_number = 2;
      return;
    }
  } else {
    *exposure_mode_number = 1;
    return;
  }

}

void CCflycap_set_trigger_mode_number( CCflycap *ccntxt,
				       int exposure_mode_number ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  FlyCapture2::TriggerMode trigger_mode;

  switch (exposure_mode_number) {
  case 0:
    trigger_mode.onOff=0;
    break;
  case 1:
    trigger_mode.onOff=1;
    trigger_mode.polarity=1;
    break;
  case 2:
    trigger_mode.onOff=1;
    trigger_mode.polarity=0;
    break;
  default:
    BACKEND_GLOBAL(cam_iface_error) = CAM_IFACE_GENERIC_ERROR;
    CAM_IFACE_ERROR_FORMAT("exposure_mode_number invalid");
    return;
  }

  if (((backend_extras->trigger_mode_info.present) &&
       (backend_extras->trigger_mode_info.onOffSupported))) {
    CIPGRCHK(((FlyCapture2::Camera *)(ccntxt->inherited.cam))->SetTriggerMode( &trigger_mode ));
  }

}

void CCflycap_get_frame_roi( CCflycap *ccntxt,
                             int *left, int *top, int* width, int* height ) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);

  *left = 0;
  *top = 0;
  *width = (int)backend_extras->current_width;
  *height = (int)backend_extras->current_height;

}

void CCflycap_set_frame_roi( CCflycap *ccntxt,
                             int left, int top, int width, int height ) {
  CHECK_CC(ccntxt);
  NOT_IMPLEMENTED;
}

void CCflycap_get_buffer_size( CCflycap *ccntxt,
			       int *size) {
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *size = backend_extras->buf_size;
}

void CCflycap_get_framerate( CCflycap *ccntxt,
			     float *framerate ) {
  CHECK_CC(ccntxt);
  *framerate = 0.0f;
}

void CCflycap_set_framerate( CCflycap *ccntxt,
			     float framerate ) {
  CHECK_CC(ccntxt);
  CAM_IFACE_THROW_ERROR("frame rate is not settable");
  NOT_IMPLEMENTED;
}

void CCflycap_get_max_frame_size( CCflycap *ccntxt,
				  int *width, int *height ){
  CHECK_CC(ccntxt);
  cam_iface_backend_extras* backend_extras = (cam_iface_backend_extras*)(ccntxt->inherited.backend_extras);
  *width = backend_extras->max_width;
  *height = backend_extras->max_height;
}

void CCflycap_get_num_framebuffers( CCflycap *ccntxt,
				    int *num_framebuffers ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  FlyCapture2::FC2Config cfg;
  CIPGRCHK(cam->GetConfiguration(&cfg));
  *num_framebuffers = cfg.numBuffers;
}

void CCflycap_set_num_framebuffers( CCflycap *ccntxt,
				    int num_framebuffers ) {
  CHECK_CC(ccntxt);
  FlyCapture2::Camera *cam = (FlyCapture2::Camera *)ccntxt->inherited.cam;
  FlyCapture2::FC2Config cfg;
  CIPGRCHK(cam->StopCapture());
  CIPGRCHK(cam->GetConfiguration(&cfg));
  cfg.numBuffers = num_framebuffers;
  CIPGRCHK(cam->SetConfiguration(&cfg));
  CIPGRCHK(cam->StartCapture());
}

} // closes: extern "C"
