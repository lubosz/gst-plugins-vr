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

#include "gst3dglm.h"

//#define GLM_FORCE_LEFT_HANDED 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GST_CAT_DEFAULT gst_3d_glm_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
G_DEFINE_TYPE_WITH_CODE (Gst3DGlm, gst_3d_glm, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_glm_debug, "3dglm", 0, "glm"));

void gst_3d_glm_init (Gst3DGlm * self) {}
static void gst_3d_glm_class_init (Gst3DGlmClass * klass) {}

graphene_matrix_t gst_3d_glm_look_at(graphene_vec3_t *eye, graphene_vec3_t *center, graphene_vec3_t *up) 
{
  glm::mat4 glm_view;
  glm::vec3 glm_center (graphene_vec3_get_x (center), graphene_vec3_get_y (center), graphene_vec3_get_z (center));
  glm::vec3 glm_up (graphene_vec3_get_x (up), graphene_vec3_get_y (up), graphene_vec3_get_z (up));
  glm::vec3 glm_eye (graphene_vec3_get_x (eye), graphene_vec3_get_y (eye), graphene_vec3_get_z (eye));
  glm_view = glm::lookAt (glm_eye, glm_center, glm_up);
  graphene_matrix_t view;
  graphene_matrix_init_from_float (&view, &glm_view[0][0]);
  return view;
}

