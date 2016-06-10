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

#include "gst3dscene.h"
#include "gst3dcamera_hmd.h"

#define GST_CAT_DEFAULT gst_3d_scene_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DScene, gst_3d_scene, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_scene_debug, "3dscene", 0, "scene"));

void
gst_3d_scene_init (Gst3DScene * self)
{
  self->wireframe_mode = FALSE;
  self->camera = NULL;
  self->renderer = NULL;
  self->context = NULL;
  self->gl_initialized = FALSE;
  self->node_draw_func = &gst_3d_node_draw;
}

Gst3DScene *
gst_3d_scene_new (Gst3DCamera * camera, void (*_init_func) (Gst3DScene *))
{
  g_return_val_if_fail (GST_IS_3D_CAMERA (camera), NULL);
  Gst3DScene *scene = g_object_new (GST_3D_TYPE_SCENE, NULL);
  scene->camera = gst_object_ref (camera);
  scene->gl_init_func = _init_func;
  gst_3d_camera_update_view (scene->camera);
  return scene;
}

static void
gst_3d_scene_finalize (GObject * object)
{
  Gst3DScene *self = GST_3D_SCENE (object);
  g_return_if_fail (self != NULL);

  if (self->camera) {
    gst_object_unref (self->camera);
    self->camera = NULL;
  }

  if (self->context) {
    gst_object_unref (self->context);
    self->context = NULL;
  }

  GList *l;
  for (l = self->nodes; l != NULL; l = l->next) {
    Gst3DNode *node = (Gst3DNode *) l->data;
    gst_object_unref (node);
  }

  G_OBJECT_CLASS (gst_3d_scene_parent_class)->finalize (object);
}

static void
gst_3d_scene_class_init (Gst3DSceneClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_scene_finalize;
}

void
gst_3d_scene_init_gl (Gst3DScene * self, GstGLContext * context)
{
  if (self->gl_initialized)
    return;
  self->context = gst_object_ref (context);
  self->gl_init_func (self);
  self->gl_initialized = TRUE;
  gst_3d_scene_init_stereo_renderer (self, context);
}

void
gst_3d_scene_draw_nodes (Gst3DScene * self, graphene_matrix_t * mvp)
{
  GList *l;
  for (l = self->nodes; l != NULL; l = l->next) {
    Gst3DNode *node = (Gst3DNode *) l->data;
    gst_3d_shader_bind (node->shader);
    gst_3d_shader_upload_matrix (node->shader, mvp, "mvp");
    self->node_draw_func (node);
  }
}

void
gst_3d_scene_draw (Gst3DScene * self)
{
  gst_3d_camera_update_view (self->camera);
  if (GST_IS_3D_CAMERA_HMD (self->camera))
    gst_3d_renderer_draw_stereo (self->renderer, self);
  else
    gst_3d_scene_draw_nodes (self, &self->camera->mvp);
  gst_3d_renderer_clear_state (self->renderer);
}

void
gst_3d_scene_append_node (Gst3DScene * self, Gst3DNode * node)
{
  self->nodes = g_list_append (self->nodes, node);
}

void
gst_3d_scene_toggle_wireframe_mode (Gst3DScene * self)
{
  if (self->wireframe_mode) {
    self->wireframe_mode = FALSE;
    self->node_draw_func = &gst_3d_node_draw;
  } else {
    self->wireframe_mode = TRUE;
    self->node_draw_func = &gst_3d_node_draw_wireframe;
  }
}

void
gst_3d_scene_navigation_event (Gst3DScene * self, GstEvent * event)
{
  gst_3d_camera_navigation_event (self->camera, event);

  GstNavigationEventType event_type = gst_navigation_event_get_type (event);
  switch (event_type) {
    case GST_NAVIGATION_EVENT_KEY_PRESS:{
      GstStructure *structure =
          (GstStructure *) gst_event_get_structure (event);
      const gchar *key = gst_structure_get_string (structure, "key");
      if (key != NULL && g_strcmp0 (key, "Tab") == 0)
        gst_3d_scene_toggle_wireframe_mode (self);
      break;
    }
    default:
      break;
  }
}

/* stereo */

gboolean
gst_3d_scene_init_hmd (Gst3DScene * self)
{
  if (GST_IS_3D_CAMERA_HMD (self->camera)) {
    Gst3DHmd *hmd = GST_3D_CAMERA_HMD (self->camera)->hmd;
    if (!hmd->device)
      return FALSE;
  }
  return TRUE;
}

void
gst_3d_scene_init_stereo_renderer (Gst3DScene * self, GstGLContext * context)
{
  self->renderer = gst_3d_renderer_new (context);
  if (GST_IS_3D_CAMERA_HMD (self->camera)) {
    Gst3DCameraHmd *hmd_cam = GST_3D_CAMERA_HMD (self->camera);
    Gst3DHmd *hmd = hmd_cam->hmd;
    gst_3d_renderer_stereo_init_from_hmd (self->renderer, hmd);
    gst_3d_renderer_init_stereo (self->renderer, self->camera);
  }
}
