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
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "cam_iface.h"

double my_floattime() {
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

#define _check_error() {                                                \
    int _check_error_err;                                               \
    _check_error_err = cam_iface_have_error();                          \
    if (_check_error_err != 0) {                                        \
                                                                        \
      fprintf(stderr,"%s:%d %s\n", __FILE__,__LINE__,cam_iface_get_error_string()); \
      exit(1);                                                          \
    }                                                                   \
  }                                                                     \

void save_pgm(const char* filename,unsigned char *pixels,int width,int height) {
  FILE* fd;
  fd = fopen(filename,"w");
  fprintf(fd,"P5\n");
  fprintf(fd,"%d %d\n",width,height);
  fprintf(fd,"255\n");
  fwrite(pixels,1,width*height,fd);
  fprintf(fd,"\n");
  fclose(fd);
}

void save_ppm(const char* filename,unsigned char *pixels,int width,int height) {
  FILE* fd;
  int row,col;
  const unsigned char *base;
  fd = fopen(filename,"w");
  fprintf(fd,"P3\n");
  fprintf(fd,"%d %d\n",width,height);
  fprintf(fd,"255\n");
  for (row=0; row<height; row++) {
    for (col=0; col<width; col++) {
      base = pixels + row*width*3 + col*3;
      fprintf(fd,"%d %d %d\n",base[0],base[1],base[2]);
    }
  }
  fprintf(fd,"\n");
  fclose(fd);
}

void show_usage(char * cmd) {
  printf("usage: %s [num_frames]\n",cmd);
  printf("  where num_frames can be a number or 'forever'\n");
  exit(1);
}

int main(int argc, char** argv) {
  CamContext *cc;
  unsigned char *pixels;

  int device_number,ncams,num_buffers;

  double last_fps_print, now, t_diff;
  double fps;
  int n_frames;
  int buffer_size;
  int num_modes, num_props, num_trigger_modes;
  char mode_string[255];
  int i,mode_number;
  CameraPropertyInfo cam_props;
  long prop_value;
  int prop_auto;
  int errnum;
  int left, top;
  int width, height;
  int do_num_frames;
  CameraPixelCoding coding;
  cam_iface_constructor_func_t new_CamContext;
  Camwire_id cam_info_struct;

  cam_iface_startup_with_version_check();
  _check_error();

  if (argc>1) {
    if (strcmp(argv[1],"forever")==0) {
      do_num_frames = -1;
    } else if (sscanf(argv[1],"%d",&do_num_frames)==0) {
      show_usage(argv[0]);
    }
  } else {
    do_num_frames = 50;
  }

  for (i=0;i<argc;i++) {
    printf("%d: %s\n",i,argv[i]);
  }
  printf("using driver %s\n",cam_iface_get_driver_name());

  ncams = cam_iface_get_num_cameras();
  _check_error();

  if (ncams<1) {

    printf("no cameras found, will now exit\n");

    cam_iface_shutdown();
    _check_error();

    exit(1);
  }
  _check_error();

  printf("%d camera(s) found.\n",ncams);
  for (i=0; i<ncams; i++) {
    cam_iface_get_camera_info(i, &cam_info_struct);
    printf("  camera %d:\n",i);
    printf("    vendor: %s\n",cam_info_struct.vendor);
    printf("    model: %s\n",cam_info_struct.model);
    printf("    chip: %s\n",cam_info_struct.chip);
  }

  device_number = ncams-1;

  printf("choosing camera %d\n",device_number);

  cam_iface_get_num_modes(device_number, &num_modes);
  _check_error();

  printf("%d mode(s) available:\n",num_modes);

  mode_number = 0;

  for (i=0; i<num_modes; i++) {
    cam_iface_get_mode_string(device_number,i,mode_string,255);
    if (strstr(mode_string,"FORMAT7_0")!=NULL) {
      if (strstr(mode_string,"MONO8")!=NULL) {
        // pick this mode
        mode_number = i;
      }
    }
    printf("  %d: %s\n",i,mode_string);
  }

  printf("Choosing mode %d\n",mode_number);

  num_buffers = 5;

  new_CamContext = cam_iface_get_constructor_func(device_number);
  cc = new_CamContext(device_number,num_buffers,mode_number);
  _check_error();

  CamContext_get_frame_roi(cc, &left, &top, &width, &height);
  _check_error();

  CamContext_get_num_framebuffers(cc,&num_buffers);
  printf("allocated %d buffers\n",num_buffers);

  CamContext_get_num_camera_properties(cc,&num_props);
  _check_error();

  printf("%d camera properties: \n",num_props);

  for (i=0; i<num_props; i++) {
    CamContext_get_camera_property_info(cc,i,&cam_props);
    _check_error();

    if (strcmp(cam_props.name,"white balance")==0) {
      fprintf(stderr,"WARNING: ignoring white balance property\n");
      continue;
    }

    printf("  %s: ",cam_props.name);
    fflush(stdout);

    if (cam_props.is_present) {
      if (cam_props.available) {
	if (cam_props.absolute_capable) {
	  if (cam_props.absolute_control_mode) {
	    printf("(absolute capable, on) " );
	  } else {
	    printf("(absolute capable, off) " );
	  }
	  fflush(stdout);
	}
	if (cam_props.readout_capable) {
	  if (cam_props.has_manual_mode) {
	    CamContext_get_camera_property(cc,i,&prop_value,&prop_auto);
	    _check_error();
	    printf("%ld\n",prop_value);
	  } else {
	    /* Firefly2 temperature won't be read out. */
	    printf("no manual mode, won't read out. Original value: %d\n",cam_props.original_value);
	  }
	} else {
	  printf("not readout capable");
	}
      } else {
	printf("present, but not available\n");
      }
    } else {
      printf("not present\n");
    }
  }

  CamContext_get_buffer_size(cc,&buffer_size);
  _check_error();

  if (buffer_size == 0) {
    fprintf(stderr,"buffer size was 0 in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }

#define USE_COPY
#ifdef USE_COPY
  pixels = (unsigned char *)malloc( buffer_size );
  if (pixels==NULL) {
    fprintf(stderr,"couldn't allocate memory in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }
#endif

  CamContext_start_camera(cc);
  _check_error();

  last_fps_print = my_floattime();
  n_frames = 0;

  if (do_num_frames < 0) {
    printf("will now run forever. press Ctrl-C to interrupt\n");
  } else {
    printf("will now grab %d frames.\n",do_num_frames);
  }

  CamContext_get_num_trigger_modes( cc, &num_trigger_modes );
  _check_error();

  printf("trigger modes:\n");
  for (i =0; i<num_trigger_modes; i++) {
    CamContext_get_trigger_mode_string( cc, i, mode_string, 255 );
    printf("  %d: %s\n",i,mode_string);
  }
  printf("\n");

  while (1) {
    if (do_num_frames>=0) {
      do_num_frames--;
      if (do_num_frames<0) break;
    }
#ifdef USE_COPY
    //CamContext_grab_next_frame_blocking(cc,pixels,0.2); // timeout after 200 msec
    CamContext_grab_next_frame_blocking(cc,pixels,-1.0f); // never timeout
    errnum = cam_iface_have_error();
    if (errnum == CAM_IFACE_FRAME_TIMEOUT) {
      cam_iface_clear_error();
      fprintf(stdout,"T");
      fflush(stdout);
      continue; // wait again
    }
    if (errnum == CAM_IFACE_FRAME_DATA_MISSING_ERROR) {
      cam_iface_clear_error();
      fprintf(stdout,"M");
      fflush(stdout);
    } else if (errnum == CAM_IFACE_FRAME_INTERRUPTED_SYSCALL) {
      cam_iface_clear_error();
      fprintf(stdout,"I");
      fflush(stdout);
    } else if (errnum == CAM_IFACE_FRAME_DATA_CORRUPT_ERROR) {
      cam_iface_clear_error();
      fprintf(stdout,"C");
      fflush(stdout);
    } else {
      _check_error();
      fprintf(stdout,".");
      fflush(stdout);
    }
    now = my_floattime();
    n_frames += 1;
#else
    CamContext_point_next_frame_blocking(cc,&pixels,-1.0f);
    now = my_floattime();
    n_frames += 1;
    _check_error();
    fprintf(stdout,".");
    fflush(stdout);
    CamContext_unpoint_frame(cc);
    _check_error();
#endif

    t_diff = now-last_fps_print;
    if (t_diff > 5.0) {
      fps = n_frames/t_diff;
      fprintf(stdout,"%.1f fps\n",fps);
      last_fps_print = now;
      n_frames = 0;
    }
  }

  coding = cc->coding;

  printf("\n");
  delete_CamContext(cc);
  _check_error();

  cam_iface_shutdown();
  _check_error();

  switch (coding) {
  case CAM_IFACE_MONO8_BAYER_BGGR:
  case CAM_IFACE_MONO8_BAYER_RGGB:
  case CAM_IFACE_MONO8_BAYER_GRBG:
  case CAM_IFACE_MONO8_BAYER_GBRG:
    printf("Bayer image will not be de-mosaiced\n");
  case CAM_IFACE_MONO8:
    save_pgm("image.pgm",pixels, width, height);
    printf("saved last image as image.pgm\n");
    break;
  case CAM_IFACE_RGB8:
    save_ppm("image.ppm",pixels, width, height);
    printf("saved last image as image.ppm\n");
    break;
  default:
    printf("do not know how to save sample image for this format\n");
  }

#ifdef USE_COPY
  free(pixels);
#endif

  return 0;
}
