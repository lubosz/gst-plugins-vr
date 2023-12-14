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
#include <gst/gl/gstglfuncs.h>

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
  self->vbo_indices = 0;
  self->index_type = GL_UNSIGNED_SHORT;
}

Gst3DMesh *
gst_3d_mesh_new (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = g_object_new (GST_3D_TYPE_MESH, NULL);
  mesh->context = gst_object_ref (context);
  return mesh;
}

Gst3DMesh *
gst_3d_mesh_new_sphere (GstGLContext * context, float radius, unsigned stacks,
    unsigned slices)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_sphere (mesh, radius, stacks, slices);
  return mesh;
}

Gst3DMesh *
gst_3d_mesh_new_plane (GstGLContext * context, float aspect)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_plane (mesh, aspect);
  return mesh;
}


Gst3DMesh *
gst_3d_mesh_new_point_plane (GstGLContext * context, unsigned width,
    unsigned height)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_point_plane (mesh, width, height);
  return mesh;
}

Gst3DMesh *
gst_3d_mesh_new_line (GstGLContext * context, graphene_vec3_t * from,
    graphene_vec3_t * to, graphene_vec3_t * color)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_line (mesh, from, to, color);
  return mesh;
}

Gst3DMesh *
gst_3d_mesh_new_cube (GstGLContext * context)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_cube (mesh);
  return mesh;
}

Gst3DMesh *
gst_3d_mesh_new_assimp (GstGLContext * context, const char *file)
{
  g_return_val_if_fail (GST_IS_GL_CONTEXT (context), NULL);
  Gst3DMesh *mesh = gst_3d_mesh_new (context);
  gst_3d_mesh_init_buffers (mesh);
  gst_3d_mesh_upload_assimp (mesh, file);
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

  GList *l;
  for (l = self->attribute_buffers; l != NULL; l = l->next) {
    struct Gst3DAttributeBuffer *buf = (struct Gst3DAttributeBuffer *) l->data;
    gl->DeleteBuffers (1, &buf->location);
    g_free (buf);
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

void
gst_3d_mesh_init_buffers (Gst3DMesh * self)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (GST_IS_GL_CONTEXT (self->context));

  GstGLFuncs *gl = self->context->gl_vtable;

  gl->GenVertexArrays (1, &self->vao);
  gl->BindVertexArray (self->vao);
  gl->GenBuffers (1, &self->vbo_indices);
}

void
gst_3d_mesh_bind (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  gl->BindVertexArray (self->vao);
}

void
gst_3d_mesh_bind_shader (Gst3DMesh * self, Gst3DShader * shader)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  gst_3d_shader_bind (shader);

  GList *l;
  for (l = self->attribute_buffers; l != NULL; l = l->next) {
    struct Gst3DAttributeBuffer *buf = (struct Gst3DAttributeBuffer *) l->data;
    GST_DEBUG ("%s: location: %d length: %d size: %zu", buf->name,
        buf->location, buf->vector_length, buf->element_size);

    gl->BindBuffer (GL_ARRAY_BUFFER, buf->location);

    GLint attrib_location =
        gst_gl_shader_get_attribute_location (shader->shader, buf->name);

    if (attrib_location != -1) {
      gl->VertexAttribPointer (attrib_location, buf->vector_length, GL_FLOAT,
          GL_FALSE, 0, 0);
      gl->EnableVertexAttribArray (attrib_location);
    } else {
      GST_WARNING ("could not find attribute %s in shader.", buf->name);
    }
  }
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
}

void
gst_3d_mesh_unbind_buffers (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  gl->BindBuffer (GL_ARRAY_BUFFER, 0);
}

void
gst_3d_mesh_draw (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  gl->DrawElements (self->draw_mode, self->index_size, self->index_type, 0);
}

void
gst_3d_mesh_draw_mode (Gst3DMesh * self, GLenum draw_mode)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  gl->DrawElements (draw_mode, self->index_size, self->index_type, 0);
}

void
gst_3d_mesh_draw_arrays (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;
  gl->DrawArrays (self->draw_mode, 0, self->index_size);
}

void
gst_3d_mesh_append_attribute_buffer (Gst3DMesh * self, const gchar * name,
    size_t element_size, guint vector_length, GLfloat * vertices)
{
  struct Gst3DAttributeBuffer *attrib_buffer =
      malloc (sizeof (struct Gst3DAttributeBuffer));

  GstGLFuncs *gl = self->context->gl_vtable;

  attrib_buffer->name = name;
  attrib_buffer->element_size = element_size;
  attrib_buffer->vector_length = vector_length;

  gl->GenBuffers (1, &attrib_buffer->location);

  GST_DEBUG ("generated %s buffer #%d", attrib_buffer->name,
      attrib_buffer->location);

  gl->BindBuffer (GL_ARRAY_BUFFER, attrib_buffer->location);
  gl->BufferData (GL_ARRAY_BUFFER,
      self->vertex_count * attrib_buffer->vector_length *
      attrib_buffer->element_size, vertices, GL_STATIC_DRAW);

  self->attribute_buffers =
      g_list_append (self->attribute_buffers, attrib_buffer);
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

  self->vertex_count = 4;
  self->draw_mode = GL_TRIANGLE_STRIP;

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 4,
      vertices);
  gst_3d_mesh_append_attribute_buffer (self, "uv", sizeof (GLfloat), 2, uvs);

  // index
  self->index_size = sizeof (indices);  // /sizeof(indices[0])
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, self->index_size, indices,
      GL_STATIC_DRAW);
}

void
gst_3d_mesh_upload_cube (Gst3DMesh * self)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  /* *INDENT-OFF* */
  GLfloat positions[] = {
      /* front face */
       1.0,  1.0, -1.0,
       1.0, -1.0, -1.0,
      -1.0, -1.0, -1.0,
      -1.0,  1.0, -1.0,
      /* back face */
       1.0,  1.0,  1.0,
      -1.0,  1.0,  1.0,
      -1.0, -1.0,  1.0,
       1.0, -1.0,  1.0,
      /* right face */
       1.0,  1.0,  1.0,
       1.0, -1.0,  1.0,
       1.0, -1.0, -1.0,
       1.0,  1.0, -1.0,
      /* left face */
      -1.0,  1.0,  1.0,
      -1.0,  1.0, -1.0,
      -1.0, -1.0, -1.0,
      -1.0, -1.0,  1.0,
      /* top face */
       1.0, -1.0,  1.0,
      -1.0, -1.0,  1.0,
      -1.0, -1.0, -1.0,
       1.0, -1.0, -1.0,
      /* bottom face */
       1.0,  1.0,  1.0,
       1.0,  1.0, -1.0,
      -1.0,  1.0, -1.0,
      -1.0,  1.0,  1.0
  };

  /* *INDENT-OFF* */
  GLfloat uvs[] = {
      /* front face */
       1.0, 0.0,
       1.0, 1.0,
       0.0, 1.0,
       0.0, 0.0,
      /* back face */
       1.0, 0.0,
       0.0, 0.0,
       0.0, 1.0,
       1.0, 1.0,
      /* right face */
       1.0, 0.0,
       0.0, 0.0,
       0.0, 1.0,
       1.0, 1.0,
      /* left face */
       1.0, 0.0,
       1.0, 1.0,
       0.0, 1.0,
       0.0, 0.0,
      /* top face */
       1.0, 0.0,
       0.0, 0.0,
       0.0, 1.0,
       1.0, 1.0,
      /* bottom face */
       1.0, 0.0,
       1.0, 1.0,
       0.0, 1.0,
       0.0, 0.0
  };

  GLushort indices[] = {
      0, 1, 2,
      0, 2, 3,
      4, 5, 6,
      4, 6, 7,
      8, 9, 10,
      8, 10, 11,
      12, 13, 14,
      12, 14, 15,
      16, 17, 18,
      16, 18, 19,
      20, 21, 22,
      20, 22, 23
  };
  /* *INDENT-ON* */

  self->vertex_count = 4 * 6;
  self->draw_mode = GL_TRIANGLES;

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 3,
      positions);
  gst_3d_mesh_append_attribute_buffer (self, "uv", sizeof (GLfloat), 2, uvs);

  // index
  self->index_size = sizeof (indices);
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, self->index_size, indices,
      GL_STATIC_DRAW);
}


void
gst_3d_mesh_upload_line (Gst3DMesh * self, graphene_vec3_t * from,
    graphene_vec3_t * to, graphene_vec3_t * color)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  GLfloat vertices[] = {
    graphene_vec3_get_x (from), graphene_vec3_get_y (from),
    graphene_vec3_get_z (from), 1.0,
    graphene_vec3_get_x (to), graphene_vec3_get_y (to),
    graphene_vec3_get_z (to), 1.0
  };
  GLfloat colors[] = {
    graphene_vec3_get_x (color), graphene_vec3_get_y (color),
    graphene_vec3_get_z (color),
    graphene_vec3_get_x (color), graphene_vec3_get_y (color),
    graphene_vec3_get_z (color)
  };

  self->vertex_count = 2;
  self->draw_mode = GL_LINES;

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 4,
      vertices);
  gst_3d_mesh_append_attribute_buffer (self, "color", sizeof (GLfloat), 3,
      colors);

  const GLushort indices[] = { 0, 1 };

  // index
  self->index_size = sizeof (indices);
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, self->index_size, indices,
      GL_STATIC_DRAW);

}


void
gst_3d_mesh_upload_point_plane (Gst3DMesh * self, unsigned width,
    unsigned height)
{
  GLfloat *vertices;
  // GLfloat *texcoords;
  GLuint *indices;

  GstGLFuncs *gl = self->context->gl_vtable;

  self->vertex_count = width * height;
  const int component_size = sizeof (GLfloat) * self->vertex_count;

  vertices = (GLfloat *) malloc (component_size * 3);
  // texcoords = (GLfloat *) malloc (component_size * 2);

  GLfloat *v = vertices;
  // GLfloat *t = texcoords;

  float w_step = 2.0 / (float) width;
  float h_step = 2.0 / (float) height;

  float curent_w = -1.0;
/*
  float u_step = 1.0 / (float) width;
  float v_step = 1.0 / (float) height;

  float curent_u = 0.0;
*/

  for (int i = 0; i < width; i++) {
    float curent_h = -1.0;
    // float curent_v = 0.0;
    for (int j = 0; j < height; j++) {

      *v++ = curent_w;
      *v++ = curent_h;
      *v++ = 0;

      curent_h += h_step;
      /*
       *t++ = curent_u;
       *t++ = curent_v;
       curent_v += v_step;
       */
    }
    // curent_u += u_step;
    curent_w += w_step;
  }

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 3,
      vertices);

  // linear index. TODO: do not use index at all here.
  self->index_size = self->vertex_count;
  indices = (GLuint *) malloc (sizeof (GLuint) * self->index_size);
  GLuint *indextemp = indices;
  for (int i = 0; i < self->index_size; i++) {
    *indextemp++ = i;
  }

  // upload index
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint) * self->index_size,
      indices, GL_STATIC_DRAW);
  self->draw_mode = GL_POINTS;
}

void
gst_3d_mesh_upload_sphere (Gst3DMesh * self, float radius, unsigned stacks,
    unsigned slices)
{
  GLfloat *positions;
  GLfloat *uvs;
  GLushort *indices;

  GstGLFuncs *gl = self->context->gl_vtable;

  self->vertex_count = (slices + 1) * stacks;
  const int component_size = sizeof (GLfloat) * self->vertex_count;

  positions = (GLfloat *) malloc (component_size * 3);
  uvs = (GLfloat *) malloc (component_size * 2);

  GLfloat *v = positions;
  GLfloat *t = uvs;

  float const J = 1. / (float) (stacks - 1);
  float const I = 1. / (float) (slices - 1);

  for (int i = 0; i < slices; i++) {
    float const theta = M_PI * i * I;
    for (int j = 0; j < stacks; j++) {
      float const phi = 2 * M_PI * j * J + M_PI / 2.0;

      float const x = sin (theta) * cos (phi);
      float const y = -cos (theta);
      float const z = sin (phi) * sin (theta);

      *v++ = x * radius;
      *v++ = y * radius;
      *v++ = z * radius;

      *t++ = j * J;
      *t++ = i * I;
    }
  }

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 3,
      positions);
  gst_3d_mesh_append_attribute_buffer (self, "uv", sizeof (GLfloat), 2, uvs);

  /* index */
  self->index_size = (slices - 1) * stacks * 2;

  indices = (GLushort *) malloc (sizeof (GLushort) * self->index_size);
  GLushort *indextemp = indices;

  // -3 = minus caps slices - one to iterate over strips
  for (int i = 0; i < slices - 1; i++) {
    for (int j = 0; j < stacks; j++) {
      *indextemp++ = i * stacks + j;
      *indextemp++ = (i + 1) * stacks + j;
    }
  }

  /* linear index */
  /*
     self->index_size = (slices - 2) * stacks;
     for (int i = 0; i < self->index_size; i++)
     *indextemp++ = i;
   */

  // upload index
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLushort) * self->index_size,
      indices, GL_STATIC_DRAW);

  self->draw_mode = GL_TRIANGLE_STRIP;
}

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void
gst_3d_mesh_upload_assimp (Gst3DMesh * self, const char *file)
{
  GstGLFuncs *gl = self->context->gl_vtable;

  const struct aiScene *scene = NULL;
  scene = aiImportFile (file, 0);


  const struct aiMesh *assimp_mesh =
      scene->mMeshes[scene->mRootNode->mChildren[0]->mMeshes[0]];

  GST_ERROR ("Uploading %s with %d verts", file, assimp_mesh->mNumVertices);

  // GLfloat *normals = malloc(3 * mesh->mNumVertices *sizeof(float));

  GLfloat *positions =
      malloc (3 * assimp_mesh->mNumVertices * sizeof (GLfloat));
  for (int i = 0; i < assimp_mesh->mNumVertices; ++i) {
    positions[i * 3] = assimp_mesh->mVertices[i].x;
    positions[i * 3 + 1] = assimp_mesh->mVertices[i].y;
    positions[i * 3 + 2] = assimp_mesh->mVertices[i].z;
  }

  GLfloat *uvs = malloc (2 * assimp_mesh->mNumVertices * sizeof (GLfloat));
  for (int i = 0; i < assimp_mesh->mNumVertices; ++i) {
    uvs[i * 2] = assimp_mesh->mTextureCoords[0][i].x;
    uvs[i * 2 + 1] = assimp_mesh->mTextureCoords[0][i].y;
  }

  GLushort *indices = malloc (3 * assimp_mesh->mNumFaces * sizeof (GLushort));

  for (int i = 0; i < assimp_mesh->mNumFaces; ++i) {
    indices[i * 3] = assimp_mesh->mFaces[i].mIndices[0];
    indices[i * 3 + 1] = assimp_mesh->mFaces[i].mIndices[1];
    indices[i * 3 + 2] = assimp_mesh->mFaces[i].mIndices[2];
  }

  self->vertex_count = assimp_mesh->mNumVertices;
  self->draw_mode = GL_TRIANGLES;
  //mesh->draw_mode = GL_LINE_STRIP;

  gst_3d_mesh_append_attribute_buffer (self, "position", sizeof (GLfloat), 3,
      positions);
  gst_3d_mesh_append_attribute_buffer (self, "uv", sizeof (GLfloat), 2, uvs);

  // index
  self->index_size = 3 * assimp_mesh->mNumFaces * sizeof (GLushort);
  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, self->vbo_indices);
  gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, self->index_size, indices,
      GL_STATIC_DRAW);
}
