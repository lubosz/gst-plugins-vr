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

G_BEGIN_DECLS

#define GST_3D_TYPE_HMD            (gst_3d_hmd_get_type ())
#define GST_3D_HMD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_HMD, Gst3DHmd))
#define GST_3D_HMD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_HMD, Gst3DHmdClass))
#define GST_IS_3D_HMD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_HMD))
#define GST_IS_3D_HMD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_HMD))
#define GST_3D_HMD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_HMD, Gst3DHmdClass))
typedef struct _Gst3DHmd  Gst3DHmd;
typedef struct _Gst3DHmdClass  Gst3DHmdClass;

struct _Gst3DHmd
{
  /*< private >*/
  GstObject parent;
  GstGLContext * context;
};

struct _Gst3DHmdClass {
  GstObjectClass parent_class;
};

Gst3DHmd * gst_3d_hmd_new            (GstGLContext * context);
GType       gst_3d_hmd_get_type       (void);

G_END_DECLS

#endif /* __GST_3D_HMD_H__ */
