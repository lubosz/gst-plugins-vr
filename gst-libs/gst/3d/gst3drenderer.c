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
#include "gst3dhmd.h"
#include "gst3dcamera_hmd.h"
#include "gst3dscene.h"


#define GST_CAT_DEFAULT gst_3d_renderer_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DRenderer, gst_3d_renderer, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_renderer_debug, "3drenderer", 0,
        "renderer"));

void
_insert_gl_debug_marker (GstGLContext * context, const gchar * message)
{
  GstGLFuncs *gl = context->gl_vtable;
  gl->DebugMessageInsert (GL_DEBUG_SOURCE_APPLICATION,
      GL_DEBUG_TYPE_OTHER,
      1, GL_DEBUG_SEVERITY_HIGH, strlen (message), message);
}

void
gst_3d_renderer_init (Gst3DRenderer * self)
{
  self->context = NULL;
  self->shader = NULL;
  self->left_color_tex = 0;
  self->left_fbo = 0;
  self->right_color_tex = 0;
  self->right_fbo = 0;
  self->eye_width = 1;
  self->eye_height = 1;
  self->filter_aspect = 1.0f;
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

  if (self->shader)
    gst_3d_shader_delete (self->shader);

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

static void
_create_fbo (GstGLFuncs * gl, GLuint * fbo, GLuint * color_tex,
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
}

/* stereo rendering */

gboolean
gst_3d_renderer_stereo_init_from_hmd (Gst3DRenderer * self, Gst3DHmd * hmd)
{
  if (!hmd->device)
    return FALSE;

  self->filter_aspect = gst_3d_hmd_get_eye_aspect (hmd);
  //self->filter_aspect = gst_3d_hmd_get_screen_aspect(hmd);
  self->eye_width = gst_3d_hmd_get_eye_width (hmd);
  self->eye_height = gst_3d_hmd_get_eye_height (hmd);

  return TRUE;
}

gboolean
gst_3d_renderer_stero_init_from_filter (Gst3DRenderer * self,
    GstGLFilter * filter)
{
  int w = GST_VIDEO_INFO_WIDTH (&filter->out_info) / 2;
  int h = GST_VIDEO_INFO_HEIGHT (&filter->out_info);

  self->filter_aspect = (gfloat) w / (gfloat) h;
  self->eye_width = w;
  self->eye_height = h;

  return TRUE;
}

static void
_draw_eye (Gst3DRenderer * self, GLuint fbo, Gst3DScene * scene,
    graphene_matrix_t * mvp)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  _insert_gl_debug_marker (self->context, "_draw_eye");
  gl->BindFramebuffer (GL_FRAMEBUFFER, fbo);
  gl->Viewport (0, 0, self->eye_width, self->eye_height);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gst_3d_scene_draw_nodes (scene, mvp);
}

static void
_draw_framebuffers_on_planes (Gst3DRenderer * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  _insert_gl_debug_marker (self->context, "_draw_framebuffers_on_planes");

  graphene_matrix_t projection_ortho;
  graphene_matrix_init_ortho (&projection_ortho, -self->filter_aspect,
      self->filter_aspect, -1.0, 1.0, -1.0, 1.0);
  gst_3d_shader_upload_matrix (self->shader, &projection_ortho, "mvp");
  gst_3d_mesh_bind (self->render_plane);

  /* left framebuffer */
  gl->Viewport (0, 0, self->eye_width, self->eye_height);
  glBindTexture (GL_TEXTURE_2D, self->left_color_tex);
  gst_3d_mesh_draw (self->render_plane);

  /* right framebuffer */
  gl->Viewport (self->eye_width, 0, self->eye_width, self->eye_height);
  glBindTexture (GL_TEXTURE_2D, self->right_color_tex);
  gst_3d_mesh_draw (self->render_plane);
}

void
gst_3d_renderer_init_stereo (Gst3DRenderer * self, Gst3DCamera * cam)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  Gst3DCameraHmd *hmd_cam = GST_3D_CAMERA_HMD (cam);
  Gst3DHmd *hmd = hmd_cam->hmd;
  float aspect_ratio = hmd->left_aspect;
  self->render_plane = gst_3d_mesh_new_plane (self->context, aspect_ratio);
  self->shader = gst_3d_shader_new_vert_frag (self->context, "mvp_uv.vert",
      "texture_uv.frag");
  gst_3d_mesh_bind_shader (self->render_plane, self->shader);

  _create_fbo (gl, &self->left_fbo, &self->left_color_tex,
      self->eye_width, self->eye_height);
  _create_fbo (gl, &self->right_fbo, &self->right_color_tex,
      self->eye_width, self->eye_height);

  gst_3d_shader_bind (self->shader);
  gst_gl_shader_set_uniform_1i (self->shader->shader, "texture", 0);
}

void
gst_3d_renderer_draw_stereo (Gst3DRenderer * self, Gst3DScene * scene)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  _insert_gl_debug_marker (self->context, "gst_3d_renderer_draw_stereo");

  /* aquire current fbo id */
  GLint bound_fbo;
  gl->GetIntegerv (GL_DRAW_FRAMEBUFFER_BINDING, &bound_fbo);
  if (bound_fbo == 0)
    return;

  Gst3DCameraHmd *hmd_cam = GST_3D_CAMERA_HMD (scene->camera);

  /* left eye */
  _draw_eye (self, self->left_fbo, scene, &hmd_cam->left_vp_matrix);

  /* right eye */
  _draw_eye (self, self->right_fbo, scene, &hmd_cam->right_vp_matrix);

  gst_3d_renderer_clear_state (self);

  gst_3d_shader_bind (self->shader);
  gl->BindFramebuffer (GL_FRAMEBUFFER, bound_fbo);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _draw_framebuffers_on_planes (self);
  gst_3d_renderer_clear_state (self);
}
