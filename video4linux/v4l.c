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


static int xioctl(int fd, int request, void *arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}


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


static int l_set_focus_type(lua_State *L){
    // device
    Cam *camera;
    int camid = 0;
    int val = 0;
    if (lua_isnumber(L, 1)) camid = lua_tonumber(L, 1);
    camera = &Cameras[camid];
    if (camera->started != 1){
        printf("Camera not open at this index\n");
        return -1;
    }
    if (lua_isnumber(L, 2)) val = lua_tonumber(L,2);
    /* val can be :
     * enum  v4l2_focus_auto_type {
     *  V4L2_FOCUS_MANUAL     = 0,
     *  V4L2_FOCUS_AUTO       = 1,
     *  V4L2_FOCUS_MACRO      = 2,
     *  V4L2_FOCUS_CONTINUOUS = 3
     *  }
     */
    if ((val >= 0) && (val <= 4)){
        set_boolean_control(camera,V4L2_CID_FOCUS_AUTO, val);
    } else {
        printf("Bad value for setFocusType\n");
    }
    return 0;
}

static int l_adjust_manual_focus(lua_State *L){
    // device
    Cam *camera;
    int camid = 0;
    int val = 0;
    if (lua_isnumber(L, 1)) camid = lua_tonumber(L, 1);
    camera = &Cameras[camid];
    if (camera->started != 1){
        printf("Camera not open at this index\n");
        return -1;
    }
    if (lua_isnumber(L, 2)) val = lua_tonumber(L,2);
    /* set to manual focus */
    set_boolean_control(camera,V4L2_CID_FOCUS_AUTO, 0);
    set_boolean_control(camera,V4L2_CID_FOCUS_ABSOLUTE, val);
    return 0;
}


static int open_device(int camid, char *dev_name)
{
    Cam * camera = &Cameras[camid];
    struct stat st;

    if (-1 == stat (dev_name, &st)) {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n",
           dev_name, errno, strerror (errno));
        return -1;
    }

    if (!S_ISCHR (st.st_mode)) {
        fprintf (stderr, "%s is no device\n", dev_name);
        return -1;
    }

    camera->fd = open(dev_name, O_RDWR);

    if (-1 == camera->fd) {
        fprintf (stderr, "Cannot open '%s': %d, %s\n",
           dev_name, errno, strerror (errno));
        return -1;
    }

    return 0;
}


static int init_capability(int camid)
{
    Cam * camera = &Cameras[camid];

    struct v4l2_capability cap;
    memset((void*) &cap, 0, sizeof(struct v4l2_capability));

    if (-1 == xioctl (camera->fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            perror("no V4L2 device found");
            return -1;
        } else {
            return -1;
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        perror("v4l2 device does not support video capture");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        perror("v4l2 device does not support streaming i/o");
        return -1;
    }

    return 0;
}


static int init_format(int camid, int width, int height, int fps, int nbuffers)
{
    Cam * camera = &Cameras[camid];

    // resetting cropping to full frame
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    memset((void*) &cropcap, 0, sizeof(struct v4l2_cropcap));

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == ioctl(camera->fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        ioctl(camera->fd, VIDIOC_S_CROP, &crop);
    }

    // set format
    struct v4l2_format fmt;

    memset((void*) &fmt, 0, sizeof(struct v4l2_format));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;

#if defined __NEON__
    // Check that line width is multiple of 16
    if(0 != (width % 16))
    {
        width = width & 0xFFFFFFF0;
    }
#endif

    // TODO: error when ratio not correct
    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    if (ioctl(camera->fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("unable to set v4l2 format");
        return -1;
    }
    camera->height   = height;
    camera->width    = width;
    camera->height1  = fmt.fmt.pix.height;
    camera->width1   = fmt.fmt.pix.width;
    camera->nbuffers = nbuffers;
    camera->fps      = fps;

    if (camera->height != camera->height1 || camera->width != camera->width1) {
        printf("Warning: camera resolution changed to %dx%d\n",
               camera->height1, camera->width1);
    }

    return 0;
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
    if (0 > open_device(camid, camera->device)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    // init device
    if (0 > init_capability(camid)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (0 > init_format(camid, width, height, fps, nbuffers)) {
        lua_pushboolean(L, 0);
        return 1;
    }

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
    int ret = ioctl(camera->fd, VIDIOC_REQBUFS, &rb);
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

    lua_pushboolean(L, 1);
    return 1;
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
    THFloatTensor * frame =
        luaT_toudata(L, 2, luaT_typenameid(L, "torch.FloatTensor"));

    // resize given tensor
    THFloatTensor_resize3d(frame, 3,
                           camera->height1, camera->width1);

    // grab frame
    int ret = 0;
    int m0 = frame->stride[1];
    int m1 = frame->stride[2];
    int m2 = frame->stride[0];
    struct v4l2_buffer buf;
    unsigned char *src, *srcp;
    float *dst = THFloatTensor_data(frame);
    float *dstp;
    int i, j, j2, k;
    memset((void*) &buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(camera->fd, VIDIOC_DQBUF, &buf);
    src = (unsigned char *)(camera->buffers[buf.index]);

    int width1 = camera->width1;
    int height1 = camera->height1;

    float fl_cst_val[5] = {0.00456620784314,
                           0.00625892941176,
                           0.00318810980392,
                           0.00791071372549,
                           0.00153618039216};
    float f,y,u,v;

#pragma omp parallel for shared(src, dst, fl_cst_val, width1, camera) private(i,j,j2,k,srcp,dstp,y,u,v)
    for (i=0; i < camera->height1; i++) {
        srcp = &src[i * (camera->width1 << 1)];
        dstp = &dst[i * m0];

// Replace the grab line part by an Neon optimized version
#if defined __NEON__
        // This assembly part operate on one line of the frame
        __asm__ __volatile__ (
            "mov         r0, %1              @ Make a working float const pointer \n\t"
            "mov         r2, %0              @ Make a working src pointer \n\t"
            "mov         r1, #0              @ init width1 counter \n\t"
            "mov         r3, #128            @ Use temp register r3 to store int const \n\t"
            "mov         r4, #16             @ Use temp register r4 to store int const\n\t"
            "vld4.8      {d16,d17,d18,d19},[r2]! @ \n\t"
            "vdup.8      d2, r3              @ \n\t"
            "vdup.8      d3, r4              @ \n\t"
            "mov         r3, %2              @ pointer on dst R \n\t"
            "lsls        r5, %3, #2          @ Multiply stride by size of float \n\t"
            "adds        r4, r3, r5          @ Add stride m2 for 2nd component \n\t"
            "adds        r5, r4, r5          @ Add stride again m2 for 3th component \n\t"
            "1:                              @ loop on line \n\t"
            "vsubl.u8    q2, d16, d3         @ \n\t"
            "vsubl.u8    q3, d17, d2         @ \n\t"
            "vld1.32     {d0,d1}, [r0]!      @ \n\t"
            "vsubl.u8    q5, d19, d2         @ \n\t"
            "vsubl.u8    q4, d18, d3         @ \n\t"
            "vmovl.s16   q6, d4              @ \n\t"
            "vmovl.s16   q7, d6              @ \n\t"
            "vmovl.s16   q8, d8              @ \n\t"
            "vmovl.s16   q9, d10             @ \n\t"
            "vcvt.f32.s32 q6, q6             @ \n\t"
            "vcvt.f32.s32 q7, q7             @ \n\t"
            "vcvt.f32.s32 q8, q8             @ \n\t"
            "vcvt.f32.s32 q9, q9             @ \n\t"
            "vmul.f32    q10, q6, d0[0]      @ \n\t"
            "vmul.f32    q12, q6, d0[0]      @ \n\t"
            "vmul.f32    q14, q6, d0[0]      @ \n\t"
            "vmul.f32    q11, q8, d0[0]      @ \n\t"
            "vmul.f32    q13, q8, d0[0]      @ \n\t"
            "vmul.f32    q15, q8, d0[0]      @ \n\t"
            "vld1.32     d0[0], [r0]         @ \n\t"
            "vmls.f32    q12, q9, d1[0]      @ \n\t"
            "vmla.f32    q10, q9, d0[1]      @ \n\t"
            "vmla.f32    q14, q7, d1[1]      @ \n\t"
            "mov         r0, %1              @ \n\t"
            "vmls.f32    q13, q9, d1[0]      @ \n\t"
            "vmla.f32    q11, q9, d0[1]      @ \n\t"
            "vmla.f32    q15, q7, d1[1]      @ \n\t"
            "vmls.f32    q12, q7, d0[0]      @ \n\t"
            "vmls.f32    q13, q7, d0[0]      @ \n\t"
            "vld1.32     {d0,d1}, [r0]!      @ \n\t"
            "vzip.32     q10, q11            @ \n\t"
            "vzip.32     q14, q15            @ \n\t"
            "vzip.32     q12, q13            @ \n\t"
            "vst1.32     {d20-d23}, [r3]!    @ \n\t"
            "vst1.32     {d24-d27}, [r4]!    @ \n\t"
            "vst1.32     {d28-d31}, [r5]!    @ \n\t"
            "vmovl.s16   q6, d5              @ \n\t"
            "vmovl.s16   q7, d7              @ \n\t"
            "vmovl.s16   q8, d9              @ \n\t"
            "vmovl.s16   q9, d11             @ \n\t"
            "vcvt.f32.s32 q6, q6             @ \n\t"
            "vcvt.f32.s32 q7, q7             @ \n\t"
            "vcvt.f32.s32 q8, q8             @ \n\t"
            "vcvt.f32.s32 q9, q9             @ \n\t"
            "vmul.f32    q10, q6, d0[0]      @ \n\t"
            "vmul.f32    q12, q6, d0[0]      @ \n\t"
            "vmul.f32    q14, q6, d0[0]      @ \n\t"
            "vmul.f32    q11, q8, d0[0]      @ \n\t"
            "vmul.f32    q13, q8, d0[0]      @ \n\t"
            "vmul.f32    q15, q8, d0[0]      @ \n\t"
            "vld1.32     d0[0], [r0]         @ \n\t"
            "vmls.f32    q12, q9, d1[0]      @ \n\t"
            "vmla.f32    q10, q9, d0[1]      @ \n\t"
            "mov         r0, %1              @ \n\t"
            "vmla.f32    q14, q7, d1[1]      @ \n\t"
            "vmls.f32    q13, q9, d1[0]      @ \n\t"
            "vmla.f32    q11, q9, d0[1]      @ \n\t"
            "vmla.f32    q15, q7, d1[1]      @ \n\t"
            "vmls.f32    q12, q7, d0[0]      @ \n\t"
            "vmls.f32    q13, q7, d0[0]      @ \n\t"
            "adds        r1, r1, #16         @ Increment widthl counter\n\t"
            "cmp         r1, %4              @ Check end of line\n\t"
            "bge         3f                  @ \n\t"
            "2:                              @ \n\t"
            "vld4.8      {d16,d17,d18,d19},[r2]! @ Load next 16 pixels \n\t"
            "vzip.32     q10, q11            @ \n\t"
            "vzip.32     q14, q15            @ \n\t"
            "vzip.32     q12, q13            @ \n\t"
            "vst1.32     {d20-d23}, [r3]!    @ \n\t"
            "vst1.32     {d24-d27}, [r4]!    @ \n\t"
            "vst1.32     {d28-d31}, [r5]!    @ \n\t"
            "b           1b                  @ \n\t"
            "3:                              @ \n\t"
            "vzip.32     q10, q11            @ \n\t"
            "vzip.32     q14, q15            @ \n\t"
            "vzip.32     q12, q13            @ \n\t"
            "vst1.32     {d20-d23}, [r3]!    @ \n\t"
            "vst1.32     {d24-d27}, [r4]!    @ \n\t"
            "vst1.32     {d28-d31}, [r5]!    @ \n\t"
            :
            :"r" (srcp),"r" (fl_cst_val),"r"(dstp),"r"(m2),"r"(width1)
            : "cc", "r0", "r1", "r2", "r3", "r4", "r5", "memory",
              "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
              "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
            );
#else
        for (j=0, k=0; j < camera->width1; j++, k+=m1) {

            j2 = j<<1;

            // get Y,U,V chanels from bayer pattern
            y = (float)srcp[j2];
            if (j & 1) {
                u = (float)srcp[j2-1]; v = (float)srcp[j2+1];
            } else {
                u = (float)srcp[j2+1]; v = (float)srcp[j2+3];
            }

            // convert to RGB
            // red:
            dstp[k] = 0.00456*(y-16) + 0.00625*(v-128);
            // green:
            dstp[k+m2] = 0.00456*(y-16) - 0.00318*(v-128) - 0.001536*(u-128);
            // blue:
            dstp[k+2*m2] = 0.00456*(y-16) + 0.007910*(u-128);
        }
#endif
    }
    ret += ioctl(camera->fd, VIDIOC_QBUF, &buf);

    return 0;
}


static int l_releaseCam (lua_State *L) {
    return 0;
}


// Register functions in LUA
static const struct luaL_reg v4l [] = {
    {"init"             , l_init},
    {"grabFrame"        , l_grabFrame},
    {"releaseCam"       , l_releaseCam},
    {"setFocusType"     , l_set_focus_type},
    {"adjustManualFocus", l_adjust_manual_focus},
    {NULL, NULL}  /* sentinel */
};

int luaopen_libv4l (lua_State *L) {
    luaL_openlib(L, "libv4l", v4l, 0);
    return 1;
}
