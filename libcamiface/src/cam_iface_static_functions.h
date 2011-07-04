/* internal structures for mega backend implementation */

#ifdef __cplusplus
extern "C" {
#endif

CAM_IFACE_API const char *BACKEND_METHOD(cam_iface_get_driver_name)(void);

CAM_IFACE_API void BACKEND_METHOD(cam_iface_clear_error)(void);
CAM_IFACE_API int BACKEND_METHOD(cam_iface_have_error)(void);
CAM_IFACE_API const char * BACKEND_METHOD(cam_iface_get_error_string)(void);
CAM_IFACE_API void BACKEND_METHOD(cam_iface_startup)(void);
CAM_IFACE_API void BACKEND_METHOD(cam_iface_shutdown)(void);
CAM_IFACE_API int BACKEND_METHOD(cam_iface_get_num_cameras)(void);
CAM_IFACE_API void BACKEND_METHOD(cam_iface_get_camera_info)(int device_number, Camwire_id *out_camid);
CAM_IFACE_API void BACKEND_METHOD(cam_iface_get_num_modes)(int device_number, int *num_modes);
CAM_IFACE_API void BACKEND_METHOD(cam_iface_get_mode_string)(int device_number,
							     int mode_number,
							     char* mode_string,
							     int mode_string_maxlen);

CAM_IFACE_API cam_iface_constructor_func_t BACKEND_METHOD(cam_iface_get_constructor_func)(int device_number);

#ifdef __cplusplus
} // closes: extern "C"
#endif
