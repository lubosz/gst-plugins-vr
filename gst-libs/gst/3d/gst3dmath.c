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

#include "gst3dmath.h"

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
      values[x * 4 + y] = graphene_matrix_get_value (a, x, y)
          * graphene_matrix_get_value (b, x, y);
  graphene_matrix_init_from_float (result, values);
}

void
gst_3d_math_vec3_print (const gchar * name, graphene_vec3_t * vector)
{
  g_print ("%s %f %f %f\n", name, graphene_vec3_get_x (vector),
      graphene_vec3_get_y (vector), graphene_vec3_get_z (vector));
}

void
gst_3d_math_vec3_negate (const graphene_vec3_t * vector,
    graphene_vec3_t * result)
{
  return graphene_vec3_subtract (graphene_vec3_zero (), vector, result);
}
