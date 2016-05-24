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

#include <graphene.h>

#include "gst3dhmd.h"

#define GST_CAT_DEFAULT gst_3d_hmd_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DHmd, gst_3d_hmd, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_hmd_debug, "3dhmd", 0, "hmd"));

Gst3DHmd *
gst_3d_hmd_new (void)
{
  Gst3DHmd *hmd = g_object_new (GST_3D_TYPE_HMD, NULL);
  return hmd;
}

static void
gst_3d_hmd_finalize (GObject * object)
{
  Gst3DHmd *self = GST_3D_HMD (object);
  g_return_if_fail (self != NULL);
  G_OBJECT_CLASS (gst_3d_hmd_parent_class)->finalize (object);
}

static void
gst_3d_hmd_class_init (Gst3DHmdClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_hmd_finalize;
}

void
gst_3d_hmd_init (Gst3DHmd * self)
{

  self->device = NULL;

  self->hmd_context = ohmd_ctx_create ();
  int num_devices = ohmd_ctx_probe (self->hmd_context);
  if (num_devices < 0) {
    GST_ERROR ("failed to probe devices: %s\n",
        ohmd_ctx_get_error (self->hmd_context));
    return;
  }

  GST_DEBUG ("We have %d deivces.\n", num_devices);

  for (int i = 0; i < num_devices; i++) {
    GST_DEBUG ("device %d", i);
    GST_DEBUG ("  vendor:  %s", ohmd_list_gets (self->hmd_context, i,
            OHMD_VENDOR));
    GST_DEBUG ("  product: %s", ohmd_list_gets (self->hmd_context, i,
            OHMD_PRODUCT));
    GST_DEBUG ("  path:    %s", ohmd_list_gets (self->hmd_context, i,
            OHMD_PATH));
  }

  int picked_device = 0;

  self->device = ohmd_list_open_device (self->hmd_context, picked_device);

  if (!self->device) {
    GST_ERROR ("Failed to open device: %s\n",
        ohmd_ctx_get_error (self->hmd_context));
    GST_ERROR ("  vendor:  %s", ohmd_list_gets (self->hmd_context,
            picked_device, OHMD_VENDOR));
    GST_ERROR ("  product: %s", ohmd_list_gets (self->hmd_context,
            picked_device, OHMD_PRODUCT));
    GST_ERROR ("  path:    %s", ohmd_list_gets (self->hmd_context,
            picked_device, OHMD_PATH));
    GST_ERROR ("Make sure you have access rights and a working rules "
        "file for your headset in /usr/lib/udev/rules.d");
    return;
  }

  int screen_width;
  int screen_height;

  float inch = 2.54;

  ohmd_device_geti (self->device, OHMD_SCREEN_HORIZONTAL_RESOLUTION,
      &screen_width);
  ohmd_device_geti (self->device, OHMD_SCREEN_VERTICAL_RESOLUTION,
      &screen_height);

  GST_DEBUG ("HMD screen resolution: %dx%d", screen_width, screen_height);

  float screen_width_physical;
  float screen_height_physical;

  ohmd_device_getf (self->device, OHMD_SCREEN_HORIZONTAL_SIZE,
      &screen_width_physical);
  ohmd_device_getf (self->device, OHMD_SCREEN_VERTICAL_SIZE,
      &screen_height_physical);

  GST_DEBUG ("Physical HMD screen dimensions: %.3fx%.3fcm",
      screen_width_physical * 100.0, screen_height_physical * 100.0);

  float x_pixels_per_cm =
      (float) screen_width / (screen_width_physical * 100.0);
  float y_pixels_per_cm =
      (float) screen_height / (screen_height_physical * 100.0);

  GST_DEBUG ("HMD DPI %f x %f", x_pixels_per_cm / inch, y_pixels_per_cm / inch);

  float lens_x_separation;
  float lens_y_position;

  ohmd_device_getf (self->device, OHMD_LENS_HORIZONTAL_SEPARATION,
      &lens_x_separation);
  ohmd_device_getf (self->device, OHMD_LENS_VERTICAL_POSITION,
      &lens_y_position);

  GST_DEBUG ("Horizontal Lens Separation: %.3fcm", lens_x_separation * 100.0);
  GST_DEBUG ("Vertical Lens Position: %.3fcm", lens_y_position * 100.0);

  float left_fov;
  float left_aspect;
  float right_fov;
  float right_aspect;

  ohmd_device_getf (self->device, OHMD_LEFT_EYE_FOV, &left_fov);
  ohmd_device_getf (self->device, OHMD_LEFT_EYE_ASPECT_RATIO, &left_aspect);
  ohmd_device_getf (self->device, OHMD_RIGHT_EYE_FOV, &right_fov);
  ohmd_device_getf (self->device, OHMD_RIGHT_EYE_ASPECT_RATIO, &right_aspect);

  GST_DEBUG ("FOV (left/right): %f %f", left_fov, right_fov);
  GST_DEBUG ("Aspect Ratio (left/right): %f %f", left_aspect, right_aspect);

  float interpupillary_distance;
  float zfar;
  float znear;

  ohmd_device_getf (self->device, OHMD_EYE_IPD, &interpupillary_distance);
  GST_DEBUG ("interpupillary_distance %.3fcm", interpupillary_distance * 100.0);

  //self->eye_separation = interpupillary_distance * 100.0 / 2.0;
  self->eye_separation = 0.65;

  ohmd_device_getf (self->device, OHMD_PROJECTION_ZNEAR, &znear);
  ohmd_device_getf (self->device, OHMD_PROJECTION_ZFAR, &zfar);

  GST_DEBUG ("znear %f, zfar %f", znear, zfar);

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

graphene_matrix_t
gst_3d_hmd_get_matrix (Gst3DHmd * self, ohmd_float_value type)
{
  float matrix[16];
  graphene_matrix_t hmd_matrix;

  // set hmd rotation, for left eye.
  ohmd_device_getf (self->device, type, matrix);
  graphene_matrix_init_from_float (&hmd_matrix, matrix);
  return hmd_matrix;
}

graphene_quaternion_t
gst_3d_hmd_get_quaternion (Gst3DHmd * self)
{
  float quaternion[4];
  ohmd_device_getf (self->device, OHMD_ROTATION_QUAT, quaternion);

  graphene_quaternion_t quat;
  graphene_quaternion_init (&quat,
      quaternion[0], -quaternion[1], quaternion[2], quaternion[3]);
  return quat;
}

void
gst_3d_hmd_update (Gst3DHmd * self)
{
  g_return_if_fail (self->hmd_context);
  g_return_if_fail (self->device);
  ohmd_ctx_update (self->hmd_context);
}
