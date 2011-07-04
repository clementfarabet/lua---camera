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
/*

 Note: see also
 http://www.auc.edu.au/conf/conf03/papers/FAUC_DV2003_Heckenberg.pdf
 which suggests the "VDIG" interface is better for low-latency video
 access.

*/

#include "cam_iface.h"

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include <stdlib.h>
#include <stdio.h>

struct CCquicktime; // forward declaration

// keep functable in sync across backends
typedef struct {
  cam_iface_constructor_func_t construct;
  void (*destruct)(struct CamContext*);

  void (*CCquicktime)(struct CCquicktime*,int,int,int);
  void (*close)(struct CCquicktime*);
  void (*start_camera)(struct CCquicktime*);
  void (*stop_camera)(struct CCquicktime*);
  void (*get_num_camera_properties)(struct CCquicktime*,int*);
  void (*get_camera_property_info)(struct CCquicktime*,
                                   int,
                                   CameraPropertyInfo*);
  void (*get_camera_property)(struct CCquicktime*,int,long*,int*);
  void (*set_camera_property)(struct CCquicktime*,int,long,int);
  void (*grab_next_frame_blocking)(struct CCquicktime*,
                                   unsigned char*,
                                   float);
  void (*grab_next_frame_blocking_with_stride)(struct CCquicktime*,
                                               unsigned char*,
                                               intptr_t,
                                               float);
  void (*point_next_frame_blocking)(struct CCquicktime*,unsigned char**,float);
  void (*unpoint_frame)(struct CCquicktime*);
  void (*get_last_timestamp)(struct CCquicktime*,double*);
  void (*get_last_framenumber)(struct CCquicktime*,unsigned long*);
  void (*get_num_trigger_modes)(struct CCquicktime*,int*);
  void (*get_trigger_mode_string)(struct CCquicktime*,int,char*,int);
  void (*get_trigger_mode_number)(struct CCquicktime*,int*);
  void (*set_trigger_mode_number)(struct CCquicktime*,int);
  void (*get_frame_roi)(struct CCquicktime*,int*,int*,int*,int*);
  void (*set_frame_roi)(struct CCquicktime*,int,int,int,int);
  void (*get_max_frame_size)(struct CCquicktime*,int*,int*);
  void (*get_buffer_size)(struct CCquicktime*,int*);
  void (*get_framerate)(struct CCquicktime*,float*);
  void (*set_framerate)(struct CCquicktime*,float);
  void (*get_num_framebuffers)(struct CCquicktime*,int*);
  void (*set_num_framebuffers)(struct CCquicktime*,int);
} CCquicktime_functable;

typedef struct CCquicktime {
  CamContext inherited;

  GWorldPtr gworld; // offscreen gworld
  SGChannel sg_video_channel;
  Rect rect;
  TimeValue last_timestamp;
  unsigned long last_framenumber;
  int buffer_size;
  ImageSequence decompression_sequence;

  /* These are private parameters to be used in DataProc and SGIdle caller only! */
  int image_was_copied;
  intptr_t stride0;
  unsigned char * image_buf;
} CCquicktime;

// forward declarations
CCquicktime* CCquicktime_construct( int device_number, int NumImageBuffers,
                              int mode_number);
void delete_CCquicktime(struct CCquicktime*);

void CCquicktime_CCquicktime(struct CCquicktime*,int,int,int);
void CCquicktime_close(struct CCquicktime*);
void CCquicktime_start_camera(struct CCquicktime*);
void CCquicktime_stop_camera(struct CCquicktime*);
void CCquicktime_get_num_camera_properties(struct CCquicktime*,int*);
void CCquicktime_get_camera_property_info(struct CCquicktime*,
                              int,
                              CameraPropertyInfo*);
void CCquicktime_get_camera_property(struct CCquicktime*,int,long*,int*);
void CCquicktime_set_camera_property(struct CCquicktime*,int,long,int);
void CCquicktime_grab_next_frame_blocking(struct CCquicktime*,
                              unsigned char*,
                              float);
void CCquicktime_grab_next_frame_blocking_with_stride(struct CCquicktime*,
                                          unsigned char*,
                                          intptr_t,
                                          float);
void CCquicktime_point_next_frame_blocking(struct CCquicktime*,unsigned char**,float);
void CCquicktime_unpoint_frame(struct CCquicktime*);
void CCquicktime_get_last_timestamp(struct CCquicktime*,double*);
void CCquicktime_get_last_framenumber(struct CCquicktime*,unsigned long*);
void CCquicktime_get_num_trigger_modes(struct CCquicktime*,int*);
void CCquicktime_get_trigger_mode_string(struct CCquicktime*,int,char*,int);
void CCquicktime_get_trigger_mode_number(struct CCquicktime*,int*);
void CCquicktime_set_trigger_mode_number(struct CCquicktime*,int);
void CCquicktime_get_frame_roi(struct CCquicktime*,int*,int*,int*,int*);
void CCquicktime_set_frame_roi(struct CCquicktime*,int,int,int,int);
void CCquicktime_get_max_frame_size(struct CCquicktime*,int*,int*);
void CCquicktime_get_buffer_size(struct CCquicktime*,int*);
void CCquicktime_get_framerate(struct CCquicktime*,float*);
void CCquicktime_set_framerate(struct CCquicktime*,float);
void CCquicktime_get_num_framebuffers(struct CCquicktime*,int*);
void CCquicktime_set_num_framebuffers(struct CCquicktime*,int);

CCquicktime_functable CCquicktime_vmt = {
  (cam_iface_constructor_func_t)CCquicktime_construct,
  (void (*)(CamContext*))delete_CCquicktime,
  CCquicktime_CCquicktime,
  CCquicktime_close,
  CCquicktime_start_camera,
  CCquicktime_stop_camera,
  CCquicktime_get_num_camera_properties,
  CCquicktime_get_camera_property_info,
  CCquicktime_get_camera_property,
  CCquicktime_set_camera_property,
  CCquicktime_grab_next_frame_blocking,
  CCquicktime_grab_next_frame_blocking_with_stride,
  CCquicktime_point_next_frame_blocking,
  CCquicktime_unpoint_frame,
  CCquicktime_get_last_timestamp,
  CCquicktime_get_last_framenumber,
  CCquicktime_get_num_trigger_modes,
  CCquicktime_get_trigger_mode_string,
  CCquicktime_get_trigger_mode_number,
  CCquicktime_set_trigger_mode_number,
  CCquicktime_get_frame_roi,
  CCquicktime_set_frame_roi,
  CCquicktime_get_max_frame_size,
  CCquicktime_get_buffer_size,
  CCquicktime_get_framerate,
  CCquicktime_set_framerate,
  CCquicktime_get_num_framebuffers,
  CCquicktime_set_num_framebuffers
};


/* typedefs */

/* globals */

#ifdef MEGA_BACKEND
  #define BACKEND_GLOBAL(m) quicktime_##m
#else
  #define BACKEND_GLOBAL(m) m
#endif

// See the following for a hint on how to make thread thread-local without __thread.
// http://lists.apple.com/archives/Xcode-users/2006/Jun/msg00551.html

int BACKEND_GLOBAL(cam_iface_error) = 0;
#define CAM_IFACE_QT_MAX_ERROR_LEN 255
char BACKEND_GLOBAL(cam_iface_error_string)[CAM_IFACE_QT_MAX_ERROR_LEN]  = {0x00}; //...

uint32_t BACKEND_GLOBAL(num_cameras) = 0;
Component *BACKEND_GLOBAL(components_by_device_number)=NULL;

#ifdef MEGA_BACKEND
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  snprintf(quicktime_cam_iface_error_string,CAM_IFACE_QT_MAX_ERROR_LEN,              \
           "%s (%d): %s\n",__FILE__,__LINE__,(m));
#else
#define CAM_IFACE_ERROR_FORMAT(m)                                       \
  snprintf(cam_iface_error_string,CAM_IFACE_QT_MAX_ERROR_LEN,              \
           "%s (%d): %s\n",__FILE__,__LINE__,(m));
#endif

#ifdef MEGA_BACKEND
#define CAM_IFACE_CHECK_DEVICE_NUMBER(m)                                \
  if ( ((m)<0) | ((m)>=quicktime_num_cameras) ) {                                 \
    quicktime_cam_iface_error = -1;                                               \
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
#define CAM_IFACE_CHECK_MODE_NUMBER(m)                  \
  if ((m)!=0) {                                         \
    quicktime_cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT("only mode 0 exists");       \
    return;                                             \
  }
#else
#define CAM_IFACE_CHECK_MODE_NUMBER(m)                  \
  if ((m)!=0) {                                         \
    cam_iface_error = -1;                               \
    CAM_IFACE_ERROR_FORMAT("only mode 0 exists");       \
    return;                                             \
  }
#endif

void do_qt_error(OSErr err,char* file,int line) {
  char* str=NULL;
  Handle h;

  h = GetResource('Estr', err);

  if (h) {
    HLock(h);
    str = (char *)*h;
    memcpy(BACKEND_GLOBAL(cam_iface_error_string), str+1, (unsigned char)str[0]);
    BACKEND_GLOBAL(cam_iface_error_string)[(unsigned char)str[0]] = '\0';
    HUnlock(h);
    ReleaseResource(h);
  }
  else {
    snprintf(BACKEND_GLOBAL(cam_iface_error_string),CAM_IFACE_QT_MAX_ERROR_LEN, "Mac OS error code %d (%s,line %d)",
             err,file,line);
  }

  BACKEND_GLOBAL(cam_iface_error) = -1;
  return;
}

#define CHK_QT(m) {                                                     \
    if ((m)!=noErr) {                                                   \
      do_qt_error(m,__FILE__,__LINE__);                                 \
      return;                                                           \
    }                                                                   \
}

#ifdef MEGA_BACKEND
#define CHECK_CC(m)                                                     \
  if (!(m)) {                                                           \
    quicktime_cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("no CamContext specified (NULL argument)");  \
    return;                                                             \
  }
#else
#define CHECK_CC(m)                                                     \
  if (!(m)) {                                                           \
    cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("no CamContext specified (NULL argument)");  \
    return;                                                             \
  }
#endif

#ifdef MEGA_BACKEND
#define CHECK_CCV(m)                                                    \
  if (!(m)) {                                                           \
    quicktime_cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("no CamContext specified (NULL argument)");  \
    return NULL;                                                        \
  }
#else
#define CHECK_CCV(m)                                                    \
  if (!(m)) {                                                           \
    cam_iface_error = -1;                                               \
    CAM_IFACE_ERROR_FORMAT("no CamContext specified (NULL argument)");  \
    return NULL;                                                        \
  }
#endif

#ifdef MEGA_BACKEND
#define NOT_IMPLEMENTED                                 \
  quicktime_cam_iface_error = -1;                                 \
  CAM_IFACE_ERROR_FORMAT("not yet implemented");        \
  return;
#else
#define NOT_IMPLEMENTED                                 \
  cam_iface_error = -1;                                 \
  CAM_IFACE_ERROR_FORMAT("not yet implemented");        \
  return;
#endif

#include "cam_iface_quicktime.h"

const char *BACKEND_METHOD(cam_iface_get_driver_name)() {
  return "QuickTime SequenceGrabber";
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

OSErr handle_frame_func(SGChannel chan, Rect *source_rect, GWorldPtr gworldptr, ImageSequence *im_sequence) {
  OSErr err = noErr;

  ImageDescriptionHandle im_desc = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));
  if (im_desc) {
    err = SGGetChannelSampleDescription(chan,(Handle)im_desc);
    if (err == noErr) {
      MatrixRecord matrixrecord;
      Rect rect;

      rect.left = 0;
      rect.right = (*im_desc)->width;
      rect.top = 0;
      rect.bottom = (*im_desc)->height;

      RectMatrix(&matrixrecord, &rect, source_rect);
      err = DecompressSequenceBegin(im_sequence, im_desc, gworldptr, 0, nil, &matrixrecord,srcCopy,nil,0, codecNormalQuality, bestSpeedCodec);
    }
    DisposeHandle((Handle)im_desc);
  }
  else {
    err = MemError();
  }

  return err;
}

pascal OSErr process_data_callback(SGChannel c, Ptr p, long len, long *offset,
                                   long chRefCon, TimeValue time, short writeType,
                                   long refCon) {
  /* This is called within SGIdle */
  CodecFlags ignore;
  CCquicktime* be=(CCquicktime*)refCon;
  Rect bounds;

  /*
  if (be->buffer_size != len ) {
    fprintf(stderr,"buffer_size did not equal image length!\n");
    exit(-1);
  }
  */

  DecompressSequenceFrameS(be->decompression_sequence,p,len,0,&ignore,NULL);

  GetPortBounds(be->gworld, &bounds);
  OffsetRect(&bounds, -bounds.left, -bounds.top); // XXX deprecated call

  PixMapHandle  pix = GetGWorldPixMap(be->gworld); // XXX deprecated call
  int h = bounds.bottom;
  int wb = GetPixRowBytes(pix); // XXX deprecated call
  int row;
  char *baseAddr;

  baseAddr = GetPixBaseAddr(pix); // XXX deprecated call

  for (row=0; row<h; row++) {
    //fprintf(stdout, "row = %d, baseAddr = %d, wb = %d, stride0 = %d\n",row,baseAddr,wb,be->stride0);
    memcpy( (void*)(be->image_buf + row*be->stride0), // dest
            baseAddr, // src
            be->stride0 );
    baseAddr += wb;
  }

  be->image_was_copied = 1;
  be->last_timestamp = time;
  (be->last_framenumber)++;

  /*
  if (be->is_buffer_waiting_to_be_filled) {
    // this should happen majority of time -- listener is waiting for frame...
    if (len > be->max_buffer_size) {
      // throw error
      fprintf(stderr,"ERROR: data too big, should quit now!\n");
    }
    memcpy(be->buffer_to_fill,pucs,len);
    signal_frame_ready();
  } else {
    // in this case, we'll have to copy the data and then return it when the listener asks for next frame
    //copy_image_data_into_circular_quete(be);
  }
  */

  /*
  fprintf(stdout,"pucs %d\n",(long)pucs);
  fprintf(stdout,"received %d values, first few: %d %d %d %d %d %d %d %d\n",len,pucs[0],pucs[1],pucs[2],pucs[3],pucs[4],pucs[5],pucs[6],pucs[7]);
  */

  /*
  fprintf(stdout,".");
  fflush(stdout);
  */

  return noErr;
}

void BACKEND_METHOD(cam_iface_startup)() {
  OSErr err;
  ComponentDescription  theDesc;
  Component             sgCompID;
  int i;

  char name[256];
  Handle componentName=NULL;
  char info[256];
  Handle componentInfo=NULL;
  componentName=NewHandle(sizeof(name));
  componentInfo=NewHandle(sizeof(info));

  err=EnterMovies();
  CHK_QT(err);

  theDesc.componentType = SeqGrabComponentType;
  theDesc.componentSubType = 0L;
  theDesc.componentManufacturer = 0L;
  theDesc.componentFlags = 0L;
  theDesc.componentFlagsMask = 0L;

  BACKEND_GLOBAL(num_cameras) = CountComponents( &theDesc );
  BACKEND_GLOBAL(components_by_device_number) = (Component*)malloc( BACKEND_GLOBAL(num_cameras)*sizeof(Component));
  if (BACKEND_GLOBAL(components_by_device_number)==NULL) {
    BACKEND_GLOBAL(cam_iface_error) = -1;
    CAM_IFACE_ERROR_FORMAT("malloc failed");
    return;
  }

  for (i=0; i<BACKEND_GLOBAL(num_cameras); i++) {
    sgCompID    = FindNextComponent(sgCompID, &theDesc);
    BACKEND_GLOBAL(components_by_device_number)[i] = sgCompID;

    // Used http://muonics.net/extras/GRL_ROTTERDAM_KPN_APP/kpnAppSourceCode.zip
    // for componentName and componentInfo example.

    err=GetComponentInfo(sgCompID,&theDesc,componentName,componentInfo,NULL);
    CHK_QT(err);

    /*
    printf("found a device to capture with \n");
    printf("Type: %s\n", &theDesc.componentType);
    printf("Manufacturer: %s\n", &theDesc.componentManufacturer);
    printf("Name: %s\n", *componentName ? p2cstr((StringPtr)*componentName) : "-" );
    printf("Info: %s\n", *componentInfo ? p2cstr((StringPtr)*componentInfo) : "-");
    */

  }
  DisposeHandle(componentName);
  DisposeHandle(componentInfo);
}

void BACKEND_METHOD(cam_iface_shutdown)() {
}

int BACKEND_METHOD(cam_iface_get_num_cameras)() {
  return BACKEND_GLOBAL(num_cameras);
}

void BACKEND_METHOD(cam_iface_get_camera_info)(int device_number, Camwire_id *out_camid) {
  /// XXX TODO: should implement this
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  snprintf(out_camid->vendor,CAMWIRE_ID_MAX_CHARS,"Quicktime API, unknown vendor");
  snprintf(out_camid->model,CAMWIRE_ID_MAX_CHARS,"unknown model");
  snprintf(out_camid->chip,CAMWIRE_ID_MAX_CHARS,"unknown chip");
}

void BACKEND_METHOD(cam_iface_get_num_modes)(int device_number, int *num_modes) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  *num_modes = 1;
}

void BACKEND_METHOD(cam_iface_get_mode_string)(int device_number,
                               int mode_number,
                               char* mode_string,
                               int mode_string_maxlen) {
  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  CAM_IFACE_CHECK_MODE_NUMBER(mode_number);
  snprintf(mode_string,mode_string_maxlen,"default mode");
}


cam_iface_constructor_func_t BACKEND_METHOD(cam_iface_get_constructor_func)(int device_number) {
  return (CamContext* (*)(int, int, int))CCquicktime_construct;
}

CCquicktime* CCquicktime_construct( int device_number, int NumImageBuffers,
                                    int mode_number) {
  CCquicktime* this=NULL;

  this = malloc(sizeof(CCquicktime));
  if (this==NULL) {
    BACKEND_GLOBAL(cam_iface_error) = -1;
    CAM_IFACE_ERROR_FORMAT("error allocating memory");
  } else {
    CCquicktime_CCquicktime( this,
                             device_number, NumImageBuffers,
                             mode_number);
    if (BACKEND_GLOBAL(cam_iface_error)) {
      free(this);
      return NULL;
    }
  }
  return this;
}

void delete_CCquicktime( CCquicktime *this ) {
  CCquicktime_close(this);
  this->inherited.vmt = NULL;
  free(this);
  this = NULL;
}

void CCquicktime_CCquicktime( CCquicktime* in_cr,
                              int device_number, int NumImageBuffers,
                              int mode_number) {
  SeqGrabComponent cam;
  Component sgCompID;
  OSErr err;

  // call parent
  CamContext_CamContext((CamContext*)in_cr,device_number,NumImageBuffers,mode_number);
  in_cr->inherited.vmt = (CamContext_functable*)&CCquicktime_vmt;

  CAM_IFACE_CHECK_DEVICE_NUMBER(device_number);
  CAM_IFACE_CHECK_MODE_NUMBER(mode_number);

  sgCompID = BACKEND_GLOBAL(components_by_device_number)[device_number];

  cam = OpenComponent(sgCompID);
  if (cam==NULL) {
    BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("Sequence Grabber could not be opened"); return;
  }

  in_cr->inherited.cam = (void *)cam;

  in_cr->gworld=NULL;
  err = SGInitialize(cam);
  CHK_QT(err);

  err = SGSetDataRef(cam, 0, 0, seqGrabDontMakeMovie);
  CHK_QT(err);

  err = SGNewChannel(cam, VideoMediaType, &(in_cr->sg_video_channel));
  CHK_QT(err);

  in_cr->rect.left=0;
  in_cr->rect.top=0;
  in_cr->rect.right=640;
  in_cr->rect.bottom=480;

  in_cr->last_framenumber = 0;

  in_cr->inherited.device_number=device_number;

  err = SGSetChannelBounds(in_cr->sg_video_channel, &(in_cr->rect));
  CHK_QT(err);

#if 0
  in_cr->coding = CAM_IFACE_ARGB8;
  in_cr->depth=32;
  err = QTNewGWorld( &(in_cr->gworld),
                     k32ARGBPixelFormat,// let Mac do conversion to RGB
                     &(in_cr->rect),
                     0, NULL, 0);
  CHK_QT(err);
#else
  in_cr->inherited.coding = CAM_IFACE_YUV422,
  in_cr->inherited.depth=16;
  err = QTNewGWorld( &(in_cr->gworld),
                     k422YpCbCr8PixelFormat,
                     &(in_cr->rect),
                     0, NULL, 0);
  CHK_QT(err);
#endif

  PixMapHandle pix = GetGWorldPixMap(in_cr->gworld); // XXX deprecated call
  int width_bytes = GetPixRowBytes(pix); // XXX deprecated call
  int height = in_cr->rect.bottom;
  in_cr->buffer_size = width_bytes*height;

  err = SGSetGWorld(cam, in_cr->gworld, NULL);
  CHK_QT(err);

  err = SGSetChannelUsage(in_cr->sg_video_channel, seqGrabRecord);
  CHK_QT(err);

  err = SGSetDataProc(cam, NewSGDataUPP(process_data_callback), (long)in_cr);
  CHK_QT(err);

  err = SGPrepare(cam,false,true);
  CHK_QT(err);

  err = handle_frame_func(in_cr->sg_video_channel, &(in_cr->rect), in_cr->gworld, &(in_cr->decompression_sequence));
  CHK_QT(err);

  return;
}

void CCquicktime_close( CCquicktime *in_cr) {
  CHECK_CC(in_cr);
}

void CCquicktime_get_max_frame_size( CCquicktime *in_cr,
                                     int *width,
                                     int *height ) {
  CHECK_CC(in_cr);
  *width=in_cr->rect.right;
  *height=in_cr->rect.bottom;
}

void CCquicktime_get_num_camera_properties(CCquicktime *in_cr,
                                           int* num_properties) {
  CHECK_CC(in_cr);
  *num_properties = 0;
}

void CCquicktime_get_camera_property_info(CCquicktime *in_cr,
                                          int property_number,
                                          CameraPropertyInfo *info) {
  NOT_IMPLEMENTED;
}

void CCquicktime_get_camera_property(CCquicktime *in_cr,
                                     int property_number,
                                     long* Value,
                                     int* Auto ) {
  NOT_IMPLEMENTED;
}

void CCquicktime_set_camera_property(CCquicktime *in_cr,
                                     int property_number,
                                     long Value,
                                     int Auto ) {
  NOT_IMPLEMENTED;
}

void CCquicktime_get_buffer_size( CCquicktime *in_cr,
                                  int *size) {
  CHECK_CC(in_cr);
  *size=in_cr->buffer_size;
}

void CCquicktime_start_camera( CCquicktime *in_cr ) {
  OSErr err;
  CHECK_CC(in_cr);
  err = SGStartRecord(in_cr->inherited.cam);
  CHK_QT(err);
}

void CCquicktime_stop_camera( CCquicktime *in_cr ) {
  OSErr err;
  CHECK_CC(in_cr);
  err = SGStop(in_cr->inherited.cam);
  CHK_QT(err);
}

void CCquicktime_grab_next_frame_blocking_with_stride( CCquicktime *ccntxt,
                                                       unsigned char *out_bytes,
                                                       intptr_t stride0,
                                                       float timeout) {
  SeqGrabComponent cam;
  OSErr err=noErr;

  CHECK_CC(ccntxt);
  cam = ccntxt->inherited.cam;

  if (timeout >= 0) {
    NOT_IMPLEMENTED;
    return;
  }

  ccntxt->image_was_copied = 0;
  ccntxt->stride0 = stride0;
  ccntxt->image_buf = out_bytes;

  while(1) {
    err = SGIdle(cam); // this will call our callback
    CHK_QT(err);

    // check for image
    if (ccntxt->image_was_copied) {
      // we've got one
      break;
    } else {
      // sleep and repeat
      usleep( 1000 ); // 1 millisecond
    }
  }
}

void CCquicktime_grab_next_frame_blocking( CCquicktime *ccntxt,
                                           unsigned char *out_bytes,
                                           float timeout ) {
  intptr_t stride0;

  CHECK_CC(ccntxt);

  stride0 = ccntxt->rect.right*(ccntxt->inherited.depth/8);
  CCquicktime_grab_next_frame_blocking_with_stride( ccntxt, out_bytes, stride0, timeout ) ;
}

void CCquicktime_point_next_frame_blocking(struct CCquicktime*this,unsigned char**data,float timeout) {
  NOT_IMPLEMENTED;
}

void CCquicktime_unpoint_frame(struct CCquicktime*this) {
  NOT_IMPLEMENTED;
}

void CCquicktime_get_last_timestamp( CCquicktime *in_cr, double* timestamp ) {
  CHECK_CC(in_cr);
  // convert from microseconds to seconds
  //*timestamp = (double)(in_cr->last_timestamp) * 1e-6;
  *timestamp = 0.0;
}

void CCquicktime_get_last_framenumber( CCquicktime *in_cr, unsigned long* framenumber ){
  CHECK_CC(in_cr);
  *framenumber=in_cr->last_framenumber;
}

void CCquicktime_get_num_trigger_modes( CCquicktime *in_cr,
                                        int *num_exposure_modes ) {
  *num_exposure_modes = 1;
}

void CCquicktime_get_trigger_mode_string( CCquicktime *ccntxt,
                                          int exposure_mode_number,
                                          char* exposure_mode_string, //output parameter
                                          int exposure_mode_string_maxlen) {
  CHECK_CC(ccntxt);
  if (exposure_mode_number!=0) {
    BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("only trigger mode 0 supported"); return;
  }
  snprintf(exposure_mode_string,exposure_mode_string_maxlen,"internal freerunning");
}

void CCquicktime_get_trigger_mode_number( CCquicktime *ccntxt,
                                          int *exposure_mode_number ) {
  CHECK_CC(ccntxt);
  *exposure_mode_number=0;
}

void CCquicktime_set_trigger_mode_number( CCquicktime *ccntxt,
                                          int exposure_mode_number ) {
  CHECK_CC(ccntxt);
  if (exposure_mode_number!=0) {
    BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("only trigger mode 0 supported"); return;
  }
}

void CCquicktime_get_frame_roi( CCquicktime *in_cr,
                                int *left, int *top, int *width, int *height ) {
  CHECK_CC(in_cr);
  *left = 0;
  *top = 0;
  *width=in_cr->rect.right;
  *height=in_cr->rect.bottom;
}

void CCquicktime_set_frame_roi( CCquicktime *in_cr,
                                int left, int top, int width, int height ) {
  CHECK_CC(in_cr);
  if ((left != 0) | (top != 0) ) {
     BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("frame offset can only be 0,0"); return;
  }
  if (width!=in_cr->rect.right) {
    BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("frame width cannot be changed"); return;
  }
  if (height!=in_cr->rect.bottom) {
    BACKEND_GLOBAL(cam_iface_error)=-1; CAM_IFACE_ERROR_FORMAT("frame height cannot be changed"); return;
  }
}
void CCquicktime_get_framerate( CCquicktime *in_cr,
                                float *framerate ) {
  CHECK_CC(in_cr);
  *framerate=30.0;
}

void CCquicktime_set_framerate( CCquicktime *in_cr,
                                float framerate ) {
  NOT_IMPLEMENTED;
}

void CCquicktime_get_num_framebuffers( CCquicktime *in_cr,
                                       int *num_framebuffers ) {

  CHECK_CC(in_cr);
  *num_framebuffers=4; // unknown, really
}

void CCquicktime_set_num_framebuffers( CCquicktime *in_cr,
                                       int num_framebuffers ) {
  CHECK_CC(in_cr);
  NOT_IMPLEMENTED;
}
