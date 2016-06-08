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

#ifndef __GST_3D_HMD_H__
#define __GST_3D_HMD_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <openhmd/openhmd.h>

G_BEGIN_DECLS
#define GST_3D_TYPE_HMD            (gst_3d_hmd_get_type ())
#define GST_3D_HMD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_HMD, Gst3DHmd))
#define GST_3D_HMD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_HMD, Gst3DHmdClass))
#define GST_IS_3D_HMD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_HMD))
#define GST_IS_3D_HMD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_HMD))
#define GST_3D_HMD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_HMD, Gst3DHmdClass))
typedef struct _Gst3DHmd Gst3DHmd;
typedef struct _Gst3DHmdClass Gst3DHmdClass;

struct _Gst3DHmd
{
  /*< private > */
  GstObject parent;
  
  ohmd_device *device;
  ohmd_context *hmd_context;
  
  int screen_width;
  int screen_height;
  
  float left_fov;
  float left_aspect;
  float right_fov;
  float right_aspect;
  
  float interpupillary_distance;
  float zfar;
  float znear;
  
  gfloat eye_separation;
};

struct _Gst3DHmdClass
{
  GstObjectClass parent_class;
};

Gst3DHmd *gst_3d_hmd_new (void);
GType gst_3d_hmd_get_type (void);
graphene_matrix_t gst_3d_hmd_get_matrix (Gst3DHmd * self, ohmd_float_value type);
graphene_quaternion_t gst_3d_hmd_get_quaternion (Gst3DHmd * self);
void gst_3d_hmd_eye_sep_inc (Gst3DHmd * self);
void gst_3d_hmd_eye_sep_dec (Gst3DHmd * self);
void gst_3d_hmd_reset(Gst3DHmd * self);

guint gst_3d_hmd_get_eye_width (Gst3DHmd * self);
guint gst_3d_hmd_get_eye_height (Gst3DHmd * self);
float gst_3d_hmd_get_screen_aspect (Gst3DHmd * self);
float gst_3d_hmd_get_eye_aspect (Gst3DHmd * self);

void gst_3d_hmd_update (Gst3DHmd * self);

G_END_DECLS
#endif /* __GST_3D_HMD_H__ */
