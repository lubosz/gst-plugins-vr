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

#ifndef __GST_3D_SCENE_H__
#define __GST_3D_SCENE_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene.h>
#include "gst3dnode.h"
#include "gst3dcamera.h"

G_BEGIN_DECLS
#define GST_3D_TYPE_SCENE            (gst_3d_scene_get_type ())
#define GST_3D_SCENE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_SCENE, Gst3DScene))
#define GST_3D_SCENE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_SCENE, Gst3DSceneClass))
#define GST_IS_3D_SCENE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_SCENE))
#define GST_IS_3D_SCENE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_SCENE))
#define GST_3D_SCENE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_SCENE, Gst3DSceneClass))
typedef struct _Gst3DScene Gst3DScene;
typedef struct _Gst3DSceneClass Gst3DSceneClass;

struct _Gst3DScene
{
  /*< private > */
  GstObject parent;
  GstGLContext *context;
  
  gboolean wireframe_mode;
  void (*node_draw_funct) (Gst3DNode *);

  Gst3DCamera *camera;
    
  GList *nodes;
};

struct _Gst3DSceneClass
{
  GstObjectClass parent_class;
};

Gst3DScene *gst_3d_scene_new (GstGLContext * context);
void gst_3d_scene_append_node(Gst3DScene *self, Gst3DNode * node);
void gst_3d_scene_draw(Gst3DScene * self, graphene_matrix_t * mvp);
void gst_3d_scene_toggle_wireframe_mode (Gst3DScene *self);
void gst_3d_scene_navigation_event (Gst3DScene *self, GstEvent * event);

GType gst_3d_scene_get_type (void);

G_END_DECLS
#endif /* __GST_3D_SCENE_H__ */
