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

#ifndef __GST_3D_RENDERER_H__
#define __GST_3D_RENDERER_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <gst/gl/gstglfuncs.h>

#include "gst3dmesh.h"
#include "gst3dshader.h"
#include "gst3dcamera.h"

#ifdef HAVE_OPENHMD
#include "gst3dhmd.h"
#endif

G_BEGIN_DECLS
#define GST_3D_TYPE_RENDERER            (gst_3d_renderer_get_type ())
#define GST_3D_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_RENDERER, Gst3DRenderer))
#define GST_3D_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_RENDERER, Gst3DRendererClass))
#define GST_IS_3D_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_RENDERER))
#define GST_IS_3D_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_RENDERER))
#define GST_3D_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_RENDERER, Gst3DRendererClass))
typedef struct _Gst3DRenderer Gst3DRenderer;
typedef struct _Gst3DRendererClass Gst3DRendererClass;

typedef struct _Gst3DScene Gst3DScene;

struct _Gst3DRenderer
{
  /*< private > */
  GstObject parent;
  GstGLContext *context;
  
  Gst3DMesh *render_plane;

  Gst3DShader *shader;
  
  GLuint left_color_tex, left_fbo;
  GLuint right_color_tex, right_fbo;
  
  guint eye_width;
  guint eye_height;
  
  gfloat filter_aspect;
};

struct _Gst3DRendererClass
{
  GstObjectClass parent_class;
};

Gst3DRenderer *gst_3d_renderer_new (GstGLContext * context);
GType gst_3d_renderer_get_type (void);
void gst_3d_renderer_create_fbo (GstGLFuncs *gl, GLuint * fbo, GLuint * color_tex, int width, int height);
void gst_3d_renderer_init_stereo (Gst3DRenderer * self, Gst3DCamera *cam);
void gst_3d_renderer_draw_stereo (Gst3DRenderer * self, Gst3DScene *scene);

void gst_3d_renderer_draw_stereo_shader_proj (Gst3DRenderer * self, Gst3DScene * scene);
void gst_3d_renderer_init_stereo_shader_proj (Gst3DRenderer * self, Gst3DCamera * cam);

#ifdef HAVE_OPENHMD
gboolean gst_3d_renderer_stereo_init_from_hmd (Gst3DRenderer * self, Gst3DHmd * hmd);
#endif
gboolean gst_3d_renderer_stero_init_from_filter (Gst3DRenderer * self, GstGLFilter * filter);

G_END_DECLS
#endif /* __GST_3D_RENDERER_H__ */
