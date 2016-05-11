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

#ifndef __GST_3D_SHADER_H__
#define __GST_3D_SHADER_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene-gobject.h>

G_BEGIN_DECLS

#define GST_3D_TYPE_SHADER            (gst_3d_shader_get_type ())
#define GST_3D_SHADER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_SHADER, Gst3DShader))
#define GST_3D_SHADER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_SHADER, Gst3DShaderClass))
#define GST_IS_3D_SHADER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_SHADER))
#define GST_IS_3D_SHADER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_SHADER))
#define GST_3D_SHADER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_SHADER, Gst3DShaderClass))
typedef struct _Gst3DShader  Gst3DShader;
typedef struct _Gst3DShaderClass  Gst3DShaderClass;

struct _Gst3DShader
{
  /*< private >*/
  GstObject parent;
  
  GstGLContext * context;

  GstGLShader *shader;
  GLint attr_position;
  GLint attr_uv;
};

struct _Gst3DShaderClass {
  GstObjectClass parent_class;
};

Gst3DShader * gst_3d_shader_new            (GstGLContext * context);
GType       gst_3d_shader_get_type       (void);

const char * gst_3d_shader_read (const char *file);
void gst_3d_shader_bind (Gst3DShader * self);
void gst_3d_shader_disable_attribs(Gst3DShader * self);
void gst_3d_shader_enable_attribs (Gst3DShader * self);
gboolean gst_3d_shader_from_vert_frag (Gst3DShader * self, const gchar *vertex, const gchar  *fragment);
void gst_3d_shader_delete(Gst3DShader * self);

void gst_3d_shader_upload_matrix(Gst3DShader * self, graphene_matrix_t * mat, const gchar* name);
void gst_3d_shader_upload_vec2(Gst3DShader * self, graphene_vec2_t * vec, const gchar* name);

G_END_DECLS

#endif /* __GST_3D_SHADER_H__ */
