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

static CvCapture* capture;
static IplImage* frame;

static int l_initCam(lua_State *L) {
    const int idx = lua_tonumber(L, 1);
    capture = cvCreateCameraCapture(idx);
    if( capture == NULL ) {
      perror("could not create OpenCV capture");
    }
    sleep(2);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
    frame = cvQueryFrame ( capture );
    if ( frame == NULL ) {
      perror("failed OpenCV test capture");
    }
    // cvSaveImage("opencv.jpg",frame,0); 
    if(frame->depth != IPL_DEPTH_8U) {
      perror("initCam: opencv supported for 8-bit unsigned capture only");
    }
    printf("camera initialized\n");
    return 0;

}

// frame grabber
static int l_grabFrame (lua_State *L) {
  // Get Tensor's Info
  THDoubleTensor * tensor = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.DoubleTensor"));

  // grab frame
  frame = cvQueryFrame ( capture );
  if( !frame ) {
    perror("could not query OpenCV capture");
  }
  
  // resize given tensor
  THDoubleTensor_resize3d(tensor, 3, frame->height, frame->width);

  // copy to tensor
  int m0 = tensor->stride[1];
  int m1 = tensor->stride[2];
  int m2 = tensor->stride[0];
  unsigned char *src = frame->imageData;
  double *dst = THDoubleTensor_data(tensor);
  int i, j, k;
  for (i=0; i < frame->height; i++) {
    for (j=0, k=0; j < frame->width; j++, k+=m1) {
      //printf("(%i,%i) ",i,j);
      // red:
      dst[k] = src[i*frame->widthStep + j*frame->nChannels + 2]/255.;
      //src[i*frame->widthStep + j*frame->nChannels + 2]/255.;
      //printf("r ");
      // green:
      dst[k+m2] = src[i*frame->widthStep + j*frame->nChannels + 1]/255.;
      //src[i*frame->widthStep + j*frame->nChannels + 1]/255.;
      //printf("g ");
      // blue:
      dst[k+2*m2] = src[i*frame->widthStep + j*frame->nChannels + 0]/255.;
      //src[i*frame->widthStep + j*frame->nChannels + 0]/255.;
      //printf("b\n");
    }
    dst += m0;
  }

  return 0;
}

static int l_releaseCam (lua_State *L) {
  cvReleaseCapture( &capture );
  return 0;
}

// Register functions
static const struct luaL_reg opencv [] = {
  {"initCam", l_initCam},
  {"grabFrame", l_grabFrame},
  {"releaseCam", l_releaseCam},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libopencv (lua_State *L) {
  luaL_openlib(L, "libopencv", opencv, 0);
  return 1; 
}
