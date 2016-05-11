/*
 * GStreamer Plugins VR
 * Copyright (C) 2016 Lubosz Sarnecki <lubosz.sarnecki@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define GST_USE_UNSTABLE_API
#include <gst/gl/gl.h>

#include "gst3dcamera.h"

#define GST_CAT_DEFAULT gst_3d_camera_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCamera, gst_3d_camera, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_camera_debug, "3dcamera", 0, "camera"));


static void init_hmd(Gst3DCamera * self) {
  self->hmd_context = ohmd_ctx_create ();
  int num_devices = ohmd_ctx_probe (self->hmd_context);
  if (num_devices < 0) {
    GST_ERROR ("failed to probe devices: %s\n",
        ohmd_ctx_get_error (self->hmd_context));
    return;
  }

  GST_DEBUG ("We have %d deivces.\n", num_devices);

	for(int i = 0; i < num_devices; i++){
		GST_DEBUG("device %d", i);
		GST_DEBUG("  vendor:  %s", ohmd_list_gets(self->hmd_context, i, OHMD_VENDOR));
		GST_DEBUG("  product: %s", ohmd_list_gets(self->hmd_context, i, OHMD_PRODUCT));
		GST_DEBUG("  path:    %s", ohmd_list_gets(self->hmd_context, i, OHMD_PATH));
	}

  self->device = ohmd_list_open_device (self->hmd_context, 0);

  if (!self->device) {
    GST_ERROR ("failed to open device: %s\n",
        ohmd_ctx_get_error (self->hmd_context));
    return;
  }
  
  int screen_width;
  int screen_height;
  
  float inch = 2.54;
  
  ohmd_device_geti (self->device, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &screen_width);
  ohmd_device_geti (self->device, OHMD_SCREEN_VERTICAL_RESOLUTION, &screen_height);
  
  GST_DEBUG("HMD screen resolution: %dx%d", screen_width, screen_height);
  
  float screen_width_physical;
  float screen_height_physical;
  
  ohmd_device_getf (self->device, OHMD_SCREEN_HORIZONTAL_SIZE, &screen_width_physical);
  ohmd_device_getf (self->device, OHMD_SCREEN_VERTICAL_SIZE, &screen_height_physical);
  
  GST_DEBUG("Physical HMD screen dimensions: %.3fx%.3fcm",
    screen_width_physical * 100.0, screen_height_physical * 100.0);

  float x_pixels_per_cm = (float) screen_width / (screen_width_physical * 100.0);
  float y_pixels_per_cm = (float) screen_height / (screen_height_physical * 100.0);

  GST_DEBUG("HMD DPI %f x %f", x_pixels_per_cm / inch, y_pixels_per_cm / inch);

  float lens_x_separation;
  float lens_y_position;
  
  ohmd_device_getf (self->device, OHMD_LENS_HORIZONTAL_SEPARATION, &lens_x_separation);
  ohmd_device_getf (self->device, OHMD_LENS_VERTICAL_POSITION, &lens_y_position);
  
  GST_DEBUG("Horizontal Lens Separation: %.3fcm", lens_x_separation * 100.0);
  GST_DEBUG("Vertical Lens Position: %.3fcm", lens_y_position * 100.0);

  float left_fov;
  float left_aspect;
  float right_fov;
  float right_aspect;
  
  ohmd_device_getf (self->device, OHMD_LEFT_EYE_FOV, &left_fov);
  ohmd_device_getf (self->device, OHMD_LEFT_EYE_ASPECT_RATIO, &left_aspect);
  ohmd_device_getf (self->device, OHMD_RIGHT_EYE_FOV, &right_fov);
  ohmd_device_getf (self->device, OHMD_RIGHT_EYE_ASPECT_RATIO, &right_aspect);
  
  GST_DEBUG("FOV (left/right): %f %f", left_fov, right_fov);
  GST_DEBUG("Aspect Ratio (left/right): %f %f", left_aspect, right_aspect);

  float interpupillary_distance;
  float zfar;
  float znear;
  
  ohmd_device_getf (self->device, OHMD_EYE_IPD, &interpupillary_distance);
  GST_DEBUG("interpupillary_distance %.3fcm", interpupillary_distance * 100.0);
  
  //self->eye_separation = interpupillary_distance * 100.0 / 2.0;
  self->eye_separation = 0.65;

  ohmd_device_getf (self->device, OHMD_PROJECTION_ZNEAR, &znear);
  ohmd_device_getf (self->device, OHMD_PROJECTION_ZFAR, &zfar);
  
  GST_DEBUG("znear %f, zfar %f", znear, zfar);  

  /*
  float kappa[6];
  ohmd_device_getf (self->device, OHMD_DISTORTION_K, kappa);
  GST_DEBUG("Kappa: %f, %f, %f, %f, %f, %f",
             kappa[0], kappa[1], kappa[2], kappa[3], kappa[4], kappa[5]);
  float position[3];
  ohmd_device_getf (self->device, OHMD_POSITION_VECTOR, position);
    GST_DEBUG("position: %f, %f, %f",
             position[0], position[1], position[2]);

  */
}


void
gst_3d_camera_init (Gst3DCamera * self)
{
  self->fov = 90;
  self->aspect = 1.0;
  self->znear = 0.1;
  self->zfar = 100;
  self->hmd_context = NULL;
  self->device = NULL;
  
  graphene_vec3_init (&self->eye, 0.f, 0.f, 1.f);
  graphene_vec3_init (&self->center, 0.f, 0.f, 0.f);
  graphene_vec3_init (&self->up, 0.f, 1.f, 0.f);
  
  init_hmd(self);
}

Gst3DCamera *
gst_3d_camera_new (void)
{
  Gst3DCamera *camera = g_object_new (GST_3D_TYPE_CAMERA, NULL);
  return camera;
}

static void
gst_3d_camera_finalize (GObject * object)
{
  Gst3DCamera *self = GST_3D_CAMERA (object);
  g_return_if_fail (self != NULL);
  
  G_OBJECT_CLASS (gst_3d_camera_parent_class)->finalize (object);
}

static void
gst_3d_camera_class_init (Gst3DCameraClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_camera_finalize;
}

graphene_matrix_t get_hmd_matrix(Gst3DCamera * self, ohmd_float_value type) {
  float matrix[16];
  graphene_matrix_t hmd_matrix;
  
  // set hmd rotation, for left eye.
  ohmd_device_getf (self->device, type, matrix);
  graphene_matrix_init_from_float (&hmd_matrix, matrix);
  return hmd_matrix;
} 

void gst_3d_camera_inc_eye_sep(Gst3DCamera * self) {
  self->eye_separation += .1;
  GST_DEBUG("separation: %f", self->eye_separation);
}

void gst_3d_camera_dec_eye_sep(Gst3DCamera * self) {
  self->eye_separation -= .1;
  GST_DEBUG("separation: %f", self->eye_separation);
}

void gst_3d_camera_update_view_from_quaternion(Gst3DCamera * self) {
  g_return_if_fail(self->hmd_context);
  g_return_if_fail(self->device);

  ohmd_ctx_update (self->hmd_context);

  graphene_matrix_t left_eye_model_view;
  graphene_matrix_t left_eye_projection = get_hmd_matrix(self, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);
  

  graphene_matrix_t right_eye_model_view;
  graphene_matrix_t right_eye_projection = get_hmd_matrix(self, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  float quaternion[4];
  ohmd_device_getf (self->device, OHMD_ROTATION_QUAT, quaternion);
  
  graphene_quaternion_t quat;
  graphene_quaternion_init (&quat,
                          quaternion[0],
                          -quaternion[1],
                          quaternion[2],
                          quaternion[3]);
  
  graphene_point3d_t left_eye;
  graphene_point3d_init (&left_eye, +self->eye_separation, 0, 0);
  graphene_matrix_t translate_left;
  graphene_matrix_init_translate (&translate_left, &left_eye);

  graphene_point3d_t rigth_eye;
  graphene_point3d_init (&rigth_eye, -self->eye_separation, 0, 0);
  graphene_matrix_t translate_right;
  graphene_matrix_init_translate (&translate_right, &rigth_eye);
  
  graphene_quaternion_to_matrix (&quat, &right_eye_model_view);
  graphene_matrix_multiply (&right_eye_model_view, &translate_right, &right_eye_model_view);

  graphene_quaternion_to_matrix (&quat, &left_eye_model_view);
  graphene_matrix_multiply (&left_eye_model_view, &translate_left, &left_eye_model_view);
      
  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection, 
      &self->right_vp_matrix);
}

void
gst_3d_camera_update_view_from_matrix (Gst3DCamera * self)
{
  g_return_if_fail(self->hmd_context);
  g_return_if_fail(self->device);

  ohmd_ctx_update (self->hmd_context);

  graphene_matrix_t left_eye_model_view = get_hmd_matrix(self, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t left_eye_projection = get_hmd_matrix(self, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);
  
  graphene_matrix_t right_eye_model_view = get_hmd_matrix(self, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t right_eye_projection = get_hmd_matrix(self, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);
      
  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection, &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection, &self->right_vp_matrix);
}

void gst_3d_camera_update_view(Gst3DCamera * self) {
  gst_3d_camera_update_view_from_quaternion (self);
  // gst_3d_camera_update_view_from_matrix (self);
}
