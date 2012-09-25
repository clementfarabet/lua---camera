//==============================================================================
// File: opencv
//
// Description: A wrapper for opencv camera frame grabber
//
// Created: November 8, 2011, 4:08pm
//
// Author: Jordan Bates // jtbates@gmail.com
//         Using code from Clement Farabet's wrappers for camiface and v4l
//==============================================================================

#include <luaT.h>
#include <TH.h>

#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>

#include <cv.h>
#include <highgui.h>

#define MAXIDX 100
static CvCapture* capture[MAXIDX];
static IplImage* frame[MAXIDX];
static fidx = 0;

static int l_initCam(lua_State *L) {
  // args
  int width = lua_tonumber(L, 2);
  int height = lua_tonumber(L, 3);

  // max allocs ?
  if (fidx == MAXIDX) {
    perror("max nb of devices reached...\n");
  }

  // if number, open a camera device
  if (lua_isnumber(L, 1)) {
    printf("initializing camera\n");
    const int idx = lua_tonumber(L, 1);
    capture[fidx] = cvCaptureFromCAM(idx);
    if( capture[fidx] == NULL ) {
      perror("could not create OpenCV capture");
    }
    //    sleep(2);
    //cvSetCaptureProperty(capture[fidx], CV_CAP_PROP_FRAME_WIDTH, width);
    //cvSetCaptureProperty(capture[fidx], CV_CAP_PROP_FRAME_HEIGHT, height);
    frame[fidx] = cvQueryFrame ( capture[fidx] );
    int tries = 10;
    while ( !frame[fidx] && tries>0) {
      frame[fidx] = cvQueryFrame ( capture[fidx] );
      tries--;
      sleep(1);
    }
    if ( frame[fidx] == NULL ) {
      perror("failed OpenCV test capture");
    }
    if(frame[fidx]->depth != IPL_DEPTH_8U) {
      perror("initCam: opencv supported for 8-bit unsigned capture only");
    }
    printf("camera initialized\n");
  } else {
    // open a file;
    const char *file = lua_tostring(L, 1);
    printf("initializing frame grabber on file: %s\n", file);
    capture[fidx] = cvCreateFileCapture(file);
    if( capture[fidx] == NULL ) {
      perror("could not create OpenCV capture");
    }
  }

  // next
  lua_pushnumber(L, fidx);
  fidx ++;
  return 1;
}

// frame grabber
static int l_grabFrame (lua_State *L) {
  // Get Tensor's Info
  const int idx = lua_tonumber(L, 1);
  THFloatTensor * tensor = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.FloatTensor"));

  // grab frame
  frame[idx] = cvQueryFrame ( capture[idx] );
  if( !frame[idx] ) {
    perror("could not query OpenCV capture");
  }

  // resize given tensor
  THFloatTensor_resize3d(tensor, 3, frame[idx]->height, frame[idx]->width);

  // copy to tensor
  int m0 = tensor->stride[1];
  int m1 = tensor->stride[2];
  int m2 = tensor->stride[0];
  unsigned char *src = frame[idx]->imageData;
  float *dst = THFloatTensor_data(tensor);
  int i, j, k;
  for (i=0; i < frame[idx]->height; i++) {
    for (j=0, k=0; j < frame[idx]->width; j++, k+=m1) {
      // red:
      dst[k] = src[i*frame[idx]->widthStep + j*frame[idx]->nChannels + 2]/255.;
      // green:
      dst[k+m2] = src[i*frame[idx]->widthStep + j*frame[idx]->nChannels + 1]/255.;
      // blue:
      dst[k+2*m2] = src[i*frame[idx]->widthStep + j*frame[idx]->nChannels + 0]/255.;
    }
    dst += m0;
  }

  return 0;
}

static int l_releaseCam (lua_State *L) {
  const int idx = lua_tonumber(L, 1);
  cvReleaseCapture( &capture[idx] );
  return 0;
}

// Register functions
static const struct luaL_reg opencv [] = {
  {"initCam", l_initCam},
  {"grabFrame", l_grabFrame},
  {"releaseCam", l_releaseCam},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libcamopencv (lua_State *L) {
  luaL_openlib(L, "libcamopencv", opencv, 0);
  return 1;
}
