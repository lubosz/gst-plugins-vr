/* GStreamer
 * Copyright (C) <2003> David A. Schleef <ds@schleef.org>
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

#ifndef __VR_TEST_SRC_H__
#define __VR_TEST_SRC_H__

#include <glib.h>

typedef struct _GstVRTestSrc GstVRTestSrc;

typedef enum {
    GST_VR_TEST_SRC_MANDELBROT
} GstVRTestSrcPattern;

#include "gstvrtestsrc.h"

struct BaseSrcImpl {
  GstVRTestSrc *src;
  GstGLContext *context;
  GstVideoInfo v_info;
};

struct SrcFuncs
{
  GstVRTestSrcPattern pattern;
  gpointer (*create) (GstVRTestSrc * src);
  gboolean (*init) (gpointer impl, GstGLContext * context, GstVideoInfo * v_info);
  gboolean (*fill_bound_fbo) (gpointer impl);
  void (*free) (gpointer impl);
};

const struct SrcFuncs * gst_vr_test_src_get_src_funcs_for_pattern (GstVRTestSrcPattern pattern);

#endif
