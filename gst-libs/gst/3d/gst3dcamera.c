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

#include "gst3dcamera.h"
#include "gst3dglm.h"

#define GST_CAT_DEFAULT gst_3d_camera_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCamera, gst_3d_camera, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_camera_debug, "3dcamera", 0, "camera"));


void
gst_3d_camera_init (Gst3DCamera * self)
{
  self->fov = 45.0;
  self->aspect = 4.0 / 3.0;
  self->znear = 0.1;
  self->zfar = 100;
  self->hmd = gst_3d_hmd_new ();
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

void
gst_3d_camera_inc_eye_sep (Gst3DCamera * self)
{
  self->hmd->eye_separation += .1;
  GST_DEBUG ("separation: %f", self->hmd->eye_separation);
}

void
gst_3d_camera_dec_eye_sep (Gst3DCamera * self)
{
  self->hmd->eye_separation -= .1;
  GST_DEBUG ("separation: %f", self->hmd->eye_separation);
}

void
gst_3d_camera_update_view_from_quaternion (Gst3DCamera * self)
{
  gst_3d_hmd_update (self->hmd);

  graphene_matrix_t left_eye_model_view;
  graphene_matrix_t left_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);


  graphene_matrix_t right_eye_model_view;
  graphene_matrix_t right_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  graphene_point3d_t left_eye;
  graphene_point3d_init (&left_eye, +self->hmd->eye_separation, 0, 0);
  graphene_matrix_t translate_left;
  graphene_matrix_init_translate (&translate_left, &left_eye);

  graphene_point3d_t rigth_eye;
  graphene_point3d_init (&rigth_eye, -self->hmd->eye_separation, 0, 0);
  graphene_matrix_t translate_right;
  graphene_matrix_init_translate (&translate_right, &rigth_eye);

  graphene_quaternion_t quat = gst_3d_hmd_get_quaternion (self->hmd);

  graphene_quaternion_to_matrix (&quat, &right_eye_model_view);
  graphene_matrix_multiply (&right_eye_model_view, &translate_right,
      &right_eye_model_view);

  graphene_quaternion_to_matrix (&quat, &left_eye_model_view);
  graphene_matrix_multiply (&left_eye_model_view, &translate_left,
      &left_eye_model_view);

  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection,
      &self->right_vp_matrix);
}

void
gst_3d_camera_update_view_from_matrix (Gst3DCamera * self)
{
  gst_3d_hmd_update (self->hmd);

  graphene_matrix_t left_eye_model_view =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t left_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_t right_eye_model_view =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t right_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection,
      &self->right_vp_matrix);
}

void
gst_3d_camera_update_view (Gst3DCamera * self)
{
  gst_3d_camera_update_view_from_quaternion (self);
  // gst_3d_camera_update_view_from_matrix (self);
}


void
gst_3d_camera_update_view_mvp (Gst3DCamera * self)
{
  graphene_matrix_t projection_matrix;
  graphene_matrix_init_perspective (&projection_matrix,
      self->fov, self->aspect, self->znear, self->zfar);

  graphene_matrix_t view_matrix;
  graphene_matrix_init_look_at (&view_matrix, &self->eye, &self->center,
      &self->up);

  graphene_matrix_multiply (&view_matrix, &projection_matrix, &self->mvp);
}

void
gst_3d_camera_translate_arcball (Gst3DCamera * self, float z)
{
  self->center_distance += z * self->scroll_speed;
  GST_DEBUG ("center distance: %f", self->center_distance);
  gst_3d_camera_update_view_arcball (self);
}

void
gst_3d_camera_rotate_arcball (Gst3DCamera * self, float x, float y)
{
  self->theta += y * self->rotation_speed;
  self->phi += x * self->rotation_speed;
  GST_DEBUG ("theta: %f phi: %f", self->theta, self->phi);
  gst_3d_camera_update_view_arcball (self);
}

/*
static void
print_graphene_vec3 (const gchar * name, graphene_vec3_t * vec)
{
  GST_ERROR ("%s %f %f %f", name, graphene_vec3_get_x (vec),
      graphene_vec3_get_y (vec), graphene_vec3_get_z (vec));
}
*/

void
gst_3d_camera_update_view_arcball (Gst3DCamera * self)
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
