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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define GST_USE_UNSTABLE_API
#include <gst/gl/gl.h>

#include "gst3dmath.h"

#define GST_CAT_DEFAULT gst_3d_math_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DMath, gst_3d_math, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_math_debug, "3dmath", 0, "math"));

void
gst_3d_math_init (Gst3DMath * self)
{
}

Gst3DMath *
gst_3d_math_new (GstGLContext * context)
{
  Gst3DMath *math = g_object_new (GST_3D_TYPE_MATH, NULL);
  return math;
}

static void
gst_3d_math_finalize (GObject * object)
{
  Gst3DMath *self = GST_3D_MATH (object);
  g_return_if_fail (self != NULL);
  G_OBJECT_CLASS (gst_3d_math_parent_class)->finalize (object);
}

static void
gst_3d_math_class_init (Gst3DMathClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_math_finalize;
}

void
gst_3d_math_matrix_negate_component (const graphene_matrix_t * matrix, guint n,
    guint m, graphene_matrix_t * result)
{
  float values[16];
  for (int x = 0; x < 4; x++)
    for (int y = 0; y < 4; y++)
      values[x * 4 + y] = graphene_matrix_get_value (matrix, x, y);
  values[n * 4 + m] = -graphene_matrix_get_value (matrix, n, m);
  graphene_matrix_init_from_float (result, values);
}

void
gst_3d_math_matrix_hadamard_product (const graphene_matrix_t * a,
    const graphene_matrix_t * b, graphene_matrix_t * result)
{
  float values[16];
  for (int x = 0; x < 4; x++)
    for (int y = 0; y < 4; y++)
      values[x * 4 + y] =
          graphene_matrix_get_value (a, x, y) * graphene_matrix_get_value (b, x,
          y);
  graphene_matrix_init_from_float (result, values);
}



void
gst_3d_math_vec3_negate (const graphene_vec3_t * vec, graphene_vec3_t * res)
{
  return graphene_vec3_subtract (graphene_vec3_zero (), vec, res);
}
