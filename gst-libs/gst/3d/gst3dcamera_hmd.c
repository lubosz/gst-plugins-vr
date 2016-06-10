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

#include "gst3dcamera_hmd.h"
#include "gst3dglm.h"
#include "gst3dmath.h"
#include "gst3drenderer.h"
#include "gst3dmath.h"


#define GST_CAT_DEFAULT gst_3d_camera_hmd_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCameraHmd, gst_3d_camera_hmd,
    GST_3D_TYPE_CAMERA, GST_DEBUG_CATEGORY_INIT (gst_3d_camera_hmd_debug,
        "3dcamera_hmd", 0, "camera_hmd"));

static void gst_3d_camera_hmd_navigation_event (Gst3DCamera * self,
    GstEvent * event);
static void gst_3d_camera_hmd_update_view (Gst3DCamera * self);

void
gst_3d_camera_hmd_init (Gst3DCameraHmd * self)
{
  self->hmd = gst_3d_hmd_new ();
  self->query_type = HMD_QUERY_TYPE_MATRIX_STEREO;
  self->update_view_funct = &gst_3d_camera_hmd_update_view_from_matrix;
}

Gst3DCameraHmd *
gst_3d_camera_hmd_new (void)
{
  Gst3DCameraHmd *camera_hmd = g_object_new (GST_3D_TYPE_CAMERA_HMD, NULL);
  return camera_hmd;
}

static void
gst_3d_camera_hmd_finalize (GObject * object)
{
  Gst3DCameraHmd *self = GST_3D_CAMERA_HMD (object);
  g_return_if_fail (self != NULL);

  gst_object_unref (self->hmd);

  G_OBJECT_CLASS (gst_3d_camera_hmd_parent_class)->finalize (object);
}

static void
gst_3d_camera_hmd_class_init (Gst3DCameraHmdClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = gst_3d_camera_hmd_finalize;
  Gst3DCameraClass *camera_class = GST_3D_CAMERA_CLASS (klass);
  camera_class->update_view = gst_3d_camera_hmd_update_view;
  camera_class->navigation_event = gst_3d_camera_hmd_navigation_event;
}

void
gst_3d_camera_hmd_update_view (Gst3DCamera * cam)
{
  Gst3DCameraHmd *self = GST_3D_CAMERA_HMD (cam);
  gst_3d_hmd_update (self->hmd);
  self->update_view_funct (self);
}

void
gst_3d_camera_hmd_update_view_from_quaternion (Gst3DCameraHmd * self)
{
  /* projection from OpenHMD */
  graphene_matrix_t left_eye_model_view;
  graphene_matrix_t left_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_t right_eye_model_view;
  graphene_matrix_t right_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  /* rotation from OpenHMD */
  graphene_quaternion_t quat = gst_3d_hmd_get_quaternion (self->hmd);
  graphene_quaternion_to_matrix (&quat, &right_eye_model_view);
  graphene_quaternion_to_matrix (&quat, &left_eye_model_view);

  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection,
      &self->right_vp_matrix);
}

void
gst_3d_camera_hmd_update_view_from_quaternion_stereo (Gst3DCameraHmd * self)
{
  /* projection from OpenHMD */
  graphene_matrix_t left_eye_model_view;
  graphene_matrix_t left_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_t right_eye_model_view;
  graphene_matrix_t right_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  /* rotation from OpenHMD */
  graphene_quaternion_t quat = gst_3d_hmd_get_quaternion (self->hmd);
  graphene_quaternion_to_matrix (&quat, &right_eye_model_view);
  graphene_quaternion_to_matrix (&quat, &left_eye_model_view);

  /* eye separation */
  graphene_point3d_t left_eye;
  graphene_point3d_init (&left_eye, +self->hmd->eye_separation, 0, 0);
  graphene_matrix_t translate_left;
  graphene_matrix_init_translate (&translate_left, &left_eye);
  graphene_point3d_t rigth_eye;
  graphene_point3d_init (&rigth_eye, -self->hmd->eye_separation, 0, 0);
  graphene_matrix_t translate_right;
  graphene_matrix_init_translate (&translate_right, &rigth_eye);

  graphene_matrix_multiply (&right_eye_model_view, &translate_right,
      &right_eye_model_view);
  graphene_matrix_multiply (&left_eye_model_view, &translate_left,
      &left_eye_model_view);

  graphene_matrix_multiply (&left_eye_model_view, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&right_eye_model_view, &right_eye_projection,
      &self->right_vp_matrix);
}

void
_matrix_invert_y_rotation (const graphene_matrix_t * source,
    graphene_matrix_t * result)
{
  gfloat invert_values[] = {
    1, -1, 1, 1,
    -1, 1, -1, 1,
    1, -1, 1, 1,
    1, 1, 1, 1
  };
  graphene_matrix_t invert_matrix;
  graphene_matrix_init_from_float (&invert_matrix, invert_values);
  gst_3d_math_matrix_hadamard_product (&invert_matrix, source, result);
}

void
gst_3d_camera_hmd_update_view_from_matrix (Gst3DCameraHmd * self)
{
  graphene_matrix_t left_eye_model_view =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t left_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_t right_eye_model_view =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX);
  graphene_matrix_t right_eye_projection =
      gst_3d_hmd_get_matrix (self->hmd, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX);

  graphene_matrix_t left_eye_model_view_inv;
  graphene_matrix_t right_eye_model_view_inv;

  _matrix_invert_y_rotation (&left_eye_model_view, &left_eye_model_view_inv);
  _matrix_invert_y_rotation (&right_eye_model_view, &right_eye_model_view_inv);

  graphene_matrix_multiply (&right_eye_model_view_inv, &left_eye_projection,
      &self->left_vp_matrix);
  graphene_matrix_multiply (&left_eye_model_view_inv, &right_eye_projection,
      &self->right_vp_matrix);

}

static void
_iterate_query_type (Gst3DCameraHmd * self)
{
  self->query_type++;

  if (self->query_type == HMD_QUERY_TYPE_NONE)
    self->query_type = HMD_QUERY_TYPE_QUATERNION_MONO;
  GST_DEBUG ("query type: %d", self->query_type);

  switch (self->query_type) {
    case HMD_QUERY_TYPE_MATRIX_STEREO:
    case HMD_QUERY_TYPE_NONE:
      self->update_view_funct = &gst_3d_camera_hmd_update_view_from_matrix;
      break;
    case HMD_QUERY_TYPE_QUATERNION_MONO:
      self->update_view_funct = &gst_3d_camera_hmd_update_view_from_quaternion;
      break;
    case HMD_QUERY_TYPE_QUATERNION_STEREO:
      self->update_view_funct =
          &gst_3d_camera_hmd_update_view_from_quaternion_stereo;
      break;
  }
}

static void
gst_3d_camera_hmd_navigation_event (Gst3DCamera * cam, GstEvent * event)
{
  GstNavigationEventType event_type = gst_navigation_event_get_type (event);
  switch (event_type) {
    case GST_NAVIGATION_EVENT_KEY_PRESS:{
      Gst3DCameraHmd *self = GST_3D_CAMERA_HMD (cam);
      GstStructure *structure =
          (GstStructure *) gst_event_get_structure (event);
      const gchar *key = gst_structure_get_string (structure, "key");
      if (key != NULL) {
        if (g_strcmp0 (key, "KP_Add") == 0)
          gst_3d_hmd_eye_sep_inc (self->hmd);
        else if (g_strcmp0 (key, "KP_Subtract") == 0)
          gst_3d_hmd_eye_sep_dec (self->hmd);
        else if (g_strcmp0 (key, "Return") == 0)
          _iterate_query_type (self);
        else if (g_strcmp0 (key, "space") == 0)
          gst_3d_hmd_reset (GST_3D_CAMERA_HMD (self)->hmd);
      }
    }
    default:
      break;
  }
}
