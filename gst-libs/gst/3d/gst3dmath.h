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

#ifndef __GST_3D_MATH_H__
#define __GST_3D_MATH_H__


#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>
#include <graphene.h>
#include "gst3dnode.h"

G_BEGIN_DECLS
#define GST_3D_TYPE_MATH            (gst_3d_math_get_type ())
#define GST_3D_MATH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_3D_TYPE_MATH, Gst3DMath))
#define GST_3D_MATH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_3D_TYPE_MATH, Gst3DMathClass))
#define GST_IS_3D_MATH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_3D_TYPE_MATH))
#define GST_IS_3D_MATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_3D_TYPE_MATH))
#define GST_3D_MATH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_3D_TYPE_MATH, Gst3DMathClass))
typedef struct _Gst3DMath Gst3DMath;
typedef struct _Gst3DMathClass Gst3DMathClass;

struct _Gst3DMath
{
  /*< private > */
  GstObject parent;
  
};

struct _Gst3DMathClass
{
  GstObjectClass parent_class;
};

Gst3DMath *gst_3d_math_new (GstGLContext * context);
void gst_3d_math_append_node(Gst3DMath *self, Gst3DNode * node);
void gst_3d_math_draw(Gst3DMath *self, graphene_matrix_t * mvp);

void
gst_3d_math_matrix_negate_component (const graphene_matrix_t * matrix, guint n, guint m,
    graphene_matrix_t * result);

void
gst_3d_math_vec3_negate(const graphene_vec3_t *vec, graphene_vec3_t *res);

void
gst_3d_math_matrix_hadamard_product (const graphene_matrix_t * a, const graphene_matrix_t * b,
    graphene_matrix_t * result);

GType gst_3d_math_get_type (void);

G_END_DECLS
#endif /* __GST_3D_MATH_H__ */
