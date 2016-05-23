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

#ifndef __GST_3D_CAMERA_H__
#define __GST_3D_CAMERA_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene.h>
#include <openhmd/openhmd.h>
G_BEGIN_DECLS

#define GST_3D_TYPE_CAMERA            (gst_3d_camera_get_type ())
#define GST_3D_CAMERA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_CAMERA, Gst3DCamera))
#define GST_3D_CAMERA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_CAMERA, Gst3DCameraClass))
#define GST_IS_3D_CAMERA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_CAMERA))
#define GST_IS_3D_CAMERA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_CAMERA))
#define GST_3D_CAMERA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_CAMERA, Gst3DCameraClass))
typedef struct _Gst3DCamera  Gst3DCamera;
typedef struct _Gst3DCameraClass  Gst3DCameraClass;

struct _Gst3DCamera
{
  /*< private >*/
  GstObject parent;
  
  graphene_matrix_t mvp;

  /* position */
  graphene_vec3_t eye;
  graphene_vec3_t center;
  graphene_vec3_t up;

  gfloat xtranslation;
  gfloat ytranslation;
  gfloat ztranslation;

  /* perspective */
  gfloat fov;
  gfloat aspect;
  gfloat znear;
  gfloat zfar;
  gboolean ortho;
    
  /* arcball cam */
  gfloat center_distance;
  gfloat scroll_speed;
  gfloat rotation_speed;
  gfloat theta;
  gfloat phi;
  
  gdouble cursor_last_x;
  gdouble cursor_last_y;
  
  /* stereo */
  gfloat eye_separation;
  
  graphene_matrix_t left_vp_matrix;
  graphene_matrix_t right_vp_matrix;
  
  
  ohmd_device *device;
  ohmd_context *hmd_context;
};

struct _Gst3DCameraClass {
  GstObjectClass parent_class;
};

Gst3DCamera * gst_3d_camera_new            (void);

void gst_3d_camera_update_view (Gst3DCamera * self);
void gst_3d_camera_update_view_mvp(Gst3DCamera * self);
void gst_3d_camera_update_view_arcball(Gst3DCamera * self);


void gst_3d_camera_translate_arcball (Gst3DCamera * self, float z);
void gst_3d_camera_rotate_arcball (Gst3DCamera * self, float x, float y);

void gst_3d_camera_inc_eye_sep(Gst3DCamera * self);
void gst_3d_camera_dec_eye_sep(Gst3DCamera * self);

GType       gst_3d_camera_get_type       (void);

G_END_DECLS

#endif /* __GST_3D_CAMERA_H__ */
