//==============================================================================
// File: v4l
//
// Description: A wrapper for a video4linux camera frame grabber
//
// Created: September 27, 2010, 10:20PM
//
// Author: Clement Farabet // clement.farabet@gmail.com
//         largely taken from Pierre Sermanet's EBLearn toolbox/
//         [BSD licensed, http://eblearn.sourceforge.net/]
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
#include <sys/time.h>
#include <sys/mman.h>
#include <signal.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

// video buffers
#define MAX_BUFFERS 8
#define MAX_CAMERAS 8

typedef struct
{
  int nbuffers;
  char device[80];
  void *buffers[MAX_BUFFERS];
  int sizes[MAX_BUFFERS];
  int started;
  int fd;
  int width;
  int width1;
  int height;
  int height1;
  int fps;
} Cam;

static Cam Cameras[MAX_CAMERAS];
static int camidx = 0;

// camera control
static void set_boolean_control(Cam *camera, int id, int val) {
  struct v4l2_control control;

  memset (&control, 0, sizeof (control));
  control.id = id;
  if (0 == ioctl (camera->fd, VIDIOC_G_CTRL, &control)) {
    control.value += 1;
    /* The driver may clamp the value or return ERANGE, ignored here */
    if (-1 == ioctl (camera->fd, VIDIOC_S_CTRL, &control)
        && errno != ERANGE) {
      perror("VIDIOC_S_CTRL");
    }
    /* Ignore if V4L2_CID_CONTRAST is unsupported */
  } else if (errno != EINVAL) {
    perror("VIDIOC_G_CTRL");
    exit (EXIT_FAILURE);
    }
  control.id = id;
  control.value = val;
  /* Errors ignored */
  ioctl (camera->fd, VIDIOC_S_CTRL, &control);
}


// frame grabber
static int l_init (lua_State *L) {
  Cam * camera;
  // device
  int camid = 0;
  if (lua_isnumber(L, 1)) camid = lua_tonumber(L, 1);
  camera = &Cameras[camid];
  sprintf(camera->device,"/dev/video%d",camid);
  printf("Initializing device: %s\n", camera->device);
  
  // width at which to grab 
  int width    = 640;
  if (lua_isnumber(L, 2)) width = lua_tonumber(L, 2);
  // height at which to grab
  int height   = 480;
  if (lua_isnumber(L, 3)) height = lua_tonumber(L, 3);
  // grabbing speed
  int fps      = 1;
  if (lua_isnumber(L, 4)) fps = lua_tonumber(L, 4);
  printf("FPS wanted %d \n", fps);
  // nb of buffers
  int nbuffers =  1;
  if (lua_isnumber(L, 5)) nbuffers = lua_tonumber(L, 5);
  printf("Using %d buffers\n", nbuffers);
  // get camera
  camera->fd = open(camera->device, O_RDWR);
  if (camera->fd == -1)
    perror("could not open v4l2 device");
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  memset((void*) &cap, 0, sizeof(struct v4l2_capability));
  int ret = ioctl(camera->fd, VIDIOC_QUERYCAP, &cap);
  if (ret < 0) {
    //      (==> this cleanup)
    perror("could not query v4l2 device");
  }
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    // (==> this cleanup)
    perror("v4l2 device does not support video capture");
  }
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    // (==> this cleanup)
    perror("v4l2 device does not support streaming i/o");
  }
  // resetting cropping to full frame
  memset((void*) &cropcap, 0, sizeof(struct v4l2_cropcap));
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 == ioctl(camera->fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; 
    ioctl(camera->fd, VIDIOC_S_CROP, &crop);
  }
  // set format
  memset((void*) &fmt, 0, sizeof(struct v4l2_format));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // TODO: error when ratio not correct
  fmt.fmt.pix.width       = width; 
  fmt.fmt.pix.height      = height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  if (ioctl(camera->fd, VIDIOC_S_FMT, &fmt) < 0) {
    // (==> this cleanup)
    perror("unable to set v4l2 format");
  }
  camera->height   = height;
  camera->width    = width;
  camera->height1  = fmt.fmt.pix.height;
  camera->width1   = fmt.fmt.pix.width;
  camera->nbuffers = nbuffers;
  camera->fps      = fps;
  
  if (camera->height != camera->height1 ||
      camera->width != camera->width1)
    printf("Warning: camera resolution changed to %dx%d\n",
           camera->height1, camera->width1);
  
  // set framerate
  struct v4l2_streamparm setfps;  
  memset((void*) &setfps, 0, sizeof(struct v4l2_streamparm));
  setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps.parm.capture.timeperframe.numerator = 1;
  setfps.parm.capture.timeperframe.denominator = camera->fps;
  ioctl(camera->fd, VIDIOC_S_PARM, &setfps); 
  // allocate and map the buffers
  struct v4l2_requestbuffers rb;
  rb.count = camera->nbuffers;
  rb.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
  rb.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(camera->fd, VIDIOC_REQBUFS, &rb);
  if (ret < 0) {
    // (==> this cleanup)
    perror("could not allocate v4l2 buffers");
  }
  ret = 0;
  int i;
  for (i = 0; i < camera->nbuffers; i++) { 
    struct v4l2_buffer buf;
    int r;
    memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
    buf.index = i;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    r = ioctl(camera->fd, VIDIOC_QUERYBUF, &buf);
    if (r < 0)
      ret = -(i+1);
    if (ret == 0) {
      camera->buffers[i] =
        mmap(0, buf.length, PROT_READ + PROT_WRITE, MAP_SHARED,
             camera->fd, buf.m.offset);
      camera->sizes[i] = buf.length;
      if (camera->buffers[i] == MAP_FAILED)
        ret = -(i+1000);
    }
  }
  if (ret < 0) {
    printf("ret = %d\n", ret);
    if (ret > -1000) {
      printf("query buffer %d\n", - (1 + ret));
      perror("could not query v4l2 buffer");
    } else {
      printf("map buffer %d\n", - (1000 + ret));
      perror("could not map v4l2 buffer");
    }
  }
  
  set_boolean_control(camera,V4L2_CID_AUTOGAIN, 1);
  set_boolean_control(camera,V4L2_CID_AUTO_WHITE_BALANCE, 1);
  
  // start capturing
  ret = 0;
  for (i = 0; i < camera->nbuffers; ++i) {
    struct v4l2_buffer buf;
      memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = i;
      ret += ioctl(camera->fd, VIDIOC_QBUF, &buf);
  }
  if (ret < 0)
    printf("WARNING: could not enqueue v4l2 buffers: errno %d\n", ret);
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(camera->fd, VIDIOC_STREAMON, &type);
  if (ret < 0) {
    printf("WARNING: could not start v4l2 capture: errno %d\n", ret);
    camera->started = 0;
      return -1;
  }
  camera->started = 1;
  printf("camera[%d] started : %d\n",camid,camera->started);
  return 0;
}
  
// frame grabber
static int l_grabFrame (lua_State *L) {
  // device
  Cam *camera;
  int camid = 0;
  if (lua_isnumber(L, 1)) camid = lua_tonumber(L, 1);
  camera = &Cameras[camid];
  if (camera->started != 1){
    printf("Camera not open at this index\n");
    return -1;
  }
  // Get Tensor's Info
  THDoubleTensor * frame =
    luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.DoubleTensor"));
  
  // resize given tensor
  THDoubleTensor_resize3d(frame, 3,
                          camera->height1, camera->width1);

  // grab frame
  int ret = 0;
  int m0 = frame->stride[1];
  int m1 = frame->stride[2];
  int m2 = frame->stride[0];
  struct v4l2_buffer buf;
  unsigned char *src;
  double *dst = THDoubleTensor_data(frame);
  int i, j, k;
  memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(camera->fd, VIDIOC_DQBUF, &buf);
  src = (unsigned char *)(camera->buffers[buf.index]);
  for (i=0; i < camera->height1; i++) {
    for (j=0, k=0; j < camera->width1; j++, k+=m1) {
      double f,y,u,v;
      int j2;
      j2 = j<<1;
      
      // get Y,U,V chanels from bayer pattern
      y = (double)src[j2]; 
      if (j & 1) {
        u = (double)src[j2-1]; v = (double)src[j2+1];
      } else {
        u = (double)src[j2+1]; v = (double)src[j2+3];
      }

      // convert to RGB
      // red:
      dst[k] = (1.164383*(y-16) + 1.596027*(v-128))/255.0;
      // green:
      dst[k+m2] = (1.164383*(y-16) - 0.812968*(v-128) - 0.391726*(u-128))/255.0;
      // blue:
      dst[k+2*m2] = (1.164383*(y-16) + 2.017232*(u-128))/255.0;
    }
    src += camera->width1 << 1;
    dst += m0;
  }
  ret += ioctl(camera->fd, VIDIOC_QBUF, &buf);
  
  return 0;
}



static int l_releaseCam (lua_State *L) {
  return 0;
}


// Register functions in LUA
static const struct luaL_reg v4l [] = {
  {"init", l_init},
  {"grabFrame", l_grabFrame},
  {"releaseCam", l_releaseCam},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libv4l (lua_State *L) {
  luaL_openlib(L, "libv4l", v4l, 0);
  return 1; 
}
