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

#ifndef _GST_POINT_CLOUD_BUILDER_H_
#define _GST_POINT_CLOUD_BUILDER_H_

#include <graphene.h>
#include <gst/gl/gstglfilter.h>
#include "../../gst-libs/gst/3d/gst3dmesh.h"
#include "../../gst-libs/gst/3d/gst3dcamera.h"
#include "../../gst-libs/gst/3d/gst3dshader.h"

G_BEGIN_DECLS
#define GST_TYPE_POINT_CLOUD_BUILDER            (gst_point_cloud_builder_get_type())
#define GST_POINT_CLOUD_BUILDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_POINT_CLOUD_BUILDER,GstPointCloudBuilder))
#define GST_IS_POINT_CLOUD_BUILDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_POINT_CLOUD_BUILDER))
#define GST_POINT_CLOUD_BUILDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_POINT_CLOUD_BUILDER,GstPointCloudBuilderClass))
#define GST_IS_POINT_CLOUD_BUILDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_POINT_CLOUD_BUILDER))
#define GST_POINT_CLOUD_BUILDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_POINT_CLOUD_BUILDER,GstPointCloudBuilderClass))
typedef struct _GstPointCloudBuilder GstPointCloudBuilder;
typedef struct _GstPointCloudBuilderClass GstPointCloudBuilderClass;

struct _GstPointCloudBuilder
{
  GstGLFilter filter;

  guint in_tex;

  guint eye_width;
  guint eye_height;

  GLenum render_mode;

  gboolean caps_change;

  GList *pushed_buttons;

  Gst3DMesh * mesh;
  Gst3DMesh * render_plane;
  
  int pressed_mouse_button;

  Gst3DShader * shader;
  Gst3DCamera * camera;
  
  GLuint left_color_tex, left_fbo;
  GLuint right_color_tex, right_fbo;
  GLint default_fbo;
};

struct _GstPointCloudBuilderClass
{
  GstGLFilterClass filter_class;
};

GType gst_point_cloud_builder_get_type (void);

G_END_DECLS
#endif /* _GST_POINT_CLOUD_BUILDER_H_ */
