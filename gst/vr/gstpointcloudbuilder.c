/*
 * GStreamer Plugins VR
 * Copyright (C) 2016 Lubosz Sarnecki <lubosz@collabora.co.uk>
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
 * SECTION:element-pointcloudbuilder
 *
 * Construct a planar point cloud from a depth buffer.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch-1.0 freenect2src sourcetype=0 ! glupload ! glcolorconvert ! pointcloudbuilder ! glimagesink
 * ]| Display point cloud from Kinect v2.
 * </refsect2>
 */

#define GST_USE_UNSTABLE_API

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstpointcloudbuilder.h"

#include <gst/gl/gstglapi.h>
#include <graphene-gobject.h>
#include <glib.h>
#include <glib/gprintf.h>

#define GST_CAT_DEFAULT gst_point_cloud_builder_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define gst_point_cloud_builder_parent_class parent_class

enum
{
  PROP_0,
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_point_cloud_builder_debug, "pointcloudbuilder", 0, "pointcloudbuilder element");

G_DEFINE_TYPE_WITH_CODE (GstPointCloudBuilder, gst_point_cloud_builder,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_point_cloud_builder_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_point_cloud_builder_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_point_cloud_builder_set_caps (GstGLFilter * filter,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_point_cloud_builder_src_event (GstBaseTransform * trans,
    GstEvent * event);

static void gst_point_cloud_builder_reset_gl (GstGLFilter * filter);
static gboolean gst_point_cloud_builder_stop (GstBaseTransform * trans);
static gboolean gst_point_cloud_builder_init_shader (GstGLFilter * filter);
static void gst_point_cloud_builder_callback (gpointer stuff);

static gboolean gst_point_cloud_builder_filter_texture (GstGLFilter * filter,
    guint in_tex, guint out_tex);

static void
gst_point_cloud_builder_class_init (GstPointCloudBuilderClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstBaseTransformClass *base_transform_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);
  base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class->set_property = gst_point_cloud_builder_set_property;
  gobject_class->get_property = gst_point_cloud_builder_get_property;

  base_transform_class->src_event = gst_point_cloud_builder_src_event;

  GST_GL_FILTER_CLASS (klass)->init_fbo = gst_point_cloud_builder_init_shader;
  GST_GL_FILTER_CLASS (klass)->display_reset_cb =
      gst_point_cloud_builder_reset_gl;
  GST_GL_FILTER_CLASS (klass)->set_caps = gst_point_cloud_builder_set_caps;
  GST_GL_FILTER_CLASS (klass)->filter_texture =
      gst_point_cloud_builder_filter_texture;
  GST_BASE_TRANSFORM_CLASS (klass)->stop = gst_point_cloud_builder_stop;

  gst_element_class_set_metadata (element_class, "Point cloud builder",
      "Filter/Effect/Video", "Turn depth buffers into a point cloud",
      "Lubosz Sarnecki <lubosz.sarnecki@colloabora.co.uk>\n");

  GST_GL_BASE_FILTER_CLASS (klass)->supported_gl_api =
      GST_GL_API_OPENGL | GST_GL_API_OPENGL3 | GST_GL_API_GLES2;
}

static void
gst_point_cloud_builder_init (GstPointCloudBuilder * self)
{
  self->shader = NULL;
  self->render_mode = GL_TRIANGLE_STRIP;
  self->in_tex = 0;
  self->mesh = NULL;
  self->camera = gst_3d_camera_new ();

  self->left_color_tex = 0;
  self->left_fbo = 0;
  self->right_color_tex = 0;
  self->right_fbo = 0;

  self->eye_width = 1;
  self->eye_height = 1;

  self->default_fbo = 0;
  
  self->pressed_mouse_button = 0;
}

static void
gst_point_cloud_builder_send_eos (GstElement * self) {
  GstPad * sinkpad = gst_element_get_static_pad (self, "sink");
  gst_pad_send_event (sinkpad, gst_event_new_eos ());
}

static void
gst_point_cloud_builder_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_point_cloud_builder_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_point_cloud_builder_set_caps (GstGLFilter * filter, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (filter);
  
  self->camera->aspect =
      (gdouble) GST_VIDEO_INFO_WIDTH (&filter->out_info) /
      (gdouble) GST_VIDEO_INFO_HEIGHT (&filter->out_info);

  self->eye_width = GST_VIDEO_INFO_WIDTH (&filter->out_info);
  self->eye_height = GST_VIDEO_INFO_HEIGHT (&filter->out_info);

  GST_ERROR ("eye %dx%d", self->eye_width, self->eye_height);

  self->caps_change = TRUE;

  return TRUE;
}


static gboolean
gst_point_cloud_builder_src_event (GstBaseTransform * trans, GstEvent * event)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (trans);
  GstStructure *structure;

  GST_DEBUG_OBJECT (trans, "handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NAVIGATION:
      event =
          GST_EVENT (gst_mini_object_make_writable (GST_MINI_OBJECT (event)));

      structure = (GstStructure *) gst_event_get_structure (event);

      const gchar *event_name = gst_structure_get_string (structure, "event");

      if (g_strcmp0 (event_name, "key-press") == 0) {
        const gchar *key = gst_structure_get_string (structure, "key");
        if (key != NULL) {
          if (g_strcmp0 (key, "Escape") == 0) {
            gst_point_cloud_builder_send_eos(GST_ELEMENT(self));
          } else if (g_strcmp0 (key, "KP_Add") == 0) {
            gst_3d_camera_inc_eye_sep (self->camera);
          } else if (g_strcmp0 (key, "KP_Subtract") == 0) {
            gst_3d_camera_dec_eye_sep (self->camera);
          } else {
            GST_DEBUG ("%s", key);
          }
        }
      } else if (g_strcmp0 (event_name, "mouse-move") == 0) {
        gdouble x, y;
        gst_structure_get_double (structure, "pointer_x", &x);
        gst_structure_get_double (structure, "pointer_y", &y);
        
        // hanlde the mouse motion for zooming and rotating the view
        gdouble dx, dy;
        dx = x - self->camera->cursor_last_x;
        dy = y - self->camera->cursor_last_y;

        if (self->pressed_mouse_button == 1) {
            // GST_DEBUG ("Rotating [%fx%f]", x, y);
            gst_3d_camera_rotate_arcball(self->camera, dx, dy);
        }
        self->camera->cursor_last_x = x;
        self->camera->cursor_last_y = y;
      } else if (g_strcmp0 (event_name, "mouse-button-release") == 0) {
        gint button;
        gst_structure_get_int (structure, "button", &button);
        self->pressed_mouse_button = 0;
        
        if (button == 1) {
          // GST_DEBUG("first button release");
          gst_structure_get_double (structure, "pointer_x", &self->camera->cursor_last_x);
          gst_structure_get_double (structure, "pointer_y", &self->camera->cursor_last_y);
        } else if (button == 4) {
          // GST_DEBUG("wheel up");
          gst_3d_camera_translate_arcball (self->camera, -1.0);
        } else if (button == 5) {
          // GST_DEBUG("wheel down");
          gst_3d_camera_translate_arcball (self->camera, 1.0);
        }
        // GST_DEBUG ("release %d", button);
      } else if (g_strcmp0 (event_name, "mouse-button-press") == 0) {
        gint button;
        gst_structure_get_int (structure, "button", &button);
        // GST_DEBUG ("press %d", button);
        self->pressed_mouse_button = button;
      // } else if (g_strcmp0 (event_name, "key-release") == 0) {
      } else {
        GST_ERROR ("unknown event %s", event_name);
      }
      break;
    default:
      break;
  }
  return GST_BASE_TRANSFORM_CLASS (parent_class)->src_event (trans, event);
}

static void
gst_point_cloud_builder_reset_gl (GstGLFilter * filter)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (filter);

  if (self->shader) {
    gst_object_unref (self->shader);
    self->shader = NULL;
  }
  gst_object_unref (self->mesh);
}

static gboolean
gst_point_cloud_builder_stop (GstBaseTransform * trans)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (trans);
  // GstGLContext *context = GST_GL_BASE_FILTER (trans)->context;

  /* blocking call, wait until the opengl thread has destroyed the shader */
  gst_3d_shader_delete (self->shader);

  return GST_BASE_TRANSFORM_CLASS (parent_class)->stop (trans);
}

void
_create_fbo2 (GstPointCloudBuilder * self, GLuint * fbo, GLuint * color_tex)
{
  GstGLContext *context = GST_GL_BASE_FILTER (self)->context;
  GstGLFuncs *gl = context->gl_vtable;

  gl->GenTextures (1, color_tex);
  gl->GenFramebuffers (1, fbo);

  gl->BindTexture (GL_TEXTURE_2D, *color_tex);
  gl->TexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, self->eye_width, self->eye_height,
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

static gboolean
_init_gl (GstPointCloudBuilder * self)
{
  GstGLContext *context = GST_GL_BASE_FILTER (self)->context;
  GstGLFuncs *gl = context->gl_vtable;
  gboolean ret = TRUE;
  if (!self->mesh) {
    self->shader = gst_3d_shader_new (context);
    ret =
        gst_3d_shader_from_vert_frag (self->shader, "points.vert",
        "points.frag");
    gst_3d_shader_bind (self->shader);
    self->render_plane = gst_3d_mesh_new (context);
    gst_3d_mesh_init_buffers (self->render_plane);
    gst_3d_shader_enable_attribs (self->shader);
    gst_3d_mesh_upload_point_plane (self->render_plane, 512, 424);
    
    gst_3d_mesh_bind_buffers (self->render_plane, self->shader->attr_position,
        self->shader->attr_uv);
    gl->ClearColor (0.f, 0.f, 0.f, 0.f);
    gl->ActiveTexture (GL_TEXTURE0);
    gst_gl_shader_set_uniform_1i (self->shader->shader, "texture", 0);
  }
  return ret;
}

static gboolean
gst_point_cloud_builder_init_shader (GstGLFilter * filter)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (filter);
  return _init_gl (self);
}

static gboolean
gst_point_cloud_builder_filter_texture (GstGLFilter * filter, guint in_tex,
    guint out_tex)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (filter);

  self->in_tex = in_tex;

  /* blocking call, use a FBO */
  gst_gl_context_use_fbo_v2 (GST_GL_BASE_FILTER (filter)->context,
      GST_VIDEO_INFO_WIDTH (&filter->out_info),
      GST_VIDEO_INFO_HEIGHT (&filter->out_info),
      filter->fbo, filter->depthbuffer,
      out_tex, gst_point_cloud_builder_callback, (gpointer) self);

  return TRUE;
}

static void
gst_point_cloud_builder_callback (gpointer this)
{
  GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (this);
  GstGLContext *context = GST_GL_BASE_FILTER (this)->context;
  GstGLFuncs *gl = context->gl_vtable;

  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gst_gl_shader_use (self->shader->shader);
  gl->BindTexture (GL_TEXTURE_2D, self->in_tex);
  
  gst_3d_camera_update_view_arcball (self->camera);

  gst_3d_shader_upload_matrix (self->shader, &self->camera->mvp, "mvp");

  gst_3d_mesh_bind (self->render_plane);
  gst_3d_mesh_draw_arrays (self->render_plane);

  gl->BindVertexArray (0);
  gl->BindTexture (GL_TEXTURE_2D, 0);
  gst_gl_context_clear_shader (context);
}
