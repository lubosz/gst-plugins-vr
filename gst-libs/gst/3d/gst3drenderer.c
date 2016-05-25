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

#include "gst3drenderer.h"

#define GST_CAT_DEFAULT gst_3d_renderer_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DRenderer, gst_3d_renderer, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_renderer_debug, "3drenderer", 0,
        "renderer"));

void
gst_3d_renderer_init (Gst3DRenderer * self)
{
  self->context = NULL;
}

Gst3DRenderer *
gst_3d_renderer_new (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DRenderer *renderer = g_object_new (GST_3D_TYPE_RENDERER, NULL);
  renderer->context = gst_object_ref (context);
  return renderer;
}

static void
gst_3d_renderer_finalize (GObject * object)
{
  Gst3DRenderer *self = GST_3D_RENDERER (object);
  g_return_if_fail (self != NULL);

  if (self->context) {
    gst_object_unref (self->context);
    self->context = NULL;
  }

  G_OBJECT_CLASS (gst_3d_renderer_parent_class)->finalize (object);
}

static void
gst_3d_renderer_class_init (Gst3DRendererClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_renderer_finalize;
}

void
gst_3d_renderer_send_eos (GstElement * element)
{
  GstPad *sinkpad = gst_element_get_static_pad (element, "sink");
  gst_pad_send_event (sinkpad, gst_event_new_eos ());
}

void
gst_3d_renderer_create_fbo (GstGLFuncs * gl, GLuint * fbo, GLuint * color_tex,
    int width, int height)
{
  gl->GenTextures (1, color_tex);
  gl->GenFramebuffers (1, fbo);

  gl->BindTexture (GL_TEXTURE_2D, *color_tex);
  gl->TexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
      0, GL_RGBA, GL_UNSIGNED_INT, NULL);
  gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  gl->BindFramebuffer (GL_FRAMEBUFFER_EXT, *fbo);
  gl->FramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      *color_tex, 0);

  GLenum status = gl->CheckFramebufferStatus (GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    GST_ERROR ("failed to create fbo %x\n", status);
  }
  gl->BindFramebuffer (GL_FRAMEBUFFER, 0);
}

void
gst_3d_renderer_navigation_event (GstElement * element, GstEvent * event)
{
  GstStructure *structure = (GstStructure *) gst_event_get_structure (event);
  const gchar *event_name = gst_structure_get_string (structure, "event");
  if (g_strcmp0 (event_name, "key-press") == 0) {
    const gchar *key = gst_structure_get_string (structure, "key");
    if (key != NULL)
      if (g_strcmp0 (key, "Escape") == 0)
        gst_3d_renderer_send_eos (element);
  }
}
