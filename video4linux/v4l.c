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
static int nbuffers = 1;
static void *buffers[MAX_BUFFERS];
static int sizes[MAX_BUFFERS];
static int started = 0;
static int fd;

// geometry
static int width = 640;
static int height = 480;  
static int height1 = -1; // height returned by camera
static int width1 = -1; // width returned by camera 
static int fps = 30;


// camera control
static void set_boolean_control(int id, int val) {
  struct v4l2_control control;

  memset (&control, 0, sizeof (control));
  control.id = id;
  if (0 == ioctl (fd, VIDIOC_G_CTRL, &control)) {
    control.value += 1;
    /* The driver may clamp the value or return ERANGE, ignored here */
    if (-1 == ioctl (fd, VIDIOC_S_CTRL, &control)
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
  ioctl (fd, VIDIOC_S_CTRL, &control);
}


// frame grabber
static int l_grabFrame (lua_State *L) {
  // Get Tensor's Info
  THDoubleTensor * frame = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.DoubleTensor"));

  // device
  const char *device = "/dev/video0";
  if (lua_isstring(L, 2)) device = lua_tostring(L, 2);

  // nb of buffers
  if (lua_isnumber(L, 3)) nbuffers = lua_tonumber(L, 3);

  // grabbing speed
  if (lua_isnumber(L, 4)) fps = lua_tonumber(L, 4);

  // get camera
  if(!started) {
    fd = open(device, O_RDWR);
    if (fd == -1)
      perror("could not open v4l2 device");
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    memset((void*) &cap, 0, sizeof(struct v4l2_capability));
    int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
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
    if (0 == ioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
      crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.c = cropcap.defrect; 
      ioctl(fd, VIDIOC_S_CROP, &crop);
    }
    // set format
    memset((void*) &fmt, 0, sizeof(struct v4l2_format));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // TODO: error when ratio not correct
    fmt.fmt.pix.width       = width; 
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
      // (==> this cleanup)
      perror("unable to set v4l2 format");
    }
    height1 = fmt.fmt.pix.height;
    width1 = fmt.fmt.pix.width;

    if (height != height1 || width != width1)
      printf("Warning: camera resolution changed to %dx%d\n", height1, width1);

    // set framerate
    struct v4l2_streamparm setfps;  
    memset((void*) &setfps, 0, sizeof(struct v4l2_streamparm));
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.numerator = 1;
    setfps.parm.capture.timeperframe.denominator = fps;
    ioctl(fd, VIDIOC_S_PARM, &setfps); 
    // allocate and map the buffers
    struct v4l2_requestbuffers rb;
    rb.count = nbuffers;
    rb.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
      // (==> this cleanup)
      perror("could not allocate v4l2 buffers");
    }
    ret = 0;
    int i;
    for (i = 0; i < nbuffers; i++) { 
      struct v4l2_buffer buf;
      int r;
      memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
      buf.index = i;
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      r = ioctl(fd, VIDIOC_QUERYBUF, &buf);
      //       printf("i=%u, length: %u, offset: %u, r=%d\n", i, buf.length, buf.m.offset, r); 
      if (r < 0)
	ret = -(i+1);
      if (ret == 0) {
	buffers[i] = mmap(0, buf.length, PROT_READ + PROT_WRITE, MAP_SHARED,
			  fd, buf.m.offset);
	sizes[i] = buf.length;
	if (buffers[i] == MAP_FAILED)
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

    set_boolean_control(V4L2_CID_AUTOGAIN, 1);
    set_boolean_control(V4L2_CID_AUTO_WHITE_BALANCE, 1);

    // start capturing
    ret = 0;
    for (i = 0; i < nbuffers; ++i) {
      struct v4l2_buffer buf;
      memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = i;
      ret += ioctl(fd, VIDIOC_QBUF, &buf);
    }
    if (ret < 0)
      printf("WARNING: could not enqueue v4l2 buffers");
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
      printf("WARNING: could not start v4l2 capture");
      started = 0;
    } else 
      started = 1;
  }

  // resize given tensor
  THDoubleTensor_resize3d(frame, 3, height1, width1);

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
  ret = ioctl(fd, VIDIOC_DQBUF, &buf);
  src = (unsigned char *)(buffers[buf.index]);
  for (i=0; i < height1; i++) {
    for (j=0, k=0; j < width1; j++, k+=m1) {
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
    src += width1 << 1;
    dst += m0;
  }
  ret += ioctl(fd, VIDIOC_QBUF, &buf);
  
  return 0;
}



static int l_releaseCam (lua_State *L) {
  return 0;
}


// Register functions in LUA
static const struct luaL_reg v4l [] = {
  {"grabFrame", l_grabFrame},
  {"releaseCam", l_releaseCam},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libv4l (lua_State *L) {
  luaL_openlib(L, "libv4l", v4l, 0);
  return 1; 
}
