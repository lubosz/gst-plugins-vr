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
#include <stdio.h>

#define GST_USE_UNSTABLE_API
#include <gst/gl/gl.h>

#include "gst3dcamera_arcball.h"
#include "gst3dglm.h"

#define GST_CAT_DEFAULT gst_3d_camera_arcball_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCameraArcball, gst_3d_camera_arcball,
    GST_TYPE_OBJECT, GST_DEBUG_CATEGORY_INIT (gst_3d_camera_arcball_debug,
        "3dcamera_arcball", 0, "camera_arcball"));


void
gst_3d_camera_arcball_init (Gst3DCameraArcball * self)
{
  self->fov = 45.0;
  self->aspect = 4.0 / 3.0;
  self->znear = 0.1;
  self->zfar = 100;

  self->center_distance = 2.5;
  self->scroll_speed = 0.05;
  self->rotation_speed = 0.002;
  self->cursor_last_x = 0;
  self->cursor_last_y = 0;

  self->theta = 5.0;
  self->phi = -5.0;

  graphene_vec3_init (&self->eye, 0.f, 0.f, 1.f);
  graphene_vec3_init (&self->center, 0.f, 0.f, 0.f);
  graphene_vec3_init (&self->up, 0.f, 1.f, 0.f);
}

Gst3DCameraArcball *
gst_3d_camera_arcball_new (void)
{
  Gst3DCameraArcball *camera_arcball =
      g_object_new (GST_3D_TYPE_CAMERA_ARCBALL, NULL);
  return camera_arcball;
}

static void
gst_3d_camera_arcball_finalize (GObject * object)
{
  Gst3DCameraArcball *self = GST_3D_CAMERA_ARCBALL (object);
  g_return_if_fail (self != NULL);

  G_OBJECT_CLASS (gst_3d_camera_arcball_parent_class)->finalize (object);
}

static void
gst_3d_camera_arcball_class_init (Gst3DCameraArcballClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_camera_arcball_finalize;
}


void
gst_3d_camera_arcball_translate (Gst3DCameraArcball * self, float z)
{
  self->center_distance += z * self->scroll_speed;
  GST_DEBUG ("center distance: %f", self->center_distance);
  gst_3d_camera_arcball_update_view (self);
}

void
gst_3d_camera_arcball_rotate (Gst3DCameraArcball * self, float x, float y)
{
  self->theta += y * self->rotation_speed;
  self->phi += x * self->rotation_speed;
  GST_DEBUG ("theta: %f phi: %f", self->theta, self->phi);
  gst_3d_camera_arcball_update_view (self);
}

void
gst_3d_camera_arcball_update_view (Gst3DCameraArcball * self)
{
  // float radius = exp (self->center_distance);
  float radius = self->center_distance;

  graphene_vec3_init (&self->eye,
      radius * sin (self->theta) * cos (self->phi),
      radius * -cos (self->theta),
      radius * sin (self->theta) * sin (self->phi));

  graphene_matrix_t projection_matrix;
  graphene_matrix_init_perspective (&projection_matrix,
      self->fov, self->aspect, self->znear, self->zfar);

  /*
     graphene_matrix_t view_matrix;
     graphene_matrix_init_look_at (&view_matrix, &self->eye, &self->center,
     &self->up);
   */

  graphene_matrix_t view_matrix =
      gst_3d_glm_look_at (&self->eye, &self->center, &self->up);

  graphene_matrix_multiply (&view_matrix, &projection_matrix, &self->mvp);
}
