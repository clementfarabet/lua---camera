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

#include "cam_iface.h"
CAM_IFACE_API void delete_CamContext(CamContext*this) {
  this->vmt->destruct(this);
}

CAM_IFACE_API void CamContext_CamContext(CamContext *this,int device_number, int NumImageBuffers,
                           int mode_number ) {
  // Must call derived class to make instance.
  this->vmt = NULL;
}

CAM_IFACE_API void CamContext_close(struct CamContext *this) {
  this->vmt->close(this);
}

CAM_IFACE_API void CamContext_start_camera(CamContext *this) {
  this->vmt->start_camera(this);
}

CAM_IFACE_API void CamContext_stop_camera(CamContext *this) {
  this->vmt->stop_camera(this);
}

CAM_IFACE_API void CamContext_get_num_camera_properties(CamContext *this,
                                          int* num_properties){
  this->vmt->get_num_camera_properties(this,num_properties);
}

CAM_IFACE_API void CamContext_get_camera_property_info(CamContext *this,
                                         int property_number,
                                         CameraPropertyInfo *info){
  this->vmt->get_camera_property_info(this,property_number,info);
}

CAM_IFACE_API void CamContext_get_camera_property(CamContext *this,
                                    int property_number,
                                    long* Value,
                                    int* Auto){
  this->vmt->get_camera_property(this,property_number,Value,Auto);
}

CAM_IFACE_API void CamContext_set_camera_property(CamContext *this,
                                    int property_number,
                                    long Value,
                                    int Auto) {
  this->vmt->set_camera_property(this,property_number,Value,Auto);
}

CAM_IFACE_API void CamContext_grab_next_frame_blocking(CamContext *this, unsigned char* out_bytes, float timeout){
  this->vmt->grab_next_frame_blocking(this,out_bytes,timeout);
}
CAM_IFACE_API void CamContext_grab_next_frame_blocking_with_stride(CamContext *this, unsigned char* out_bytes, intptr_t stride0, float timeout){
  this->vmt->grab_next_frame_blocking_with_stride(this,out_bytes,stride0,timeout);
}
CAM_IFACE_API void CamContext_point_next_frame_blocking(CamContext *this, unsigned char** buf_ptr, float timeout){
  this->vmt->point_next_frame_blocking(this,buf_ptr,timeout);
}
CAM_IFACE_API void CamContext_unpoint_frame(CamContext *this){
  this->vmt->unpoint_frame(this);
}
CAM_IFACE_API void CamContext_get_last_timestamp( CamContext *this,
                                    double* timestamp ){
  this->vmt->get_last_timestamp(this,timestamp);
}
CAM_IFACE_API void CamContext_get_last_framenumber( CamContext *this,
                                      unsigned long* framenumber ){
  this->vmt->get_last_framenumber(this,framenumber);
}
CAM_IFACE_API void CamContext_get_num_trigger_modes( CamContext *this,
                                       int *num_exposure_modes ){
  this->vmt->get_num_trigger_modes(this,num_exposure_modes);
}
CAM_IFACE_API void CamContext_get_trigger_mode_string( CamContext *this,
                                         int exposure_mode_number,
                                         char* exposure_mode_string, //output parameter
                                         int exposure_mode_string_maxlen){
  this->vmt->get_trigger_mode_string(this,exposure_mode_number,exposure_mode_string,exposure_mode_string_maxlen);
}
CAM_IFACE_API void CamContext_get_trigger_mode_number( CamContext *this,
                                         int *exposure_mode_number ){
  this->vmt->get_trigger_mode_number(this,exposure_mode_number);
}
CAM_IFACE_API void CamContext_set_trigger_mode_number( CamContext *this,
                                         int exposure_mode_number ){
  this->vmt->set_trigger_mode_number(this,exposure_mode_number);
}
CAM_IFACE_API void CamContext_get_frame_roi( CamContext *this,
                               int *left, int *top, int *width, int *height ){
  this->vmt->get_frame_roi(this,left,top,width,height);
}
CAM_IFACE_API void CamContext_set_frame_roi( CamContext *this,
                               int left, int top, int width, int height ){
  this->vmt->set_frame_roi(this,left,top, width, height);
}
CAM_IFACE_API void CamContext_get_max_frame_size( CamContext *this,
                                    int *width,
                                    int *height ){
  this->vmt->get_max_frame_size(this,width,height);
}
CAM_IFACE_API void CamContext_get_buffer_size( CamContext *this,
                                 int *size){
  this->vmt->get_buffer_size(this,size);
}
CAM_IFACE_API void CamContext_get_framerate( CamContext *this,
                               float *framerate ) {
  this->vmt->get_framerate(this,framerate);
}
CAM_IFACE_API void CamContext_set_framerate( CamContext *this,
                               float framerate ){
  this->vmt->set_framerate(this,framerate);
}
CAM_IFACE_API void CamContext_get_num_framebuffers( CamContext *this,
                                      int *num_framebuffers ){
  this->vmt->get_num_framebuffers(this,num_framebuffers);
}
CAM_IFACE_API void CamContext_set_num_framebuffers( CamContext *this,
                                      int num_framebuffers ){
  this->vmt->set_num_framebuffers(this,num_framebuffers);
}
