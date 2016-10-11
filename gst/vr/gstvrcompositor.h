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

#ifndef _GST_VR_COMPOSITOR_H_
#define _GST_VR_COMPOSITOR_H_

#include <graphene.h>
#include <gst/gl/gstglfilter.h>
#include "gst/3d/gst3dmesh.h"
#include "gst/3d/gst3dshader.h"
#include "gst/3d/gst3drenderer.h"
#include "gst/3d/gst3dscene.h"
#include "gst/3d/gst3drenderer.h"

#ifdef HAVE_OPENHMD
#include "gst/3d/gst3dcamera_hmd.h"
#endif

G_BEGIN_DECLS
#define GST_TYPE_VR_COMPOSITOR            (gst_vr_compositor_get_type())
#define GST_VR_COMPOSITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VR_COMPOSITOR,GstVRCompositor))
#define GST_IS_VR_COMPOSITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VR_COMPOSITOR))
#define GST_VR_COMPOSITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_VR_COMPOSITOR,GstVRCompositorClass))
#define GST_IS_VR_COMPOSITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_VR_COMPOSITOR))
#define GST_VR_COMPOSITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_VR_COMPOSITOR,GstVRCompositorClass))
typedef struct _GstVRCompositor GstVRCompositor;
typedef struct _GstVRCompositorClass GstVRCompositorClass;

struct _GstVRCompositor
{
  GstGLFilter filter;

  GstGLMemory * in_tex;
  gboolean caps_change;

  Gst3DScene *scene;
};

struct _GstVRCompositorClass
{
  GstGLFilterClass filter_class;
};

GType gst_vr_compositor_get_type (void);

G_END_DECLS
#endif /* _GST_VR_COMPOSITOR_H_ */
