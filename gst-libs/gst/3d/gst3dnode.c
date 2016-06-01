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

#include "gst3dnode.h"
#include "gst3dmesh.h"

#define GST_CAT_DEFAULT gst_3d_node_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DNode, gst_3d_node, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_node_debug, "3dnode", 0, "node"));

void
gst_3d_node_init (Gst3DNode * self)
{
  self->context = NULL;
}

Gst3DNode *
gst_3d_node_new (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DNode *node = g_object_new (GST_3D_TYPE_NODE, NULL);
  node->context = gst_object_ref (context);
  return node;
}

static void
gst_3d_node_finalize (GObject * object)
{
  Gst3DNode *self = GST_3D_NODE (object);
  g_return_if_fail (self != NULL);

  if (self->context) {
    gst_object_unref (self->context);
    self->context = NULL;
  }

  G_OBJECT_CLASS (gst_3d_node_parent_class)->finalize (object);
}

static void
gst_3d_node_class_init (Gst3DNodeClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_node_finalize;
}

Gst3DNode *
gst_3d_node_new_debug_axes (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DNode *node = g_object_new (GST_3D_TYPE_NODE, NULL);
  node->context = gst_object_ref (context);

  node->shader =
      gst_3d_shader_new_vert_frag (context, "mvp_color.vert", "color.frag");

  gst_gl_shader_use (node->shader->shader);

  graphene_vec3_t from, to, color;
  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 1.f, 0.f, 0.f);
  graphene_vec3_init (&color, 1.f, 0.f, 0.f);
  Gst3DMesh *x_axis = gst_3d_mesh_new_line (context, &from, &to, &color);
  gst_3d_mesh_bind_shader (x_axis, node->shader);
  node->meshes = g_list_append (node->meshes, x_axis);

  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 0.f, 1.f, 0.f);
  graphene_vec3_init (&color, 0.f, 1.f, 0.f);
  Gst3DMesh *y_axis = gst_3d_mesh_new_line (context, &from, &to, &color);
  gst_3d_mesh_bind_shader (y_axis, node->shader);
  node->meshes = g_list_append (node->meshes, y_axis);

  graphene_vec3_init (&from, 0.f, 0.f, 0.f);
  graphene_vec3_init (&to, 0.f, 0.f, 1.f);
  graphene_vec3_init (&color, 0.f, 0.f, 1.f);
  Gst3DMesh *z_axis = gst_3d_mesh_new_line (context, &from, &to, &color);
  gst_3d_mesh_bind_shader (z_axis, node->shader);
  node->meshes = g_list_append (node->meshes, z_axis);

  return node;
}

void
gst_3d_node_draw (Gst3DNode * self)
{
  GList *l;
  for (l = self->meshes; l != NULL; l = l->next) {
    Gst3DMesh *mesh = (Gst3DMesh *) l->data;
    gst_3d_mesh_bind (mesh);
    gst_3d_mesh_draw (mesh);
  }
}

void
gst_3d_node_draw_wireframe (Gst3DNode * self)
{
  GList *l;
  for (l = self->meshes; l != NULL; l = l->next) {
    Gst3DMesh *mesh = (Gst3DMesh *) l->data;
    gst_3d_mesh_bind (mesh);
    gst_3d_mesh_draw_mode (mesh, GL_LINES);
  }
}
