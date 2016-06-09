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

#define GST_CAT_DEFAULT gst_3d_camera_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCamera, gst_3d_camera, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_camera_debug, "3dcamera", 0, "camera"));


void
gst_3d_camera_init (Gst3DCamera * self)
{
  self->fov = 45.0;
  //self->aspect = 4.0 / 3.0;
  self->aspect = 16.0 / 9.0;
  self->znear = 0.01;
  self->zfar = 1000.0;

  graphene_vec3_init (&self->eye, 0.f, 0.f, 1.f);
  graphene_vec3_init (&self->center, 0.f, 0.f, 0.f);
  graphene_vec3_init (&self->up, 0.f, 1.f, 0.f);

  self->cursor_last_x = 0;
  self->cursor_last_y = 0;
  self->pressed_mouse_button = 0;
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
gst_3d_camera_update_view (Gst3DCamera * self)
{
  g_return_if_fail (self);
  Gst3DCameraClass *camera_class = GST_3D_CAMERA_GET_CLASS (self);
  if (camera_class->update_view)
    camera_class->update_view (self);
}

void
gst_3d_camera_navigation_event (Gst3DCamera * self, GstEvent * event)
{
  Gst3DCameraClass *camera_class = GST_3D_CAMERA_GET_CLASS (self);
  if (camera_class->navigation_event)
    camera_class->navigation_event (self, event);
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
gst_3d_camera_press_key (Gst3DCamera * self, const gchar * key)
{
  GList *l;
  gboolean already_pushed = FALSE;

  GST_DEBUG ("Event: Press %s", key);

  for (l = self->pushed_buttons; l != NULL; l = l->next)
    if (g_strcmp0 (l->data, key) == 0)
      already_pushed = TRUE;

  if (!already_pushed)
    self->pushed_buttons = g_list_append (self->pushed_buttons, g_strdup (key));
}

void
gst_3d_camera_release_key (Gst3DCamera * self, const gchar * key)
{
  GST_DEBUG ("Event: Release %s", key);

  GList *l = self->pushed_buttons;
  while (l != NULL) {
    GList *next = l->next;
    if (g_strcmp0 (l->data, key) == 0) {
      g_free (l->data);
      self->pushed_buttons = g_list_delete_link (self->pushed_buttons, l);
    }
    l = next;
  }
}

void
gst_3d_camera_print_pressed_keys (Gst3DCamera * self)
{
  GList *l;
  GST_DEBUG ("Pressed keys:");
  for (l = self->pushed_buttons; l != NULL; l = l->next)
    GST_DEBUG ("%s", (const gchar *) l->data);
}
