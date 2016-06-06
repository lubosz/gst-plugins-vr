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

/**
 * SECTION:element-vrcompositor
 *
 * Transform video for VR.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch-1.0 filesrc location=~/Videos/360.webm ! decodebin ! glupload ! glcolorconvert ! vrcompositor ! glimagesink
 * ]| Play spheric video.
 * </refsect2>
 */

#define GST_USE_UNSTABLE_API

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstvrcompositor.h"

#include <gst/gl/gstglapi.h>
#include <graphene-gobject.h>
#include "../../gst-libs/gst/3d/gst3drenderer.h"
#include "../../gst-libs/gst/3d/gst3dnode.h"
#include "../../gst-libs/gst/3d/gst3dscene.h"
#include "../../gst-libs/gst/3d/gst3dcamera_hmd.h"

#define GST_CAT_DEFAULT gst_vr_compositor_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define gst_vr_compositor_parent_class parent_class

enum
{
  PROP_0,
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_vr_compositor_debug, "vrcompositor", 0, "vrcompositor element");

G_DEFINE_TYPE_WITH_CODE (GstVRCompositor, gst_vr_compositor,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_vr_compositor_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_vr_compositor_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_vr_compositor_set_caps (GstGLFilter * filter,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_vr_compositor_src_event (GstBaseTransform * trans,
    GstEvent * event);

static void gst_vr_compositor_reset_gl (GstGLFilter * filter);
static gboolean gst_vr_compositor_stop (GstBaseTransform * trans);
static gboolean gst_vr_compositor_init_scene (GstGLFilter * filter);
static void gst_vr_compositor_draw (gpointer stuff);

static gboolean gst_vr_compositor_filter_texture (GstGLFilter * filter,
    guint in_tex, guint out_tex);

static void
gst_vr_compositor_class_init (GstVRCompositorClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstBaseTransformClass *base_transform_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);
  base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class->set_property = gst_vr_compositor_set_property;
  gobject_class->get_property = gst_vr_compositor_get_property;

  base_transform_class->src_event = gst_vr_compositor_src_event;

  GST_GL_FILTER_CLASS (klass)->init_fbo = gst_vr_compositor_init_scene;
  GST_GL_FILTER_CLASS (klass)->display_reset_cb = gst_vr_compositor_reset_gl;
  GST_GL_FILTER_CLASS (klass)->set_caps = gst_vr_compositor_set_caps;
  GST_GL_FILTER_CLASS (klass)->filter_texture =
      gst_vr_compositor_filter_texture;
  GST_BASE_TRANSFORM_CLASS (klass)->stop = gst_vr_compositor_stop;

  gst_element_class_set_metadata (element_class, "VR compositor",
      "Filter/Effect/Video", "Transform video for VR",
      "Lubosz Sarnecki <lubosz.sarnecki@collabora.co.uk>\n");

  GST_GL_BASE_FILTER_CLASS (klass)->supported_gl_api =
      GST_GL_API_OPENGL | GST_GL_API_OPENGL3 | GST_GL_API_GLES2;
}

static void
gst_vr_compositor_init (GstVRCompositor * self)
{
  self->shader = NULL;
  self->in_tex = 0;
  self->camera = NULL;

  self->left_color_tex = 0;
  self->left_fbo = 0;
  self->right_color_tex = 0;
  self->right_fbo = 0;

  self->eye_width = 1;
  self->eye_height = 1;

  self->default_fbo = 0;
  self->filter_aspect = 1.0f;
}

static void
gst_vr_compositor_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_vr_compositor_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static float
_aspect_from_filter (GstGLFilter * filter)
{
  int w = GST_VIDEO_INFO_WIDTH (&filter->out_info);
  int h = GST_VIDEO_INFO_HEIGHT (&filter->out_info);
  return (gdouble) w / (gdouble) h;
}

static gboolean
gst_vr_compositor_set_caps (GstGLFilter * filter, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (filter);

  if (!self->camera)
    self->camera = gst_3d_camera_hmd_new ();

  if (!(self->camera)->hmd->device)
    return FALSE;

  self->caps_change = TRUE;

  self->filter_aspect = gst_3d_camera_hmd_get_eye_aspect (self->camera);
  //self->filter_aspect = _aspect_from_filter (filter);
  //self->filter_aspect = gst_3d_camera_hmd_get_screen_aspect(self->camera);
  self->eye_width = gst_3d_camera_hmd_get_eye_width (self->camera);
  self->eye_height = gst_3d_camera_hmd_get_eye_height (self->camera);

  gst_3d_camera_hmd_update_view (self->camera);

  return TRUE;
}

static gboolean
gst_vr_compositor_src_event (GstBaseTransform * trans, GstEvent * event)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (trans);
  GST_DEBUG_OBJECT (trans, "handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NAVIGATION:
      event =
          GST_EVENT (gst_mini_object_make_writable (GST_MINI_OBJECT (event)));
      gst_3d_renderer_navigation_event (GST_ELEMENT (self), event);
      gst_3d_camera_hmd_navigation_event (self->camera, event);

      GstNavigationEventType event_type = gst_navigation_event_get_type (event);
      switch (event_type) {
        case GST_NAVIGATION_EVENT_KEY_PRESS:{
          GstStructure *structure =
              (GstStructure *) gst_event_get_structure (event);
          const gchar *key = gst_structure_get_string (structure, "key");
          if (key != NULL && g_strcmp0 (key, "space") == 0)
            gst_3d_hmd_reset ((self->camera)->hmd);
          break;
        }
        default:
          break;
      }

      break;
    default:
      break;
  }
  return GST_BASE_TRANSFORM_CLASS (parent_class)->src_event (trans, event);
}

static void
gst_vr_compositor_reset_gl (GstGLFilter * filter)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (filter);

  if (self->shader) {
    gst_object_unref (self->shader);
    self->shader = NULL;
  }
  // gst_object_unref (self->mesh);
}

static gboolean
gst_vr_compositor_stop (GstBaseTransform * trans)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (trans);
  // GstGLContext *context = GST_GL_BASE_FILTER (trans)->context;

  /* blocking call, wait until the opengl thread has destroyed the shader */
  gst_3d_shader_delete (self->shader);
  gst_object_unref (self->scene);

  return GST_BASE_TRANSFORM_CLASS (parent_class)->stop (trans);
}

static gboolean
gst_vr_compositor_init_scene (GstGLFilter * filter)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (filter);

  GstGLContext *context = GST_GL_BASE_FILTER (self)->context;
  GstGLFuncs *gl = context->gl_vtable;
  gboolean ret = TRUE;
  if (!self->shader) {

    self->scene = gst_3d_scene_new (context);

    self->shader = gst_3d_shader_new_vert_frag (context, "mvp_uv.vert",
        "texture_uv.frag");
    //Gst3DShader * sphere_shader = gst_3d_shader_new_vert_frag (context, "mvp_uv.vert",
    //    "debug_uv.frag");
    Gst3DShader *sphere_shader = self->shader;
    Gst3DMesh *sphere_mesh = gst_3d_mesh_new_sphere (context, 800.0, 100, 100);
    Gst3DNode *sphere_node =
        gst_3d_node_new_from_mesh_shader (context, sphere_mesh, sphere_shader);
    gst_3d_scene_append_node (self->scene, sphere_node);

    self->render_plane =
        gst_3d_mesh_new_plane (context, self->camera->hmd->left_aspect);
    gst_3d_mesh_bind_shader (self->render_plane, self->shader);

    gst_3d_renderer_create_fbo (gl, &self->left_fbo, &self->left_color_tex,
        self->eye_width, self->eye_height);
    gst_3d_renderer_create_fbo (gl, &self->right_fbo, &self->right_color_tex,
        self->eye_width, self->eye_height);
    gl->ClearColor (0.f, 0.f, 0.f, 0.f);
    gl->ActiveTexture (GL_TEXTURE0);

    gst_3d_shader_bind (self->shader);
    gst_gl_shader_set_uniform_1i (self->shader->shader, "texture", 0);
  }
  return ret;
}

static gboolean
gst_vr_compositor_filter_texture (GstGLFilter * filter, guint in_tex,
    guint out_tex)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (filter);

  self->in_tex = in_tex;

  /* blocking call, use a FBO */
  gst_gl_context_use_fbo_v2 (GST_GL_BASE_FILTER (filter)->context,
      GST_VIDEO_INFO_WIDTH (&filter->out_info),
      GST_VIDEO_INFO_HEIGHT (&filter->out_info),
      filter->fbo, filter->depthbuffer,
      out_tex, gst_vr_compositor_draw, (gpointer) self);

  return TRUE;
}

static void
_draw_eye (GstVRCompositor * self, GstGLFuncs * gl, GLuint fbo,
    graphene_matrix_t * mvp)
{
  gl->BindFramebuffer (GL_FRAMEBUFFER, fbo);
  gl->Viewport (0, 0, self->eye_width, self->eye_height);
  gst_3d_scene_draw (self->scene, mvp);
}

static void
_clear_state (GstGLContext * context, GstGLFuncs * gl)
{
  gl->BindVertexArray (0);
  gl->BindTexture (GL_TEXTURE_2D, 0);
  gst_gl_context_clear_shader (context);
}

static void
_draw_framebuffers_on_planes (GstVRCompositor * self, GstGLFuncs * gl)
{
  gl->BindFramebuffer (GL_FRAMEBUFFER, self->default_fbo);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  graphene_matrix_t projection_ortho;
  graphene_matrix_init_ortho (&projection_ortho, -self->filter_aspect,
      self->filter_aspect, -1.0, 1.0, -1.0, 1.0);
  gst_3d_shader_upload_matrix (self->shader, &projection_ortho, "mvp");
  gst_3d_mesh_bind (self->render_plane);

  // left framebuffer
  gl->Viewport (0, 0, self->eye_width, self->eye_height);
  glBindTexture (GL_TEXTURE_2D, self->left_color_tex);
  gst_3d_mesh_draw (self->render_plane);

  // right framebuffer
  gl->Viewport (self->eye_width, 0, self->eye_width, self->eye_height);
  glBindTexture (GL_TEXTURE_2D, self->right_color_tex);
  gst_3d_mesh_draw (self->render_plane);
}

static void
_draw_stereo (GstVRCompositor * self, GstGLFuncs * gl)
{
  gl->BindTexture (GL_TEXTURE_2D, self->in_tex);

  /* store current fbo id */
  if (self->default_fbo == 0)
    gl->GetIntegerv (GL_DRAW_FRAMEBUFFER_BINDING, &self->default_fbo);

  // LEFT EYE
  _draw_eye (self, gl, self->left_fbo, &(self->camera)->left_vp_matrix);

  // RIGHT EYE
  _draw_eye (self, gl, self->right_fbo, &(self->camera)->right_vp_matrix);

  gst_gl_shader_use (self->shader->shader);
  _draw_framebuffers_on_planes (self, gl);
}

static void
gst_vr_compositor_draw (gpointer this)
{
  GstVRCompositor *self = GST_VR_COMPOSITOR (this);
  GstGLContext *context = GST_GL_BASE_FILTER (this)->context;
  GstGLFuncs *gl = context->gl_vtable;

  gst_3d_camera_hmd_update_view (self->camera);
  _draw_stereo (self, gl);

  _clear_state (context, gl);

}
