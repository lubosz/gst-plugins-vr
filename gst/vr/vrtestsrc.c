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

static void
_unbind_buffer (struct SrcShader *src)
{
  GstGLContext *context = src->base.context;
  const GstGLFuncs *gl = context->gl_vtable;
  gint i;

  gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  gl->BindBuffer (GL_ARRAY_BUFFER, 0);

  for (i = 0; i < src->n_attributes; i++) {
    struct attribute *attr = &src->attributes[i];

    gl->DisableVertexAttribArray (attr->location);
  }
}

static gboolean
_src_shader_init (gpointer impl, GstGLContext * context, GstVideoInfo * v_info)
{
  struct SrcShader *src = impl;
  const GstGLFuncs *gl = context->gl_vtable;

  src->base.context = context;

  if (!src->vbo) {
    if (gl->GenVertexArrays) {
      gl->GenVertexArrays (1, &src->vao);
      gl->BindVertexArray (src->vao);
    }

    gl->GenBuffers (1, &src->vbo);
    gl->BindBuffer (GL_ARRAY_BUFFER, src->vbo);
    gl->BufferData (GL_ARRAY_BUFFER, src->vertices_size,
        src->vertices, GL_STATIC_DRAW);

    gl->GenBuffers (1, &src->vbo_indices);
    gl->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, src->vbo_indices);
    gl->BufferData (GL_ELEMENT_ARRAY_BUFFER, src->n_indices * sizeof (gushort),
        src->indices, GL_STATIC_DRAW);

    if (gl->GenVertexArrays) {
      _bind_buffer (src);
      gl->BindVertexArray (0);
    }

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

  if (gl->GenVertexArrays)
    gl->BindVertexArray (src->vao);
  else
    _bind_buffer (src);

  gl->DrawElements (GL_TRIANGLES, src->n_indices, GL_UNSIGNED_SHORT,
      (gpointer) (gintptr) src->index_offset);

  if (gl->GenVertexArrays)
    gl->BindVertexArray (0);
  else
    _unbind_buffer (src);

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


/* *INDENT-OFF* */
static const gchar *mandelbrot_vertex_src = "attribute vec4 position;\n"
    "uniform float aspect_ratio;\n"
    "varying vec2 fractal_position;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = position;\n"
    "  fractal_position = vec2(position.y * 0.5 - 0.3, aspect_ratio * position.x * 0.5);\n"
    "  fractal_position *= 2.5;\n"
    "}";

static const gchar *mandelbrot_fragment_src = 
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "uniform float time;\n"
    "varying vec2 fractal_position;\n"
    "const vec4 K = vec4(1.0, 0.66, 0.33, 3.0);\n"
    "vec4 hsv_to_rgb(float hue, float saturation, float value) {\n"
    "  vec4 p = abs(fract(vec4(hue) + K) * 6.0 - K.wwww);\n"
    "  return value * mix(K.xxxx, clamp(p - K.xxxx, 0.0, 1.0), saturation);\n"
    "}\n"
    "vec4 i_to_rgb(int i) {\n"
    "  float hue = float(i) / 100.0 + sin(time);\n"
    "  return hsv_to_rgb(hue, 0.5, 0.8);\n"
    "}\n"
    "vec2 pow_2_complex(vec2 c) {\n"
    "  return vec2(c.x*c.x - c.y*c.y, 2.0 * c.x * c.y);\n"
    "}\n"
    "vec2 mandelbrot(vec2 c, vec2 c0) {\n"
    "  return pow_2_complex(c) + c0;\n"
    "}\n"
    "vec4 iterate_pixel(vec2 position) {\n"
    "  vec2 c = vec2(0);\n"
    "  for (int i=0; i < 20; i++) {\n"
    "    if (c.x*c.x + c.y*c.y > 2.0*2.0)\n"
    "      return i_to_rgb(i);\n"
    "    c = mandelbrot(c, position);\n"
    "  }\n"
    "  return vec4(0, 0, 0, 1);\n"
    "}\n"
    "void main() {\n"
    "  gl_FragColor = iterate_pixel(fractal_position);\n"
    "}";
/* *INDENT-ON* */

static gboolean
_src_mandelbrot_init (gpointer impl, GstGLContext * context,
    GstVideoInfo * v_info)
{
  struct SrcShader *src = impl;
  GError *error = NULL;

  src->base.context = context;

  if (src->shader)
    gst_object_unref (src->shader);
  src->shader = gst_gl_shader_new_link_with_stages (context, &error,
      gst_glsl_stage_new_with_string (context, GL_VERTEX_SHADER,
          GST_GLSL_VERSION_NONE,
          GST_GLSL_PROFILE_ES | GST_GLSL_PROFILE_COMPATIBILITY,
          mandelbrot_vertex_src),
      gst_glsl_stage_new_with_string (context, GL_FRAGMENT_SHADER,
          GST_GLSL_VERSION_NONE,
          GST_GLSL_PROFILE_ES | GST_GLSL_PROFILE_COMPATIBILITY,
          mandelbrot_fragment_src), NULL);
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
