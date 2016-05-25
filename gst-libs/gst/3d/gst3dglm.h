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

#ifndef __GST_3D_GLM_H__
#define __GST_3D_GLM_H__

#include <graphene.h>

#include <gst/gst.h>
#include <gst/gl/gstgl_fwd.h>

G_BEGIN_DECLS

struct Gst3DGlm{};
struct Gst3DGlmClass{};

GType gst_3d_glm_get_type (void);

graphene_matrix_t gst_3d_glm_look_at(graphene_vec3_t *eye, graphene_vec3_t *center, graphene_vec3_t *up);

G_END_DECLS
#endif /* __GST_3D_GLM_H__ */
