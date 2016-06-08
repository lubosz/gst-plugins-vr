/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2016> Matthew Waters <matthew@centricular.com>
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

#include "vrtestsrc.h"

#include "gst/3d/gst3dshader.h"
#include "gst/3d/gst3dmesh.h"
#include "gst/3d/gst3dcamera_arcball.h"
#include "gst/3d/gst3dscene.h"

struct GeometryScene
{
  struct BaseSceneImpl base;
  Gst3DScene *scene;
};

static gboolean
_scene_geometry_navigate (gpointer impl, GstEvent * event)
{
  struct GeometryScene *self = impl;
  gst_3d_scene_navigation_event (self->scene, event);
  return TRUE;
}

static gboolean
_scene_geometry_init (gpointer impl, GstGLContext * context,
    GstVideoInfo * v_info)
{
  struct GeometryScene *self = impl;
  self->base.context = context;
  self->scene = gst_3d_scene_new (context);
  self->scene->camera = GST_3D_CAMERA (gst_3d_camera_arcball_new ());

  /*
     Gst3DMesh * plane_mesh = gst_3d_mesh_new_plane (context, 1.0);
     Gst3DMesh *cube_mesh = gst_3d_mesh_new_cube (context);
   */

  Gst3DShader *uv_shader =
      gst_3d_shader_new_vert_frag (context, "mvp_uv.vert", "debug_uv.frag");

  Gst3DNode *axes_node = gst_3d_node_new_debug_axes (context);
  gst_3d_scene_append_node (self->scene, axes_node);

  Gst3DMesh *sphere_mesh = gst_3d_mesh_new_sphere (context, 0.5, 100, 100);
  Gst3DNode *sphere_node =
      gst_3d_node_new_from_mesh_shader (context, sphere_mesh, uv_shader);

  gst_3d_scene_append_node (self->scene, sphere_node);

  /*
     gst_gl_shader_use (axes_node->shader->shader);
     gst_gl_shader_use (self->shader);
     gst_gl_shader_set_uniform_1f (self->shader, "aspect_ratio",
     (gfloat) GST_VIDEO_INFO_WIDTH (v_info) /
     (gfloat) GST_VIDEO_INFO_HEIGHT (v_info));
     gst_gl_context_clear_shader (self->base.context);
   */
  return TRUE;
}

static gboolean
_scene_geometry_draw (gpointer impl)
{
  struct GeometryScene *self = impl;
  g_return_val_if_fail (self->base.context, FALSE);

  GstGLFuncs *gl = self->base.context->gl_vtable;
  /*
     gst_gl_shader_use (self->shader->shader);
     gst_gl_shader_set_uniform_1f (self->shader->shader, "time",
     (gfloat) self->base.src->running_time / GST_SECOND);
   */

  gst_3d_camera_update_view (self->scene->camera);
  gl->Enable (GL_DEPTH_TEST);
  gst_3d_scene_draw (self->scene, &self->scene->camera->mvp);
  gl->Disable (GL_DEPTH_TEST);

  return TRUE;
}

static void
_scene_geometry_free (gpointer impl)
{
  struct GeometryScene *self = impl;
  if (!self)
    return;
  gst_object_unref (self->scene);
  g_free (impl);
}

static gpointer
_scene_geometry_new (GstVRTestSrc * test)
{
  struct GeometryScene *scene = g_new0 (struct GeometryScene, 1);
  scene->base.src = test;
  return scene;
}

static const struct SceneFuncs scene_geometry = {
  GST_VR_TEST_SCENE_GEOMETRY,
  _scene_geometry_new,
  _scene_geometry_init,
  _scene_geometry_draw,
  _scene_geometry_navigate,
  _scene_geometry_free,
};


static const struct SceneFuncs *src_impls[] = {
  &scene_geometry,
};

const struct SceneFuncs *
gst_vr_test_src_get_funcs_for_scene (GstVRTestScene scene)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (src_impls); i++) {
    if (src_impls[i]->scene == scene)
      return src_impls[i];
  }

  return NULL;
}
