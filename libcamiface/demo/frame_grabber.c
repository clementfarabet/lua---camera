//==============================================================================
// File: frame_grabber
//
// Description: A webcam interface and frame server.
//
//              Grabs frames from generic webcams, and serves them in a shared
//              memory segment. 
//              A simple api is provided through the shared segment, to sync 
//              clean frames.
//
// Created: February 10, 2010, 12:29AM
//
// Author: Clement Farabet // clement.farabet@gmail.com
//==============================================================================

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "cam_iface.h"

// That flag is used by SigInt, to notify the main loop to quit
static int processingStopped = 0;

void get_time(char * formatted_time) {
  time_t current_time;
  time(&current_time);
  ctime_r(&current_time, formatted_time);
  int i = 0;
  do {
    if (formatted_time[i] == ' ') formatted_time[i] = '_'; 
    if (formatted_time[i] == ':') formatted_time[i] = '-'; 
  } while (formatted_time[i++] != 0);
  formatted_time[i-2] = 0;
}

#define _check_error() {                                                            \
    int _check_error_err;                                                           \
    _check_error_err = cam_iface_have_error();                                      \
    if (_check_error_err != 0) {                                                    \
      fprintf(stderr,"%s:%d %s\n", __FILE__,__LINE__,cam_iface_get_error_string()); \
      exit(1);                                                                      \
    }                                                                               \
  }                                                                                 \

void sigint_handler(int sig_no) {
  printf("\nreceived sigint...\n");
  processingStopped = 1;
}

void yuv422_to_mono8(const unsigned char *src_pixels, 
                     unsigned char *dest_pixels, 
                     int width, int height,
                     size_t src_stride, size_t dest_stride) {
  size_t i,j;
  const unsigned char *src_chunk;
  unsigned char *dest_chunk;
  for (i=0; i<(unsigned int)height; i++) {
    src_chunk = src_pixels + i*src_stride;
    dest_chunk = dest_pixels + i*dest_stride;
    for (j=0; j<((unsigned int)width/2); j++) {
      dest_chunk[0] = src_chunk[1];
      dest_chunk[1] = src_chunk[3];
      dest_chunk+=2;
      src_chunk+=4;
    }
  }
}

#define CLIP(m)                                 \
  (m)<0?0:((m)>255?255:(m))

// from http://en.wikipedia.org/wiki/YUV
#define convert_chunk(src,dest,rgba) {                                  \
  u = src[0];                                                           \
  y1 = src[1];                                                          \
  v = src[2];                                                           \
  y2 = src[3];                                                          \
                                                                        \
  C1 = y1-16;                                                           \
  C2 = y2-16;                                                           \
  D = u-128;                                                            \
  E = v-128;                                                            \
                                                                        \
  dest[0] = CLIP(( 298 * C1           + 409 * E + 128) >> 8);           \
  dest[1] = CLIP(( 298 * C1 - 100 * D - 208 * E + 128) >> 8);           \
  dest[2] = CLIP(( 298 * C1 + 516 * D           + 128) >> 8);           \
  if (rgba) {                                                           \
    dest[3] = 255;                                                      \
    dest[4] = CLIP(( 298 * C2           + 409 * E + 128) >> 8);         \
    dest[5] = CLIP(( 298 * C2 - 100 * D - 208 * E + 128) >> 8);         \
    dest[6] = CLIP(( 298 * C2 + 516 * D           + 128) >> 8);         \
    dest[7] = 255;                                                      \
  } else {                                                              \
    dest[3] = CLIP(( 298 * C2           + 409 * E + 128) >> 8);         \
    dest[4] = CLIP(( 298 * C2 - 100 * D - 208 * E + 128) >> 8);         \
    dest[5] = CLIP(( 298 * C2 + 516 * D           + 128) >> 8);         \
  }                                                                     \
}

void yuv422_to_rgb8(const unsigned char *src_pixels, unsigned char *dest_pixels,
					int width, int height, size_t src_stride,
					size_t dest_stride) {
  int C1, C2, D, E;
  int i,j;
  const unsigned char* src_chunk;
  unsigned char* dest_chunk;
  unsigned char u,y1,v,y2;
  for (i=0; i<height; i++) {
    src_chunk = src_pixels + i*src_stride;
    dest_chunk = dest_pixels + i*dest_stride;
    for (j=0; j<(width/2); j++) {
      convert_chunk(src_chunk,dest_chunk,0);
      dest_chunk+=6;
      src_chunk+=4;
    }
  }
}

void yuv422_to_rgba8(const unsigned char *src_pixels, 
                     unsigned char *dest_pixels, int width, int height,
                     size_t src_stride, size_t dest_stride) {
  int C1, C2, D, E;
  int i,j;
  const unsigned char* src_chunk;
  unsigned char* dest_chunk;
  unsigned char u,y1,v,y2;
  for (i=0; i<height; i++) {
    src_chunk = src_pixels + i*src_stride;
    dest_chunk = dest_pixels + i*dest_stride;
    for (j=0; j<(width/2); j++) {
      convert_chunk(src_chunk,dest_chunk,1);
      dest_chunk+=8;
      src_chunk+=4;
    }
  }
}

int main(int argc, char** argv) {
  CamContext *cc;
  unsigned char *raw_pixels;

  struct video_buffer {
    char request_frame;
    char frame_ready;
    char bytes_per_pixel;
    char exit;
    int width;
    int height;
    unsigned char pixels[];
  };
  volatile struct video_buffer * buffer = NULL; 

  int num_buffers;
  int n_frames;
  int buffer_size;
  int num_modes, num_props;
  char mode_string[255];
  int i,mode_number;
  CameraPropertyInfo cam_props;
  long prop_value;
  int prop_auto;
  int errnum;
  int left, top;
  int width, height;
  Camwire_id cam_info_struct;
  cam_iface_constructor_func_t new_CamContext;
  key_t shmem_key;
  int shmem_id;
  int stride;

  // frequency
  int sleep_btn_frames = 0;
  if (argc > 2) sleep_btn_frames = atoi(argv[2]);

  // That's quite important to handle SIGINT cleanly (ctrl+c)
  struct sigaction sa;  
  memset(&sa, 0, sizeof(sa));  
  sa.sa_handler = &sigint_handler;  
  sigaction(SIGINT, &sa, NULL);

  // Basic checks
  cam_iface_startup_with_version_check();
  _check_error();
  printf("using driver %s\n",cam_iface_get_driver_name());
  printf("%d camera(s) found:\n",cam_iface_get_num_cameras());
  cam_iface_get_camera_info(0, &cam_info_struct);
  printf("\tcamera %d:\n",0);
  printf("\tvendor: %s\n",cam_info_struct.vendor);
  printf("\tmodel: %s\n",cam_info_struct.model);
  printf("\tchip: %s\n",cam_info_struct.chip);

  // Get modes from camera
  cam_iface_get_num_modes(0, &num_modes);
  _check_error();
  printf("%d mode(s) available:\n",num_modes);
  mode_number = 0;
  for (i=0; i<num_modes; i++) {
    cam_iface_get_mode_string(0,i,mode_string,255);
    printf("\t%d: %s\n",i,mode_string);
    if (strstr(mode_string,"640x480")!=NULL) {
      if (strstr(mode_string,"YUV422")!=NULL) {
        // pick this mode
        mode_number = i;
        break;
      }
    }
  }
  printf("Choosing mode %d\n",mode_number);
  
  // Create Camera with 4 buffers
  num_buffers = 1;
  new_CamContext = cam_iface_get_constructor_func(0);
  cc = new_CamContext(0, num_buffers, mode_number);
  _check_error();
  
  // coding
  printf("Camera coding is : %d (%d)\n", cc->coding, CAM_IFACE_YUV422);

  // Region of interest
  CamContext_get_frame_roi(cc, &left, &top, &width, &height);
  _check_error();

  // Stride
  stride = width*cc->depth/8;
  printf("raw image width: %d, stride: %d\n",width,stride);

  // Allocate buffers ?
  CamContext_get_num_framebuffers(cc,&num_buffers);
  printf("allocated %d buffers\n",num_buffers);

  // Properties
  CamContext_get_num_camera_properties(cc,&num_props);
  _check_error();

  for (i=0; i<num_props; i++) {
    CamContext_get_camera_property_info(cc,i,&cam_props);
    _check_error();
    
    if (strcmp(cam_props.name,"white balance")==0) {
      fprintf(stderr,"WARNING: ignoring white balance property\n");
      continue;
    }

    if (cam_props.is_present) {
      CamContext_get_camera_property(cc,i,&prop_value,&prop_auto);
      _check_error();
      printf("  %s: %ld (%s)\n",cam_props.name,prop_value, prop_auto ? "AUTO" : "MANUAL");
    } else {
      printf("  %s: not present\n",cam_props.name);
    }
  }

  // Buffer size
  CamContext_get_buffer_size(cc,&buffer_size);
  _check_error();
  
  if (buffer_size == 0) {
    fprintf(stderr,"buffer size was 0 in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }
  
  // Allocate memory in a shared mem segment.
  // that way it is visible to any external process.
  FILE *temp=fopen("shared-mem", "w"); fclose(temp); // just create the file if it doesnt exist
  // then create the IPC
  if ((shmem_key = ftok("shared-mem", 'A')) == -1) {
    perror("ftok");
    exit(1);
  }
  if((shmem_id = shmget(shmem_key, buffer_size*4 + 16, 0644 | IPC_CREAT)) == -1) {
    perror("shmget"); 
    exit(1);
  }
  buffer = (struct video_buffer *)shmat(shmem_id, (void *)0, 0);

  // Allocate mem for raw pixels
  raw_pixels = (unsigned char *)malloc( buffer_size );
  if (raw_pixels==NULL) {
    fprintf(stderr,"couldn't allocate memory in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }
  printf("allocated %dB for raw pixels\n", buffer_size);

  // ... and start camera !!
  CamContext_start_camera(cc);
  _check_error();

  // trigger modes ?
  int num_trigger_modes;
  CamContext_get_num_trigger_modes( cc, &num_trigger_modes );
  _check_error();
  printf("trigger modes:\n");
  for (i =0; i<num_trigger_modes; i++) {
    CamContext_get_trigger_mode_string( cc, i, mode_string, 255 );
    printf("  %d: %s\n",i,mode_string);
  }
  printf("\n");

  // setting framerate
  // CamContext_set_framerate(cc, 1);

  // Init frame buffer
  buffer->bytes_per_pixel = 3; // RGB
  buffer->request_frame = 0;
  buffer->frame_ready = 0;
  buffer->exit = 0;
  buffer->width = width;
  buffer->height = height;
  
  n_frames = 0;
  while (1) {

    // Grab a frame when requested
    while (!buffer->request_frame) {
      if (processingStopped) goto cleanup; // sigint received
      if (getppid() == 1) goto cleanup; // parent died
      if (buffer->exit) goto cleanup; // exit requested
      if (argc > 1) break; // standalone mode
      usleep(1); // save CPU
    }
    // frame's not ready anymore
    buffer->frame_ready = 0;

    // Ack request
    buffer->request_frame = 0;  

    // Request frame
    CamContext_grab_next_frame_blocking(cc,raw_pixels,-1);

    // Convert the frame
    if (buffer->bytes_per_pixel == 4)
      yuv422_to_rgba8(raw_pixels, buffer->pixels, width, height, stride, width*4);
    else if (buffer->bytes_per_pixel == 3)
      yuv422_to_rgb8(raw_pixels, buffer->pixels, width, height, stride, width*3);

    buffer->frame_ready = 1;

    sleep(sleep_btn_frames); // Time between snapshots

    n_frames++;
  }
  
 cleanup:

  // Message
  printf("<camiface> releasing\n");

  // Free data
  delete_CamContext(cc);
  _check_error();
  
  // Shutdown camera
  cam_iface_shutdown();
  _check_error();

  // Destroy Shared Mem
  if(shmdt(buffer) == -1) {
    perror("shmdt");
    exit(1);
  }
  if(shmctl(shmem_id, IPC_RMID, NULL) == -1) {
    perror("shmctl"); 
    exit(1);
  }
  free(raw_pixels);
  remove("shared-mem");

  return 0;
}


