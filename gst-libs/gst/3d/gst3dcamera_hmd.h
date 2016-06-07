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

#ifndef __GST_3D_CAMERA_HMD_H__
#define __GST_3D_CAMERA_HMD_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene.h>
#include "gst3dhmd.h"
#include "gst3dcamera.h"
#include "gst3dhmd.h"

typedef enum Gst3DHmdQueryType
{
  HMD_QUERY_TYPE_QUATERNION_MONO,
  HMD_QUERY_TYPE_QUATERNION_STEREO,
  HMD_QUERY_TYPE_MATRIX_STEREO,
  HMD_QUERY_TYPE_NONE,
} Gst3DHmdQueryType;

G_BEGIN_DECLS
#define GST_3D_TYPE_CAMERA_HMD            (gst_3d_camera_hmd_get_type ())
#define GST_3D_CAMERA_HMD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_CAMERA_HMD, Gst3DCameraHmd))
#define GST_3D_CAMERA_HMD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_CAMERA_HMD, Gst3DCameraHmdClass))
#define GST_IS_3D_CAMERA_HMD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_CAMERA_HMD))
#define GST_IS_3D_CAMERA_HMD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_CAMERA_HMD))
#define GST_3D_CAMERA_HMD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_CAMERA_HMD, Gst3DCameraHmdClass))
typedef struct _Gst3DCameraHmd Gst3DCameraHmd;
typedef struct _Gst3DCameraHmdClass Gst3DCameraHmdClass;

struct _Gst3DCameraHmd
{
  /*< private > */
  GstObject parent;
  
  graphene_matrix_t left_vp_matrix;
  graphene_matrix_t right_vp_matrix;
  
  Gst3DHmdQueryType query_type;
  
  Gst3DHmd * hmd;
};

struct _Gst3DCameraHmdClass
{
  Gst3DCameraClass parent_class;
};

Gst3DCameraHmd *gst_3d_camera_hmd_new (void);

void gst_3d_camera_hmd_update_view (Gst3DCameraHmd * self);
void gst_3d_camera_hmd_navigation_event (Gst3DCameraHmd * self,
    GstEvent * event);

void gst_3d_camera_hmd_update_view_from_matrix (Gst3DCameraHmd * self);
void gst_3d_camera_hmd_update_view_from_quaternion (Gst3DCameraHmd * self);

void
gst_3d_camera_hmd_update_view_from_quaternion_stereo (Gst3DCameraHmd * self);

float gst_3d_camera_hmd_get_screen_aspect (Gst3DCameraHmd * self);

guint gst_3d_camera_hmd_get_eye_width (Gst3DCameraHmd * self);
guint gst_3d_camera_hmd_get_eye_height (Gst3DCameraHmd * self);
float gst_3d_camera_hmd_get_eye_aspect (Gst3DCameraHmd * self);

GType gst_3d_camera_hmd_get_type (void);

G_END_DECLS
#endif /* __GST_3D_CAMERA_HMD_H__ */
