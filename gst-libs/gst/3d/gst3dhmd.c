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

#include "gst3dhmd.h"

#define GST_CAT_DEFAULT gst_3d_hmd_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DHmd, gst_3d_hmd, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_hmd_debug, "3dhmd", 0, "hmd"));

void
gst_3d_hmd_init (Gst3DHmd * self)
{
  self->context = NULL;
}

Gst3DHmd *
gst_3d_hmd_new (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DHmd *hmd = g_object_new (GST_3D_TYPE_HMD, NULL);
  hmd->context = gst_object_ref (context);
  return hmd;
}

static void
gst_3d_hmd_finalize (GObject * object)
{
  Gst3DHmd *self = GST_3D_HMD (object);
  g_return_if_fail (self != NULL);

  if (self->context) {
    gst_object_unref (self->context);
    self->context = NULL;
  }

  G_OBJECT_CLASS (gst_3d_hmd_parent_class)->finalize (object);
}

static void
gst_3d_hmd_class_init (Gst3DHmdClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_hmd_finalize;
}
