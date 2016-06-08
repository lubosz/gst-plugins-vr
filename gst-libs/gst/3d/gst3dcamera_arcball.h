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

#ifndef __GST_3D_CAMERA_ARCBALL_H__
#define __GST_3D_CAMERA_ARCBALL_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene.h>
#include "gst3dhmd.h"
#include "gst3dcamera.h"

G_BEGIN_DECLS
#define GST_3D_TYPE_CAMERA_ARCBALL            (gst_3d_camera_arcball_get_type ())
#define GST_3D_CAMERA_ARCBALL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_CAMERA_ARCBALL, Gst3DCameraArcball))
#define GST_3D_CAMERA_ARCBALL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_CAMERA_ARCBALL, Gst3DCameraArcballClass))
#define GST_IS_3D_CAMERA_ARCBALL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_CAMERA_ARCBALL))
#define GST_IS_3D_CAMERA_ARCBALL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_CAMERA_ARCBALL))
#define GST_3D_CAMERA_ARCBALL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_CAMERA_ARCBALL, Gst3DCameraArcballClass))
typedef struct _Gst3DCameraArcball Gst3DCameraArcball;
typedef struct _Gst3DCameraArcballClass Gst3DCameraArcballClass;

struct _Gst3DCameraArcball
{
  Gst3DCamera parent;

  gfloat center_distance;
  gfloat scroll_speed;
  gdouble rotation_speed;
  gfloat theta;
  gfloat phi;
};

struct _Gst3DCameraArcballClass
{
  Gst3DCameraClass parent_class;
};

Gst3DCameraArcball *gst_3d_camera_arcball_new (void);

void gst_3d_camera_arcball_translate (Gst3DCameraArcball * self, float z);
void gst_3d_camera_arcball_rotate (Gst3DCameraArcball * self, gdouble x, gdouble y);

GType gst_3d_camera_arcball_get_type (void);

G_END_DECLS
#endif /* __GST_3D_CAMERA_ARCBALL_H__ */
