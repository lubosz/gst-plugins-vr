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

#ifndef __GST_3D_MESH_H__
#define __GST_3D_MESH_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>

G_BEGIN_DECLS
#define GST_3D_TYPE_MESH            (gst_3d_mesh_get_type ())
#define GST_3D_MESH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_MESH, Gst3DMesh))
#define GST_3D_MESH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_MESH, Gst3DMeshClass))
#define GST_IS_3D_MESH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_MESH))
#define GST_IS_3D_MESH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_MESH))
#define GST_3D_MESH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_MESH, Gst3DMeshClass))
typedef struct _Gst3DMesh Gst3DMesh;
typedef struct _Gst3DMeshClass Gst3DMeshClass;

struct _Gst3DMesh
{
  /*< private > */
  GstObject parent;

  GstGLContext *context;

  GLuint vao;
  GLuint vbo_indices;
  GLuint vbo_positions;
  GLuint vbo_uv;

  unsigned index_size;
  unsigned vector_length;

  GLenum draw_mode;
};

struct _Gst3DMeshClass
{
  GstObjectClass parent_class;
};

Gst3DMesh *gst_3d_mesh_new (GstGLContext * context);
void gst_3d_mesh_init_buffers (Gst3DMesh * self);
gboolean gst_3d_mesh_has_buffers (Gst3DMesh * self);
void gst_3d_mesh_unbind_buffers (Gst3DMesh * self);
void gst_3d_mesh_bind_buffers (Gst3DMesh * self, GLint attr_position,
    GLint attr_uv);
void gst_3d_mesh_bind (Gst3DMesh * self);
void gst_3d_mesh_draw (Gst3DMesh * self);
void gst_3d_mesh_upload_sphere (Gst3DMesh * self, float radius, unsigned stacks,
    unsigned slices);
void gst_3d_mesh_upload_plane (Gst3DMesh * self, float aspect);
void gst_3d_mesh_upload_point_plane (Gst3DMesh * self, unsigned width,
    unsigned height);
void gst_3d_mesh_draw_arrays (Gst3DMesh * self);
GType gst_3d_mesh_get_type (void);

G_END_DECLS
#endif /* __GST_3D_MESH_H__ */
