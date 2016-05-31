/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2016> Matthew Waters <matthew@centricular.com>
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

#include "vrtestsrc.h"

#include "../../gst-libs/gst/3d/gst3dshader.h"
#include "../../gst-libs/gst/3d/gst3dmesh.h"
#include "../../gst-libs/gst/3d/gst3dcamera_arcball.h"

// #define MAX_ATTRIBUTES 4

struct SrcShader
{
  struct BaseSrcImpl base;

  Gst3DShader *shader;

  GList *axesDebugMeshes;

  guint vao;
  guint vbo;
  guint vbo_indices;

  Gst3DCameraArcball *camera;

  guint n_attributes;
};

/*
static gboolean
_src_mandelbrot_src_event (gpointer impl, GstEvent * event)
{
  // GstPointCloudBuilder *self = GST_POINT_CLOUD_BUILDER (trans);

  struct SrcShader *src = impl;

  GST_DEBUG ("handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NAVIGATION:
      //event =
      //    GST_EVENT (gst_mini_object_make_writable (GST_MINI_OBJECT (event)));
      //gst_3d_renderer_navigation_event (GST_ELEMENT (src), event);
      //gst_3d_camera_arcball_navigation_event (src->camera, event);
      break;
    default:
      break;
  }

  // gst_event_unref (event);
  // return GST_BASE_TRANSFORM_CLASS (parent_class)->src_event (trans, event);
  return TRUE;
}
*/

static void
_create_debug_axes (struct SrcShader *src)
{
  graphene_vec3_t from, to, color;
  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 1.f, 0.f, 0.f);
  graphene_vec3_init (&color, 1.f, 0.f, 0.f);
  Gst3DMesh *x_axis =
      gst_3d_mesh_new_line (src->base.context, &from, &to, &color);
  gst_3d_mesh_bind_shader (x_axis, src->shader);
  src->axesDebugMeshes = g_list_append (src->axesDebugMeshes, x_axis);

  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 0.f, 1.f, 0.f);
  graphene_vec3_init (&color, 0.f, 1.f, 0.f);
  Gst3DMesh *y_axis =
      gst_3d_mesh_new_line (src->base.context, &from, &to, &color);
  gst_3d_mesh_bind_shader (y_axis, src->shader);
  src->axesDebugMeshes = g_list_append (src->axesDebugMeshes, y_axis);

  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 0.f, 0.f, 1.f);
  graphene_vec3_init (&color, 0.f, 0.f, 1.f);
  Gst3DMesh *z_axis =
      gst_3d_mesh_new_line (src->base.context, &from, &to, &color);
  gst_3d_mesh_bind_shader (z_axis, src->shader);
  src->axesDebugMeshes = g_list_append (src->axesDebugMeshes, z_axis);
}

static gboolean
_src_mandelbrot_init (gpointer impl, GstGLContext * context,
    GstVideoInfo * v_info)
{
  struct SrcShader *src = impl;
  GError *error = NULL;

  src->base.context = context;

  GstGLFuncs *gl = context->gl_vtable;

  src->camera = gst_3d_camera_arcball_new ();

  if (src->shader)
    gst_object_unref (src->shader);


  src->shader =
      gst_3d_shader_new_vert_frag (context, "mvp_color.vert", "color.frag");

  if (!src->shader) {
    GST_ERROR_OBJECT (src->base.src, "%s", error->message);
    return FALSE;
  }
  //src->plane_mesh = gst_3d_mesh_new_plane (context, 1.0);
  //src->plane_mesh = gst_3d_mesh_new_sphere (context, 2.0, 20, 20);
  // src->plane_mesh->draw_mode = GL_LINES;

  _create_debug_axes (src);


  gl->Enable (GL_CULL_FACE);

/*
  gst_gl_shader_use (src->shader);
  gst_gl_shader_set_uniform_1f (src->shader, "aspect_ratio",
      (gfloat) GST_VIDEO_INFO_WIDTH (v_info) /
      (gfloat) GST_VIDEO_INFO_HEIGHT (v_info));
  gst_gl_context_clear_shader (src->base.context);
*/


  return TRUE;
}

static gboolean
_src_mandelbrot_draw (gpointer impl)
{
  struct SrcShader *src = impl;

  g_return_val_if_fail (src->base.context, FALSE);

  //TODO: exit with an error message (shader compiler mostly)
  if (!src->shader)
    exit (0);
  g_return_val_if_fail (src->shader, FALSE);

  GstGLFuncs *gl = src->base.context->gl_vtable;

  // gl->Viewport (0, 0, self->eye_width, self->eye_height);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gst_gl_shader_use (src->shader->shader);
  gst_gl_shader_set_uniform_1f (src->shader->shader, "time",
      (gfloat) src->base.src->running_time / GST_SECOND);

  Gst3DCameraArcball *camera = src->base.src->camera;

  gst_3d_camera_arcball_update_view (camera);
  gst_3d_shader_upload_matrix (src->shader, &camera->mvp, "mvp");

  GList *l;
  for (l = src->axesDebugMeshes; l != NULL; l = l->next) {
    Gst3DMesh *mesh = (Gst3DMesh *) l->data;
    gst_3d_mesh_bind (mesh);
    gst_3d_mesh_draw (mesh);
  }

  //gst_3d_mesh_draw_arrays(src->plane_mesh);

  // gl->BindVertexArray (0);
  gst_gl_context_clear_shader (src->base.context);

  return TRUE;
}

static void
_src_mandelbrot_free (gpointer impl)
{
  struct SrcShader *src = impl;

  if (!src)
    return;

  g_free (impl);
}

static gpointer
_src_mandelbrot_new (GstVRTestSrc * test)
{
  struct SrcShader *src = g_new0 (struct SrcShader, 1);

  src->base.src = test;

  return src;
}

static const struct SrcFuncs src_mandelbrot = {
  GST_VR_TEST_SRC_MANDELBROT,
  _src_mandelbrot_new,
  _src_mandelbrot_init,
  _src_mandelbrot_draw,
  _src_mandelbrot_free,
};


static const struct SrcFuncs *src_impls[] = {
  &src_mandelbrot,
};

const struct SrcFuncs *
gst_vr_test_src_get_src_funcs_for_pattern (GstVRTestSrcPattern pattern)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (src_impls); i++) {
    if (src_impls[i]->pattern == pattern)
      return src_impls[i];
  }

  return NULL;
}
