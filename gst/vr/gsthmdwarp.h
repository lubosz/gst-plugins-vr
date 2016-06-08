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


#ifndef _GST_HMD_WARP_H_
#define _GST_HMD_WARP_H_

#include <graphene.h>
#include <gst/gl/gstglfilter.h>
#include "gst/3d/gst3dmesh.h"
#include "gst/3d/gst3dcamera.h"
#include "gst/3d/gst3dshader.h"

G_BEGIN_DECLS
#define GST_TYPE_HMD_WARP            (gst_hmd_warp_get_type())
#define GST_HMD_WARP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HMD_WARP,GstHmdWarp))
#define GST_IS_HMD_WARP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HMD_WARP))
#define GST_HMD_WARP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_HMD_WARP,GstHmdWarpClass))
#define GST_IS_HMD_WARP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_HMD_WARP))
#define GST_HMD_WARP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_HMD_WARP,GstHmdWarpClass))
typedef struct _GstHmdWarp GstHmdWarp;
typedef struct _GstHmdWarpClass GstHmdWarpClass;

struct _GstHmdWarp
{
  GstGLFilter filter;
  Gst3DShader *shader;
  guint in_tex;
  gboolean caps_change;
  graphene_vec2_t screen_size;
  Gst3DMesh *render_plane;
  float aspect;
};

struct _GstHmdWarpClass
{
  GstGLFilterClass filter_class;
};

GType gst_hmd_warp_get_type (void);

G_END_DECLS
#endif /* _GST_HMD_WARP_H_ */
