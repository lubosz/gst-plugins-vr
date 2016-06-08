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

#include "gst3dcamera_wasd.h"
#include "gst3dglm.h"
#include "gst3drenderer.h"

#define GST_CAT_DEFAULT gst_3d_camera_wasd_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DCameraWasd, gst_3d_camera_wasd,
    GST_3D_TYPE_CAMERA, GST_DEBUG_CATEGORY_INIT (gst_3d_camera_wasd_debug,
        "3dcamera_wasd", 0, "camera_wasd"));

void gst_3d_camera_wasd_update_view (Gst3DCamera * cam);
void gst_3d_camera_wasd_navigation_event (Gst3DCamera * cam, GstEvent * event);

void
gst_3d_camera_wasd_init (Gst3DCameraWasd * self)
{
  self->xtranslation = 0;
  self->ytranslation = 0;
  self->ztranslation = 0;
}

Gst3DCameraWasd *
gst_3d_camera_wasd_new (void)
{
  Gst3DCameraWasd *camera_wasd = g_object_new (GST_3D_TYPE_CAMERA_WASD, NULL);
  return camera_wasd;
}

static void
gst_3d_camera_wasd_finalize (GObject * object)
{
  Gst3DCameraWasd *self = GST_3D_CAMERA_WASD (object);
  g_return_if_fail (self != NULL);

  G_OBJECT_CLASS (gst_3d_camera_wasd_parent_class)->finalize (object);
}

static void
gst_3d_camera_wasd_class_init (Gst3DCameraWasdClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = gst_3d_camera_wasd_finalize;
  Gst3DCameraClass *camera_class = GST_3D_CAMERA_CLASS (klass);
  camera_class->update_view = gst_3d_camera_wasd_update_view;
  camera_class->navigation_event = gst_3d_camera_wasd_navigation_event;
}

void
gst_3d_camera_wasd_process_input (Gst3DCameraWasd * self)
{
  //_print_pressed_keys (self);

  gfloat fast_modifier = 1.0;
  GList *l;
  for (l = GST_3D_CAMERA (self)->pushed_buttons; l != NULL; l = l->next)
    if (g_strcmp0 (l->data, "Shift_L") == 0)
      fast_modifier = 3.0;


  gfloat distance = 0.01 * fast_modifier;

  for (l = GST_3D_CAMERA (self)->pushed_buttons; l != NULL; l = l->next) {
    if (g_strcmp0 (l->data, "w") == 0) {
      self->ztranslation += distance;
      continue;
    } else if (g_strcmp0 (l->data, "s") == 0) {
      self->ztranslation -= distance;
      continue;
    }

    if (g_strcmp0 (l->data, "a") == 0) {
      self->xtranslation += distance;
      continue;
    } else if (g_strcmp0 (l->data, "d") == 0) {
      self->xtranslation -= distance;
      continue;
    }

    if (g_strcmp0 (l->data, "space") == 0) {
      self->ytranslation += distance;
      continue;
    } else if (g_strcmp0 (l->data, "Control_L") == 0) {
      self->ytranslation -= distance;
      continue;
    }
  }
}


void
gst_3d_camera_wasd_update_view (Gst3DCamera * cam)
{
}

void
gst_3d_camera_wasd_navigation_event (Gst3DCamera * cam, GstEvent * event)
{

}
