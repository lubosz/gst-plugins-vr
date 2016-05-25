/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2016> Matthew Waters <matthew@centricular.com>
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

#include "vrtestsrc.h"

#include "../../gst-libs/gst/3d/gst3dshader.h"

#define MAX_ATTRIBUTES 4

/* *INDENT-OFF* */
static const GLfloat positions[] = {
     -1.0,  1.0,  0.0, 1.0,
      1.0,  1.0,  0.0, 1.0,
      1.0, -1.0,  0.0, 1.0,
     -1.0, -1.0,  0.0, 1.0,
};

static const GLushort indices_quad[] = { 0, 1, 2, 0, 2, 3 };
/* *INDENT-ON* */

struct attribute
{
  const gchar *name;
  gint location;
  guint n_elements;
  GLenum element_type;
  guint offset;                 /* in bytes */
  guint stride;                 /* in bytes */
};

struct SrcShader
{
  struct BaseSrcImpl base;

  GstGLShader *shader;

  guint vao;
  guint vbo;
  guint vbo_indices;

  guint n_attributes;
  struct attribute attributes[MAX_ATTRIBUTES];

  gconstpointer vertices;
  gsize vertices_size;
  const gushort *indices;
  guint index_offset;
  guint n_indices;
};

static void
_bind_buffer (struct SrcShader *src)
{
  GstGLContext *context = src->base.context;
  const GstGLFuncs *gl = context->gl_vtable;
  gint i;

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, src->vbo_indices);
  gl->BindBuffer (GL_ARRAY_BUFFER, src->vbo);

  /* Load the vertex position */
  for (i = 0; i < src->n_attributes; i++) {
    struct attribute *attr = &src->attributes[i];

    if (attr->location == -1)
      attr->location =
          gst_gl_shader_get_attribute_location (src->shader, attr->name);

    gl->VertexAttribPointer (attr->location, attr->n_elements,
        attr->element_type, GL_FALSE, attr->stride,
        (void *) (gintptr) attr->offset);

    gl->EnableVertexAttribArray (attr->location);
  }
}

static gboolean
_src_shader_init (gpointer impl, GstGLContext * context, GstVideoInfo * v_info)
{
  struct SrcShader *src = impl;
  const GstGLFuncs *gl = context->gl_vtable;

  src->base.context = context;

  if (!src->vbo) {
    gl->GenVertexArrays (1, &src->vao);
    gl->BindVertexArray (src->vao);

    gl->GenBuffers (1, &src->vbo);
    gl->BindBuffer (GL_ARRAY_BUFFER, src->vbo);
    gl->BufferData (GL_ARRAY_BUFFER, src->vertices_size,
        src->vertices, GL_STATIC_DRAW);

    gl->GenBuffers (1, &src->vbo_indices);
    gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, src->vbo_indices);
    gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, src->n_indices * sizeof (gushort),
        src->indices, GL_STATIC_DRAW);

    _bind_buffer (src);
    gl->BindVertexArray (0);

    gl->BindBuffer (GL_ARRAY_BUFFER, 0);
    gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  return TRUE;
}

static gboolean
_src_shader_fill_bound_fbo (gpointer impl)
{
  struct SrcShader *src = impl;
  const GstGLFuncs *gl;

  g_return_val_if_fail (src->base.context, FALSE);
  g_return_val_if_fail (src->shader, FALSE);
  gl = src->base.context->gl_vtable;

  gst_gl_shader_use (src->shader);

  gl->BindVertexArray (src->vao);

  gl->DrawElements (GL_TRIANGLES, src->n_indices, GL_UNSIGNED_SHORT,
      (gpointer) (gintptr) src->index_offset);

  gl->BindVertexArray (0);

  gst_gl_context_clear_shader (src->base.context);

  return TRUE;
}

static void
_src_shader_deinit (gpointer impl)
{
  struct SrcShader *src = impl;
  const GstGLFuncs *gl = src->base.context->gl_vtable;

  if (src->shader)
    gst_object_unref (src->shader);
  src->shader = NULL;

  if (src->vao)
    gl->DeleteVertexArrays (1, &src->vao);
  src->vao = 0;

  if (src->vbo)
    gl->DeleteBuffers (1, &src->vbo);
  src->vbo = 0;

  if (src->vbo_indices)
    gl->DeleteBuffers (1, &src->vbo_indices);
  src->vbo_indices = 0;
}

static gboolean
_src_mandelbrot_init (gpointer impl, GstGLContext * context,
    GstVideoInfo * v_info)
{
  struct SrcShader *src = impl;
  GError *error = NULL;

  src->base.context = context;

  if (src->shader)
    gst_object_unref (src->shader);


  Gst3DShader *a3dshader =
      gst_3d_shader_new_vert_frag (context, "mandelbrot.vert",
      "mandelbrot.frag");
  src->shader = a3dshader->shader;

  if (!src->shader) {
    GST_ERROR_OBJECT (src->base.src, "%s", error->message);
    return FALSE;
  }

  src->n_attributes = 1;

  src->attributes[0].name = "position";
  src->attributes[0].location = -1;
  src->attributes[0].n_elements = 4;
  src->attributes[0].element_type = GL_FLOAT;
  src->attributes[0].offset = 0;
  src->attributes[0].stride = 4 * sizeof (gfloat);

  src->vertices = positions;
  src->vertices_size = sizeof (positions);
  src->indices = indices_quad;
  src->n_indices = 6;

  gst_gl_shader_use (src->shader);
  gst_gl_shader_set_uniform_1f (src->shader, "aspect_ratio",
      (gfloat) GST_VIDEO_INFO_WIDTH (v_info) /
      (gfloat) GST_VIDEO_INFO_HEIGHT (v_info));
  gst_gl_context_clear_shader (src->base.context);

  return _src_shader_init (impl, context, v_info);
}

static gboolean
_src_mandelbrot_fill_bound_fbo (gpointer impl)
{
  struct SrcShader *src = impl;

  g_return_val_if_fail (src->base.context, FALSE);
  g_return_val_if_fail (src->shader, FALSE);

  gst_gl_shader_use (src->shader);
  gst_gl_shader_set_uniform_1f (src->shader, "time",
      (gfloat) src->base.src->running_time / GST_SECOND);

  return _src_shader_fill_bound_fbo (impl);
}

static void
_src_mandelbrot_free (gpointer impl)
{
  struct SrcShader *src = impl;

  if (!src)
    return;

  _src_shader_deinit (impl);

  g_free (impl);
}

static gpointer
_src_mandelbrot_new (GstVRTestSrc * test)
{
  struct SrcShader *src = g_new0 (struct SrcShader, 1);

  src->base.src = test;

  return src;
}

static const struct SrcFuncs src_mandelbrot = {
  GST_VR_TEST_SRC_MANDELBROT,
  _src_mandelbrot_new,
  _src_mandelbrot_init,
  _src_mandelbrot_fill_bound_fbo,
  _src_mandelbrot_free,
};


static const struct SrcFuncs *src_impls[] = {
  &src_mandelbrot,
};

const struct SrcFuncs *
gst_vr_test_src_get_src_funcs_for_pattern (GstVRTestSrcPattern pattern)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (src_impls); i++) {
    if (src_impls[i]->pattern == pattern)
      return src_impls[i];
  }

  return NULL;
}
