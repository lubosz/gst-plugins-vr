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

#include "gst3dmesh.h"

#define GST_CAT_DEFAULT gst_3d_mesh_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

G_DEFINE_TYPE_WITH_CODE (Gst3DMesh, gst_3d_mesh, GST_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (gst_3d_mesh_debug, "3dmesh", 0, "mesh"));

void
gst_3d_mesh_init (Gst3DMesh * self)
{
  self->index_size = 0;
  self->vao = 0;
  self->vbo_positions = 0;
  self->vbo_indices = 0;
  self->vbo_uv = 0;
}

Gst3DMesh *
gst_3d_mesh_new (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = g_object_new (GST_3D_TYPE_MESH, NULL);
  mesh->context = gst_object_ref (context);
  return mesh;
}

static void
gst_3d_mesh_finalize (GObject * object)
{
  Gst3DMesh *self = GST_3D_MESH (object);
  g_return_if_fail (self != NULL);

  GstGLFuncs *gl = self->context->gl_vtable;
  if (self->vao) {
    gl->DeleteVertexArrays (1, &self->vao);
    self->vao = 0;
  }

  if (self->vbo_positions) {
    gl->DeleteBuffers (1, &self->vbo_positions);
    self->vbo_positions = 0;
  }

  if (self->vbo_indices) {
    gl->DeleteBuffers (1, &self->vbo_indices);
    self->vbo_indices = 0;
  }

  if (self->context) {
    gst_object_unref (self->context);
    self->context = NULL;
  }
  
  G_OBJECT_CLASS (gst_3d_mesh_parent_class)->finalize (object);
}

static void
gst_3d_mesh_class_init (Gst3DMeshClass * klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = gst_3d_mesh_finalize;
}


gboolean gst_3d_mesh_has_buffers (Gst3DMesh * self) {
  if (self->vbo_positions)
    return TRUE;
  return FALSE;
}

void
gst_3d_mesh_init_buffers (Gst3DMesh * self) {
    g_return_if_fail (self != NULL);
    g_return_if_fail (GST_IS_GL_CONTEXT (self->context));

    GstGLFuncs *gl = self->context->gl_vtable;


    gl->GenVertexArrays (1, &self->vao);
    gl->BindVertexArray (self->vao);

    gl->GenBuffers (1, &self->vbo_positions);
    gl->GenBuffers (1, &self->vbo_uv);
    gl->GenBuffers (1, &self->vbo_indices);
    
    GST_DEBUG("generating mesh. vao: %d pos: %d uv: %d index: %d", self->vao, self->vbo_positions, self->vbo_uv, self->vbo_indices);
}

void
gst_3d_mesh_bind (Gst3DMesh * self) {
    GstGLFuncs *gl = self->context->gl_vtable;
    gl->BindVertexArray (self->vao);
}

void
gst_3d_mesh_bind_buffers (Gst3DMesh * self, GLint attr_position, GLint attr_uv)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);

  /* Load the vertex positions */
  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_positions);
  gl->VertexAttribPointer (attr_position,
      self->vector_length,
      GL_FLOAT, GL_FALSE, self->vector_length * sizeof (GLfloat), 0);

  /* Load the texture coordinates */
  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_uv);
  gl->VertexAttribPointer (attr_uv,
      2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
}

void
gst_3d_mesh_unbind_buffers (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  gl->BindBuffer (GL_ARRAY_BUFFER, 0);
}


void
gst_3d_mesh_upload_sphere (Gst3DMesh * self, float radius, unsigned stacks, unsigned slices)
{
  GLfloat *vertices;
  GLfloat *texcoords;
  GLushort *indices;
  
  GstGLFuncs *gl = self->context->gl_vtable;

  self->vector_length = 3;
  int vertex_count = (slices + 1) * stacks;
  const int component_size = sizeof (GLfloat) * vertex_count;

  vertices = (GLfloat *) malloc (component_size * 3);
  texcoords = (GLfloat *) malloc (component_size * 2);

  GLfloat *v = vertices;
  GLfloat *t = texcoords;

  float const J = 1. / (float) (stacks - 1);
  float const I = 1. / (float) (slices - 1);


  for (int i = 0; i < slices + 1; i++) {
    float const theta = M_PI * i * I;
    for (int j = 0; j < stacks; j++) {
      float const phi = 2 * M_PI * j * J;

      float const x = sin (theta) * cos (phi);
      float const y = -cos (theta);
      float const z = sin (phi) * sin (theta);

      *v++ = x * radius;
      *v++ = y * radius;
      *v++ = z * radius;

/*
      float hacky = j * J + 0.75;
      if (hacky > 1)
        hacky -= 1.0;
      *t++ = hacky;
*/
      *t++ = j * J;
      *t++ = i * I;
    }
  }

  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_positions);
  gl->BufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * vertex_count * 3,
      vertices, GL_STATIC_DRAW);

  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_uv);
  gl->BufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * vertex_count * 2,
      texcoords, GL_STATIC_DRAW);

  int vau = 0;
  self->index_size = slices * stacks * 2 * 3;
  indices = (GLushort *) malloc (sizeof (GLushort) * self->index_size);

  GLushort *indextemp = indices;
  for (int i = 0; i < slices; i++) {
    for (int j = 0; j < stacks; j++) {
      int next = (j + 1) % stacks;
      *indextemp++ = vau + j;
      *indextemp++ = vau + next;
      *indextemp++ = vau + j + stacks;

      *indextemp++ = vau + next;
      *indextemp++ = vau + next + stacks;
      *indextemp++ = vau + j + stacks;
    }
    vau += stacks;
  }

  // upload index
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLushort) * self->index_size,
      indices, GL_STATIC_DRAW);

  self->draw_mode = GL_TRIANGLES;
}

void
gst_3d_mesh_draw (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  gl->DrawElements (self->draw_mode, self->index_size, GL_UNSIGNED_SHORT, 0);
}

void
gst_3d_mesh_upload_plane (Gst3DMesh * self, float aspect)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  /* *INDENT-OFF* */
  GLfloat vertices[] = {
     -aspect,  1.0,  0.0, 1.0,
      aspect,  1.0,  0.0, 1.0,
      aspect, -1.0,  0.0, 1.0,
     -aspect, -1.0,  0.0, 1.0
  };
  GLfloat uvs[] = {
     0.0, 1.0,
     1.0, 1.0,
     1.0, 0.0,
     0.0, 0.0
  };
  /* *INDENT-ON* */
  const GLushort indices[] = { 0, 1, 2, 3, 0 };

    int vertex_count = 4;
  self->vector_length = 4;

  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_positions);
  gl->BufferData (GL_ARRAY_BUFFER,
      vertex_count * 4 * sizeof (GLfloat), vertices,
      GL_STATIC_DRAW);

  // index
  self->index_size = sizeof (indices);

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, self->index_size, indices,
      GL_STATIC_DRAW);

  // load uv coords
  gl->BindBuffer (GL_ARRAY_BUFFER, self->vbo_uv);
  gl->BufferData (GL_ARRAY_BUFFER,
      vertex_count * 2 * sizeof (GLfloat), uvs, GL_STATIC_DRAW);

  self->draw_mode = GL_TRIANGLE_STRIP;
}
