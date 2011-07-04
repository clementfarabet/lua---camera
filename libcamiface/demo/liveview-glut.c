/*

Copyright (c) 2004-2009, California Institute of Technology.
Copyright (c) 2009, António Ramires Fernandes.
All rights reserved.

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef USE_GLEW
#  include <GL/glew.h>
#endif

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glut.h>
#endif
#include <math.h>

#include "cam_iface.h"

#ifndef SHADER_DIR
#error "SHADER_DIR is undefined"
#endif

#define MAX_N_CAMERAS 10

/* global variables */
CamContext *cc_all[MAX_N_CAMERAS];
int ncams=0;

int stride, width, height;
unsigned char *raw_pixels;
double buf_wf, buf_hf;
GLuint pbo;
GLuint textureId_all[MAX_N_CAMERAS];
int use_pbo, use_shaders;
int tex_width, tex_height;
size_t PBO_stride;
GLint gl_data_format;

#ifdef USE_GLEW
GLhandleARB glsl_program;
#endif

#define _check_error() {                                                \
    int _check_error_err;                                               \
    _check_error_err = cam_iface_have_error();                          \
    if (_check_error_err != 0) {                                        \
                                                                        \
      fprintf(stderr,"%s:%d %s\n", __FILE__,__LINE__,cam_iface_get_error_string()); \
      exit(1);                                                          \
    }                                                                   \
  }                                                                     \

void setShaders();

void yuv422_to_mono8(const unsigned char *src_pixels, unsigned char *dest_pixels, int width, int height,
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

void yuv422_to_rgba8(const unsigned char *src_pixels, unsigned char *dest_pixels, int width, int height,
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

#define do_copy() {                                       \
  rowstart = dest;                                        \
  for (i=0; i<height; i++) {                              \
    memcpy(rowstart, src + (stride*i), width );           \
    rowstart += dest_stride;                              \
  }                                                       \
}

unsigned char* convert_pixels(unsigned char* src,
                     CameraPixelCoding src_coding,
                     size_t dest_stride,
                     unsigned char* dest, int force_copy) {
  static int gave_error=0;
  unsigned char *src_ptr;
  static int attempted_to_start_glsl_program=0;
  GLubyte* rowstart;
  int i,j;
  int copy_required;
  GLint firstRed;

  copy_required = force_copy || (dest_stride!=stride);
  src_ptr = src;

  switch (src_coding) {
  case CAM_IFACE_MONO8:
    switch (gl_data_format) {
    case GL_LUMINANCE:
      if (copy_required) {
        do_copy();
        return dest;
      }
      return src; /* no conversion necessary*/
      break;
    default:
      fprintf(stderr,"ERROR: will not convert MONO8 image to non-luminance\n");
      exit(1);
      break;
    }
    break;
  case CAM_IFACE_YUV422:
    switch (gl_data_format) {
    case GL_LUMINANCE:
      yuv422_to_mono8(src_ptr, dest, width, height, stride, dest_stride);
      return dest;
      break;
    case GL_RGB:
      yuv422_to_rgb8(src_ptr, dest, width, height, stride, dest_stride);
      return dest;
      break;
    case GL_RGBA:
      yuv422_to_rgba8(src_ptr, dest, width, height, stride, dest_stride);
      return dest;
      break;
    default:
      fprintf(stderr,"ERROR: invalid conversion at line %d\n",__LINE__);
      exit(1);
      break;
    }
    break;
  case CAM_IFACE_RGB8:
    switch (gl_data_format) {
    case GL_RGB:
      if (copy_required) {
        // update data directly on the mapped buffer
        GLubyte* rowstart = dest;
        for (i=0; i<height; i++) {
          memcpy(rowstart, src_ptr, width*3 );
          rowstart += dest_stride;
          src_ptr += stride;
        }
        return dest;
      } else {
        return src; /* no conversion necessary*/
      }
      break;
    case GL_RGBA:
      if (copy_required) {
        // update data directly on the mapped buffer
        GLubyte* rowstart = dest;
        int j;
        for (i=0; i<height; i++) {
          for (j=0; j<width; j++) {
            rowstart[j*4] = src_ptr[j*3];
            rowstart[j*4+1] = src_ptr[j*3+1];
            rowstart[j*4+2] = src_ptr[j*3+2];
            rowstart[j*4+3] = 255;
          }
          rowstart += dest_stride;
          src_ptr += stride;
        }
        return dest;
      } else {
        return src; /* no conversion necessary*/
      }
      break;
    default:
      fprintf(stderr,"ERROR: invalid conversion at line %d\n",__LINE__);
      exit(1);
      break;
    }
    break;
#ifdef USE_GLEW
  case CAM_IFACE_MONO8_BAYER_BGGR:
  case CAM_IFACE_MONO8_BAYER_RGGB:
  case CAM_IFACE_MONO8_BAYER_GRBG:
  case CAM_IFACE_MONO8_BAYER_GBRG:
    //FIXME: add switch (gl_data_format)
    if (!attempted_to_start_glsl_program) {
      setShaders();
      if (use_shaders) {
        firstRed = glGetUniformLocation(glsl_program,"firstRed");
        switch(src_coding) {
        case CAM_IFACE_MONO8_BAYER_BGGR:
          glUniform2f(firstRed,0,0);
          break;
        case CAM_IFACE_MONO8_BAYER_RGGB:
          glUniform2f(firstRed,1,1);
          break;
        case CAM_IFACE_MONO8_BAYER_GRBG:
          glUniform2f(firstRed,0,1);
          break;
        case CAM_IFACE_MONO8_BAYER_GBRG:
        default:
          glUniform2f(firstRed,1,0);
          break;
        }
      } else {
        fprintf(stderr,"ERROR: Failed to start GLSL Bayer program\n");
      }
      attempted_to_start_glsl_program=1;
    }
    do_copy();
    return dest;
    break;
#endif
  case CAM_IFACE_MONO16:
    switch (gl_data_format) {
    case GL_LUMINANCE:
      rowstart = dest;
      for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
          rowstart[j] = (src + (stride*i))[j*2];
        }
        rowstart += dest_stride;
      }
      return dest;
      break;
    default:
      fprintf(stderr,"ERROR: invalid conversion at line %d\n",__LINE__);
      exit(1);
      break;
    }
  default:
    if (!gave_error) {
      fprintf(stderr,"ERROR: unsupported pixel coding %d\n",src_coding);
      gave_error=1;
    }
    if (copy_required) {
      do_copy();
      return dest;
    } else {
      return src; /* no conversion necessary*/
    }
    break;
  }
}

void show_usage(char * cmd) {
  printf("usage: %s [num_frames]\n",cmd);
  printf("  where num_frames can be a number or 'forever'\n");
  exit(1);
}

double next_power_of_2(double f) {
  return pow(2.0,ceil(log(f)/log(2.0)));
}

void initialize_gl_texture() {
  char *buffer;
  int bytes_per_pixel;
  CamContext *cc = cc_all[0];
  GLuint textureId;
  int i;

  if (use_pbo) {
    // align
    PBO_stride = ((unsigned int)width/32)*32;
    if (PBO_stride<(unsigned int)width) PBO_stride+=32;
    tex_width = PBO_stride;
    tex_height = height;

  } else {
    PBO_stride = width;
    tex_width = (int)next_power_of_2(stride);
    tex_height = (int)next_power_of_2(height);
  }

  buf_wf = ((double)(width))/((double)tex_width);
  buf_hf = ((double)(height))/((double)tex_height);

  printf("for %dx%d image, allocating %dx%d texture (fractions: %.2f, %.2f)\n",
         width,height,tex_width,tex_height,buf_wf,buf_hf);

  if ((cc->coding==CAM_IFACE_RGB8) || (cc->coding==CAM_IFACE_YUV422)) {
    bytes_per_pixel=4;
    gl_data_format = GL_RGBA;

    /* This gives only grayscale images ATI fglrx Ubuntu Jaunty, but
       transfers less data:     */

    /*
    bytes_per_pixel=3;
    gl_data_format = GL_RGB;
    */

  } else if (cc->coding==CAM_IFACE_MONO16) {
    bytes_per_pixel=1;
    gl_data_format = GL_LUMINANCE;
  } else {
    bytes_per_pixel=1;
    gl_data_format = GL_LUMINANCE;
  }
  printf("bytes per pixel %d, depth/8 %d\n",bytes_per_pixel,(cc->depth/8));
  PBO_stride = PBO_stride*bytes_per_pixel; // FIXME this pads the rows more than necessary

  if (use_pbo) {
    printf("image stride: %d, PBO stride: %d\n",stride,(int)PBO_stride);
  }

  buffer = malloc( tex_height*PBO_stride );
  if (!buffer) {
    fprintf(stderr,"ERROR: failed to allocate buffer\n");
    exit(1);
  }

  glGenTextures(ncams, &(textureId_all[0]));
for (i=0; i<ncams; i++) {
  textureId = textureId_all[i];
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, /* target */
               0, /* mipmap level */
               GL_RGBA, /* internal format */
               tex_width, tex_height,
               0, /* border */
               gl_data_format, /* format */
               GL_UNSIGNED_BYTE, /* type */
               buffer);
 }
  free(buffer);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void grab_frame(void); /* forward declaration */

void display_pixels(void) {
  GLuint textureId;
  int i;
  int ncols,nrows,col_idx,row_idx;
  float wfrac, hfrac, lowx, highx, lowy, highy;
  if (ncams > 2) {
    nrows = 2;
  } else {
    nrows = 1;
  }
  ncols = (ncams+1) / 2;
  wfrac = 2.0f/ncols;
  hfrac = 2.0f/nrows;

for (i=0; i<ncams; i++) {

  if (i < ncols) {
    row_idx = 0;
  } else {
    row_idx = 1;
  }

  col_idx = i % ncols;
  lowx = col_idx*wfrac-1.0f;
  highx = (col_idx+1)*wfrac-1.0f;
  lowy = row_idx*hfrac-1.0f;
  highy = (row_idx+1)*hfrac-1.0f;

  //  printf("i %d\n col_idx %d, row_idx %d, lowx %f, highx %f, lowy %f, highy %f\n",
  // i,col_idx, row_idx, lowx, highx, lowy, highy );

  textureId = textureId_all[i];

  //glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);

    glNormal3d(0, 0, 1);

    glTexCoord2f(0,0);
    glVertex3f(lowx,highy,0);

    glTexCoord2f(0,buf_hf);
    glVertex3f(lowx,lowy,0);

    glTexCoord2f(buf_wf,buf_hf);
    glVertex3f(highx,lowy,0);

    glTexCoord2f(buf_wf,0);
    glVertex3f(highx,highy,0);

    glEnd();

    glutSwapBuffers();
}
}

char *textFileRead(char *fn) {


        FILE *fp;
        char *content = NULL;

        int count;
        fp = fopen(fn, "rt");
	if (fp == NULL) {
	  return NULL;
	}

	fseek(fp, 0, SEEK_END);
        count = ftell(fp);

        fseek(fp, 0, SEEK_SET);

        if (fn != NULL) {

                if (fp != NULL) {


                        if (count > 0) {
                                content = (char *)malloc(sizeof(char) * (count+1));
                                count = fread(content,sizeof(char),count,fp);
                                content[count] = '\0';
                        }
                        fclose(fp);
                }
        }
        return content;
}

#ifdef USE_GLEW
        void printShaderInfoLog(GLuint obj)
        {
            int infologLength = 0;
            int charsWritten  = 0;
            char *infoLog;

                glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

            if (infologLength > 0)
            {
                infoLog = (char *)malloc(infologLength);
                glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
                        printf("%s\n",infoLog);
                free(infoLog);
            }
        }

        void printProgramInfoLog(GLuint obj)
        {
            int infologLength = 0;
            int charsWritten  = 0;
            char *infoLog;

                glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

            if (infologLength > 0)
            {
                infoLog = (char *)malloc(infologLength);
                glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
                        printf("%s\n",infoLog);
                free(infoLog);
            }
        }

void setShaders() {

  const char * vv;
  const char * ff;
                char *vs,*fs;
                GLint status;
                GLhandleARB vertex_program,fragment_program;
                GLint sourceSize, shader_texture_source;

                vertex_program = glCreateShader(GL_VERTEX_SHADER);
                fragment_program = glCreateShader(GL_FRAGMENT_SHADER);

                vs = textFileRead(SHADER_DIR "demosaic.vrt");
                if (vs==NULL) {
                  fprintf(stderr,"ERROR: failed to read vertex shader %s\n",
			  SHADER_DIR "demosaic.vrt");
                  use_shaders = 0;
                  return;
                }
                fs = textFileRead(SHADER_DIR "demosaic.frg");
                if (fs==NULL) {
                  fprintf(stderr,"ERROR: failed to read fragment shader %s\n",
			  SHADER_DIR "demosaic.frg");
                  use_shaders = 0;
                  return;
                }

                vv = vs;
                ff = fs;

                glShaderSource(vertex_program, 1, &vv,NULL);
                glShaderSource(fragment_program, 1, &ff,NULL);

                free(vs);free(fs);

                glCompileShader(vertex_program);
                glGetShaderiv(vertex_program,GL_COMPILE_STATUS,&status);
                if (status!=GL_TRUE) {
                  fprintf(stderr,
                          "ERROR: GLSL vertex shader compile error, disabling shaders\n");
                  use_shaders = 0;
                  return;
                }

                glCompileShader(fragment_program);
                glGetShaderiv(fragment_program,GL_COMPILE_STATUS,&status);
                if (status!=GL_TRUE) {
                  fprintf(stderr,
                          "ERROR: GLSL fragment shader compile error, disabling shaders\n");
                  use_shaders = 0;
                  return;
                }

                printShaderInfoLog(vertex_program);
                printShaderInfoLog(fragment_program);

                glsl_program = glCreateProgram();
                glAttachShader(glsl_program,vertex_program);
                glAttachShader(glsl_program,fragment_program);
                glLinkProgram(glsl_program);

                printProgramInfoLog(glsl_program);

                glGetProgramiv(glsl_program,GL_LINK_STATUS,&status);
                if (status!=GL_TRUE) {
                  fprintf(stderr,"ERROR: GLSL link error, disabling shaders\n");
                  use_shaders = 0;
                  return;
                }

                glUseProgram(glsl_program);
                printf("GLSL shaders in use\n");


                sourceSize = glGetUniformLocation(glsl_program,"sourceSize");
                shader_texture_source = glGetUniformLocation(glsl_program,"source");

                glUniform4f(sourceSize,
                            PBO_stride,height,
                            1.0/PBO_stride,1.0/height);
                glUniform1i(shader_texture_source, 0);
}
#endif  /* ifdef USE_GLEW */

int main(int argc, char** argv) {
  int device_number,num_buffers;

  int buffer_size;
  int num_modes, num_props, num_trigger_modes;
  char mode_string[255];
  int i,mode_number;
  CameraPropertyInfo cam_props;
  long prop_value;
  int prop_auto;
  int left, top;
  int orig_left, orig_top, orig_width, orig_height, orig_stride;
  cam_iface_constructor_func_t new_CamContext;
  Camwire_id cam_info_struct;
  CamContext *cc;

  glutInit(&argc, argv);

  cam_iface_startup_with_version_check();
  _check_error();

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

for (device_number = 0; device_number < ncams; device_number++) {

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
  cc_all[device_number] = new_CamContext(device_number,num_buffers,mode_number);
  _check_error();

  cc = cc_all[device_number];

  CamContext_get_frame_roi(cc, &left, &top, &width, &height);
  _check_error();

  stride = width*cc->depth/8;
  printf("raw image width: %d, stride: %d\n",width,stride);

  if (device_number==0) {
    orig_left = left;
    orig_top = top;
    orig_width = width;
    orig_height = height;
    orig_stride = stride;
  } else {
    if (!((orig_left == left) &&
	  (orig_top == top) &&
	  (orig_width == width) &&
	  (orig_height == height) &&
	  (orig_stride = stride))) {
      fprintf(stderr,"not all cameras have same shape\n");
      exit(1);
    }
  }
}

  glutInitWindowPosition(-1,-1);
  glutInitWindowSize(width, height);
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
  glutCreateWindow("libcamiface liveview");

  use_shaders=0;

#ifdef USE_GLEW
  glewInit();
  if (glewIsSupported("GL_VERSION_2_0 "
                      "GL_ARB_pixel_buffer_object")) {
    printf("PBO enabled\n");
    use_pbo=1;

    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader) {
      printf("GLSL shaders present\n");
      use_shaders=1;
    }

  } else {
    printf("GLEW available, but no pixel buffer support -- not using PBO\n");
    use_pbo=0;
  }
#else
  printf("GLEW not available -- not using PBO\n");
  use_pbo=0;
#endif

  initialize_gl_texture();

#ifdef USE_GLEW
  if (use_pbo) {
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
                 PBO_stride*tex_height, 0, GL_STREAM_DRAW);
  }
#endif  /* ifdef USE_GLEW */

  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

for (device_number=0; device_number < ncams; device_number++) {
  cc = cc_all[device_number];

  CamContext_get_num_framebuffers(cc,&num_buffers);
  printf("allocated %d buffers\n",num_buffers);

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
      printf("  %s: %ld\n",cam_props.name,prop_value);
    } else {
      printf("  %s: not present\n",cam_props.name);
    }
  }

  CamContext_get_buffer_size(cc,&buffer_size);
  _check_error();

  if (buffer_size == 0) {
    fprintf(stderr,"buffer size was 0 in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }
 }

#define USE_COPY
#ifdef USE_COPY
  raw_pixels = (unsigned char *)malloc( buffer_size );
  if (raw_pixels==NULL) {
    fprintf(stderr,"couldn't allocate memory in %s, line %d\n",__FILE__,__LINE__);
    exit(1);
  }
#endif

for (device_number=0; device_number < ncams; device_number++) {
  cc = cc_all[device_number];
  CamContext_start_camera(cc);
  _check_error();
}

  printf("will now run forever. press Ctrl-C to interrupt\n");

for (device_number=0; device_number < ncams; device_number++) {
  cc = cc_all[device_number];

  CamContext_get_num_trigger_modes( cc, &num_trigger_modes );
  _check_error();

  printf("trigger modes:\n");
  for (i =0; i<num_trigger_modes; i++) {
    CamContext_get_trigger_mode_string( cc, i, mode_string, 255 );
    printf("  %d: %s\n",i,mode_string);
  }
  printf("\n");
 }

  glutDisplayFunc(display_pixels); /* set the display callback */
  glutIdleFunc(grab_frame); /* set the idle callback */

  glutMainLoop();
  printf("\n");

for (device_number=0; device_number < ncams; device_number++) {
  cc = cc_all[device_number];
  delete_CamContext(cc);
  _check_error();
}

  cam_iface_shutdown();
  _check_error();

#ifdef USE_COPY
  free(raw_pixels);
#endif

  return 0;
}

/* Send the data to OpenGL. Use the fastest possible method. */

void upload_image_data_to_opengl(unsigned char* raw_image_data,
                                 CameraPixelCoding coding,
				 int device_number) {
  unsigned char * gl_image_data;
  static unsigned char* show_pixels=NULL;
  GLuint textureId;
  GLubyte* ptr;

  textureId = textureId_all[device_number];

  if (use_pbo) {
#ifdef USE_GLEW
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width, tex_height, gl_data_format, GL_UNSIGNED_BYTE, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, PBO_stride*tex_height, 0, GL_STREAM_DRAW_ARB);
    ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    if(ptr) {
      convert_pixels(raw_image_data, coding, PBO_stride, ptr, 1);
      glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer
    }
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
#endif  /* ifdef USE_GLEW */
  } else {

    if (show_pixels==NULL) {
      /* allocate memory */
      show_pixels = (unsigned char *)malloc( PBO_stride*height );
      if (show_pixels==NULL) {
        fprintf(stderr,"couldn't allocate memory in %s, line %d\n",__FILE__,__LINE__);
        exit(1);
      }
    }

    gl_image_data = convert_pixels(raw_image_data, coding, PBO_stride, show_pixels, 0);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, /* target */
                    0, /* mipmap level */
                    0, /* x offset */
                    0, /* y offset */
                    width,
                    height,
                    gl_data_format, /* data format */
                    GL_UNSIGNED_BYTE, /* data type */
                    gl_image_data);

  }
}

/* grab_frame() is the idle-time callback function. It grabs an image,
   sends it to OpenGL, and tells GLUT to do the display function
   callback. */

void grab_frame(void) {
  int errnum;
  CamContext *cc;
  static int next_device_number=0;
  int data_ok = 0;

#ifdef USE_COPY
    cc = cc_all[next_device_number];

    CamContext_grab_next_frame_blocking(cc,raw_pixels,-1); // block forever
    errnum = cam_iface_have_error();
    if (errnum == CAM_IFACE_FRAME_TIMEOUT) {
      cam_iface_clear_error();
      return; // wait again
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
      data_ok = 1;
    }

    next_device_number++;
    next_device_number = next_device_number % ncams;
    if (data_ok) {
      upload_image_data_to_opengl(raw_pixels,cc->coding,next_device_number);
    }

#else
    CamContext_point_next_frame_blocking(cc,&raw_pixels,-1.0f);
    _check_error();

    upload_image_data_to_opengl(raw_pixels,cc->coding,next_device_number);

    CamContext_unpoint_frame(cc);
    _check_error();
#endif

    glutPostRedisplay(); /* trigger display redraw */

}
