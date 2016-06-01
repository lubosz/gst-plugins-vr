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

#include "../../gst-libs/gst/3d/gst3dshader.h"
#include "../../gst-libs/gst/3d/gst3dmesh.h"
#include "../../gst-libs/gst/3d/gst3dcamera_arcball.h"
#include "../../gst-libs/gst/3d/gst3dnode.h"

struct GeometryScene
{
  struct BaseSceneImpl base;
  Gst3DCameraArcball *camera;
  GList *nodes;
  gboolean wireframe_mode;
};

void
_toggle_wireframe_mode (struct GeometryScene *self)
{
  if (self->wireframe_mode)
    self->wireframe_mode = FALSE;
  else
    self->wireframe_mode = TRUE;
}

static gboolean
_scene_geometry_navigate (gpointer impl, GstEvent * event)
{
  struct GeometryScene *self = impl;
  gst_3d_camera_arcball_navigation_event (self->camera, event);

  GstNavigationEventType event_type = gst_navigation_event_get_type (event);
  switch (event_type) {
    case GST_NAVIGATION_EVENT_KEY_PRESS:{
      GstStructure *structure =
          (GstStructure *) gst_event_get_structure (event);
      const gchar *key = gst_structure_get_string (structure, "key");
      if (key != NULL && g_strcmp0 (key, "Tab") == 0)
        _toggle_wireframe_mode (self);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

static void
_scene_append_node (struct GeometryScene *self, Gst3DMesh * mesh,
    Gst3DShader * shader)
{
  Gst3DNode *node = gst_3d_node_new (self->base.context);
  node->meshes = g_list_append (node->meshes, mesh);
  node->shader = shader;
  gst_gl_shader_use (shader->shader);
  gst_3d_mesh_bind_shader (mesh, shader);
  self->nodes = g_list_append (self->nodes, node);
}

static gboolean
_scene_geometry_init (gpointer impl, GstGLContext * context,
    GstVideoInfo * v_info)
{
  // GstGLFuncs *gl = context->gl_vtable;

  struct GeometryScene *self = impl;
  self->base.context = context;
  self->camera = gst_3d_camera_arcball_new ();

/*
  if (self->shader)
    gst_object_unref (self->shader);
*/

  // Gst3DMesh * plane_mesh = gst_3d_mesh_new_plane (context, 1.0);

  Gst3DShader *uv_shader =
      gst_3d_shader_new_vert_frag (context, "mvp_uv.vert", "debug_uv.frag");
/*
  Gst3DMesh *cube_mesh = gst_3d_mesh_new_cube (context);
  _scene_append_node (self, cube_mesh, uv_shader);
*/

/*
  Gst3DNode *axes_node = gst_3d_node_new_debug_axes (context);
  self->nodes = g_list_append (self->nodes, axes_node);
*/

/*
  Gst3DMesh *sphere_mesh = gst_3d_mesh_new_sphere (context, 2.0, 15, 15);
  // sphere_mesh->draw_mode = GL_LINES;
  _scene_append_node (self, sphere_mesh, uv_shader);

  Gst3DMesh *top_cap_mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (top_cap_mesh);
  gst_3d_mesh_upload_sphere_top_cap (top_cap_mesh, 2.0, 15, 15);
  _scene_append_node (self, top_cap_mesh, uv_shader);

  Gst3DMesh *bottom_cap_mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (bottom_cap_mesh);
  gst_3d_mesh_upload_sphere_bottom_cap (bottom_cap_mesh, 2.0, 15, 15);
  _scene_append_node (self, bottom_cap_mesh, uv_shader);
*/

  Gst3DNode *sphere_node =
      gst_3d_node_new_sphere (context, uv_shader, 2.0, 15, 15);
  self->nodes = g_list_append (self->nodes, sphere_node);

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

  glPointSize (5.0);


  //TODO: exit with an error message (shader compiler mostly)
  // if (!self->shader)
  //  exit (0);
  // g_return_val_if_fail (self->shader, FALSE);

  GstGLFuncs *gl = self->base.context->gl_vtable;

  // gl->Viewport (0, 0, self->eye_width, self->eye_height);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /*
     gst_gl_shader_use (self->shader->shader);
     gst_gl_shader_set_uniform_1f (self->shader->shader, "time",
     (gfloat) self->base.src->running_time / GST_SECOND);
   */
  Gst3DCameraArcball *camera = self->camera;

  gst_3d_camera_arcball_update_view (camera);
  gl->Enable (GL_DEPTH_TEST);

  GList *l;
  for (l = self->nodes; l != NULL; l = l->next) {
    Gst3DNode *node = (Gst3DNode *) l->data;
    gst_gl_shader_use (node->shader->shader);
    gst_3d_shader_upload_matrix (node->shader, &camera->mvp, "mvp");
    if (self->wireframe_mode)
      gst_3d_node_draw_wireframe (node);
    else
      gst_3d_node_draw (node);
  }
  gl->Disable (GL_DEPTH_TEST);
  //gst_3d_mesh_draw_arrays(self->plane_mesh);

  // gl->BindVertexArray (0);
  // gst_gl_context_clear_shader (self->base.context);

  return TRUE;
}

static void
_scene_geometry_free (gpointer impl)
{
  struct GeometryScene *self = impl;
  if (!self)
    return;
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
