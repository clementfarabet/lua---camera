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
#include <highgui.c>

static CvCapture* capture;
static IplImage* frame;
static int height;
static int width;

static int l_initCam(lua_State *L) {
    const int idx = luaL_tonumber(L, 1);
    capture = cvCreateCameraCapture(idx);
    if( capture == NULL ) {
      perror("could not create OpenCV capture");
    }
    height = (int) cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT);
    width = (int) cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH);
    printf("cap height %i\n", height);
    printf("cap width %i\n", width);
    frame = cvQueryFrame( capture );
    printf("frame height %i\n", frame->height);
    printf("frame width %i\n", frame->width);
    printf("frame depth %i\n", frame->depth);
    
    return 0;
}

// frame grabber
static int l_grabFrame (lua_State *L) {
  // Get Tensor's Info
  THDoubleTensor * tensor = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.DoubleTensor"));

  frame = cvQueryFrame ( capture );
  if( !frame ) {
    perror("could not query OpenCV capture");
  }

  // resize given tensor
  THDoubleTensor_resize3d(tensor, 3, height, width);

  // grab frame
  int m0 = tensor->stride[1];
  int m1 = tensor->stride[2];
  int m2 = tensor->stride[0];
  unsigned char *src;
  double *dst = THDoubleTensor_data(tensor);
  int i, j, k;
  for (i=0; i < height; i++) {
    for (j=0, k=0; j < width; j++, k+=m1) {
    }
  }

  return 0;
}

static int l_releaseCam (lua_State *L) {
  cvReleaseCapture( &capture );
  return 0;
}

// Register functions
static const struct luaL_reg camiface [] = {
  {"initCam", l_initCam},
  {"grabFrame", l_grabFrame},
  {"releaseCam", l_releaseCam},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libopencv (lua_State *L) {
  luaL_openlib(L, "libopencv", opencv, 0);
  return 1; 
}
