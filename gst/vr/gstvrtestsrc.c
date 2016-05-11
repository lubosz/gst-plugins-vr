/*
 * GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2002,2007 David A. Schleef <ds@schleef.org>
 * Copyright (C) 2008 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2015 Matthew Waters <matthew@centricular.com>
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

/**
 * SECTION:element-gstvrtestsrc
 *
 * <refsect2>
 * <para>
 * The gstvrtestsrc element is used to produce test video texture.
 * The video test produced can be controlled with the "pattern"
 * property.
 * </para>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch-1.0 -v gstvrtestsrc pattern=smpte ! glimagesink
 * </programlisting>
 * Shows original SMPTE color bars in a window.
 * </para>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstvrtestsrc.h"
#include "gstvrtestsrc.h"
//#include <gst/gst-i18n-plugin.h>

#define USE_PEER_BUFFERALLOC
#define SUPPORTED_GL_APIS (GST_GL_API_OPENGL | GST_GL_API_OPENGL3 | GST_GL_API_GLES2)

GST_DEBUG_CATEGORY_STATIC (vr_test_src_debug);
#define GST_CAT_DEFAULT vr_test_src_debug

enum
{
  PROP_0,
  PROP_PATTERN,
  PROP_TIMESTAMP_OFFSET,
  PROP_IS_LIVE
      /* FILL ME */
};

/* *INDENT-OFF* */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw(" GST_CAPS_FEATURE_MEMORY_GL_MEMORY "), "
        "format = (string) RGBA, "
        "width = " GST_VIDEO_SIZE_RANGE ", "
        "height = " GST_VIDEO_SIZE_RANGE ", "
        "framerate = " GST_VIDEO_FPS_RANGE ","
        "texture-target = (string) 2D")
    );
/* *INDENT-ON* */

#define gst_vr_test_src_parent_class parent_class
G_DEFINE_TYPE (GstVRTestSrc, gst_vr_test_src, GST_TYPE_PUSH_SRC);

static void gst_vr_test_src_set_pattern (GstVRTestSrc * gstvrtestsrc,
    int pattern_type);
static void gst_vr_test_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_vr_test_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_vr_test_src_setcaps (GstBaseSrc * bsrc, GstCaps * caps);
static GstCaps *gst_vr_test_src_fixate (GstBaseSrc * bsrc, GstCaps * caps);

static gboolean gst_vr_test_src_is_seekable (GstBaseSrc * psrc);
static gboolean gst_vr_test_src_do_seek (GstBaseSrc * bsrc,
    GstSegment * segment);
static gboolean gst_vr_test_src_query (GstBaseSrc * bsrc, GstQuery * query);
static void gst_vr_test_src_set_context (GstElement * element,
    GstContext * context);
static GstStateChangeReturn gst_vr_test_src_change_state (GstElement * element,
    GstStateChange transition);

static void gst_vr_test_src_get_times (GstBaseSrc * basesrc,
    GstBuffer * buffer, GstClockTime * start, GstClockTime * end);
static GstFlowReturn gst_vr_test_src_fill (GstPushSrc * psrc,
    GstBuffer * buffer);
static gboolean gst_vr_test_src_start (GstBaseSrc * basesrc);
static gboolean gst_vr_test_src_stop (GstBaseSrc * basesrc);
static gboolean gst_vr_test_src_decide_allocation (GstBaseSrc * basesrc,
    GstQuery * query);

static void gst_vr_test_src_callback (gpointer stuff);

static gboolean gst_vr_test_src_init_shader (GstVRTestSrc * gstvrtestsrc);

#define GST_TYPE_VR_TEST_SRC_PATTERN (gst_vr_test_src_pattern_get_type ())
static GType
gst_vr_test_src_pattern_get_type (void)
{
  static GType vr_test_src_pattern_type = 0;
  static const GEnumValue pattern_types[] = {
    {GST_VR_TEST_SRC_MANDELBROT, "Mandelbrot Fractal", "mandelbrot"},
    {0, NULL, NULL}
  };

  if (!vr_test_src_pattern_type) {
    vr_test_src_pattern_type =
        g_enum_register_static ("GstVRTestSrcPattern", pattern_types);
  }
  return vr_test_src_pattern_type;
}

static void
gst_vr_test_src_class_init (GstVRTestSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpushsrc_class;
  GstElementClass *element_class;

  GST_DEBUG_CATEGORY_INIT (vr_test_src_debug, "gstvrtestsrc", 0,
      "VR Test Source");

  gobject_class = (GObjectClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;
  gstpushsrc_class = (GstPushSrcClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_vr_test_src_set_property;
  gobject_class->get_property = gst_vr_test_src_get_property;

  g_object_class_install_property (gobject_class, PROP_PATTERN,
      g_param_spec_enum ("pattern", "Pattern",
          "Type of test pattern to generate", GST_TYPE_VR_TEST_SRC_PATTERN,
          GST_VR_TEST_SRC_MANDELBROT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class,
      PROP_TIMESTAMP_OFFSET, g_param_spec_int64 ("timestamp-offset",
          "Timestamp offset",
          "An offset added to timestamps set on buffers (in ns)", G_MININT64,
          G_MAXINT64, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_IS_LIVE,
      g_param_spec_boolean ("is-live", "Is Live",
          "Whether to act as a live source", FALSE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_metadata (element_class, "VR test source",
      "Source/Video", "Creates a test video stream",
      "David A. Schleef <ds@schleef.org>");

  gst_element_class_add_static_pad_template (element_class, &src_factory);

  element_class->set_context = gst_vr_test_src_set_context;
  element_class->change_state = gst_vr_test_src_change_state;

  gstbasesrc_class->set_caps = gst_vr_test_src_setcaps;
  gstbasesrc_class->is_seekable = gst_vr_test_src_is_seekable;
  gstbasesrc_class->do_seek = gst_vr_test_src_do_seek;
  gstbasesrc_class->query = gst_vr_test_src_query;
  gstbasesrc_class->get_times = gst_vr_test_src_get_times;
  gstbasesrc_class->start = gst_vr_test_src_start;
  gstbasesrc_class->stop = gst_vr_test_src_stop;
  gstbasesrc_class->fixate = gst_vr_test_src_fixate;
  gstbasesrc_class->decide_allocation = gst_vr_test_src_decide_allocation;

  gstpushsrc_class->fill = gst_vr_test_src_fill;
}

static void
gst_vr_test_src_init (GstVRTestSrc * src)
{
  gst_vr_test_src_set_pattern (src, GST_VR_TEST_SRC_MANDELBROT);

  src->timestamp_offset = 0;

  /* we operate in time */
  gst_base_src_set_format (GST_BASE_SRC (src), GST_FORMAT_TIME);
  gst_base_src_set_live (GST_BASE_SRC (src), FALSE);
}

static GstCaps *
gst_vr_test_src_fixate (GstBaseSrc * bsrc, GstCaps * caps)
{
  GstStructure *structure;

  GST_DEBUG ("fixate");

  caps = gst_caps_make_writable (caps);

  structure = gst_caps_get_structure (caps, 0);

  gst_structure_fixate_field_nearest_int (structure, "width", 1920);
  gst_structure_fixate_field_nearest_int (structure, "height", 1080);
  gst_structure_fixate_field_nearest_fraction (structure, "framerate", 75, 1);

  caps = GST_BASE_SRC_CLASS (parent_class)->fixate (bsrc, caps);

  return caps;
}

static void
gst_vr_test_src_set_pattern (GstVRTestSrc * gstvrtestsrc, gint pattern_type)
{
  gstvrtestsrc->set_pattern = pattern_type;
}

static void
gst_vr_test_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (object);

  switch (prop_id) {
    case PROP_PATTERN:
      gst_vr_test_src_set_pattern (src, g_value_get_enum (value));
      break;
    case PROP_TIMESTAMP_OFFSET:
      src->timestamp_offset = g_value_get_int64 (value);
      break;
    case PROP_IS_LIVE:
      gst_base_src_set_live (GST_BASE_SRC (src), g_value_get_boolean (value));
      break;
    default:
      break;
  }
}

static void
gst_vr_test_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (object);

  switch (prop_id) {
    case PROP_PATTERN:
      g_value_set_enum (value, src->set_pattern);
      break;
    case PROP_TIMESTAMP_OFFSET:
      g_value_set_int64 (value, src->timestamp_offset);
      break;
    case PROP_IS_LIVE:
      g_value_set_boolean (value, gst_base_src_is_live (GST_BASE_SRC (src)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_vr_test_src_setcaps (GstBaseSrc * bsrc, GstCaps * caps)
{
  GstVRTestSrc *gstvrtestsrc = GST_VR_TEST_SRC (bsrc);

  GST_DEBUG ("setcaps");

  if (!gst_video_info_from_caps (&gstvrtestsrc->out_info, caps))
    goto wrong_caps;

  gstvrtestsrc->negotiated = TRUE;

  gst_caps_replace (&gstvrtestsrc->out_caps, caps);

  return TRUE;

/* ERRORS */
wrong_caps:
  {
    GST_WARNING ("wrong caps");
    return FALSE;
  }
}

static void
gst_vr_test_src_set_context (GstElement * element, GstContext * context)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (element);

  gst_gl_handle_set_context (element, context, &src->display,
      &src->other_context);

  if (src->display)
    gst_gl_display_filter_gl_api (src->display, SUPPORTED_GL_APIS);

  GST_ELEMENT_CLASS (parent_class)->set_context (element, context);
}

static gboolean
gst_vr_test_src_query (GstBaseSrc * bsrc, GstQuery * query)
{
  gboolean res = FALSE;
  GstVRTestSrc *src;

  src = GST_VR_TEST_SRC (bsrc);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_CONTEXT:
    {
      const gchar *context_type;
      GstContext *context, *old_context;

      res = gst_gl_handle_context_query ((GstElement *) src, query,
          &src->display, &src->other_context);
      if (src->display)
        gst_gl_display_filter_gl_api (src->display, SUPPORTED_GL_APIS);

      gst_query_parse_context_type (query, &context_type);

      if (g_strcmp0 (context_type, "gst.gl.local_context") == 0) {
        GstStructure *s;

        gst_query_parse_context (query, &old_context);

        if (old_context)
          context = gst_context_copy (old_context);
        else
          context = gst_context_new ("gst.gl.local_context", FALSE);

        s = gst_context_writable_structure (context);
        gst_structure_set (s, "context", GST_GL_TYPE_CONTEXT, src->context,
            NULL);
        gst_query_set_context (query, context);
        gst_context_unref (context);

        res = src->context != NULL;
      }
      GST_LOG_OBJECT (src, "context query of type %s %i", context_type, res);

      if (res)
        return res;

      break;
    }
    case GST_QUERY_CONVERT:
    {
      GstFormat src_fmt, dest_fmt;
      gint64 src_val, dest_val;

      gst_query_parse_convert (query, &src_fmt, &src_val, &dest_fmt, &dest_val);
      res =
          gst_video_info_convert (&src->out_info, src_fmt, src_val, dest_fmt,
          &dest_val);
      gst_query_set_convert (query, src_fmt, src_val, dest_fmt, dest_val);

      return res;
    }
    default:
      break;
  }

  return GST_BASE_SRC_CLASS (parent_class)->query (bsrc, query);
}

static void
gst_vr_test_src_get_times (GstBaseSrc * basesrc, GstBuffer * buffer,
    GstClockTime * start, GstClockTime * end)
{
  /* for live sources, sync on the timestamp of the buffer */
  if (gst_base_src_is_live (basesrc)) {
    GstClockTime timestamp = GST_BUFFER_TIMESTAMP (buffer);

    if (GST_CLOCK_TIME_IS_VALID (timestamp)) {
      /* get duration to calculate end time */
      GstClockTime duration = GST_BUFFER_DURATION (buffer);

      if (GST_CLOCK_TIME_IS_VALID (duration))
        *end = timestamp + duration;
      *start = timestamp;
    }
  } else {
    *start = -1;
    *end = -1;
  }
}

static gboolean
gst_vr_test_src_do_seek (GstBaseSrc * bsrc, GstSegment * segment)
{
  GstClockTime time;
  GstVRTestSrc *src;

  src = GST_VR_TEST_SRC (bsrc);

  segment->time = segment->start;
  time = segment->position;

  /* now move to the time indicated */
  if (src->out_info.fps_n) {
    src->n_frames = gst_util_uint64_scale (time,
        src->out_info.fps_n, src->out_info.fps_d * GST_SECOND);
  } else
    src->n_frames = 0;

  if (src->out_info.fps_n) {
    src->running_time = gst_util_uint64_scale (src->n_frames,
        src->out_info.fps_d * GST_SECOND, src->out_info.fps_n);
  } else {
    /* FIXME : Not sure what to set here */
    src->running_time = 0;
  }

  g_return_val_if_fail (src->running_time <= time, FALSE);

  return TRUE;
}

static gboolean
gst_vr_test_src_is_seekable (GstBaseSrc * psrc)
{
  /* we're seekable... */
  return TRUE;
}

static gboolean
gst_vr_test_src_init_shader (GstVRTestSrc * gstvrtestsrc)
{
  if (gst_gl_context_get_gl_api (gstvrtestsrc->context)) {
    /* blocking call, wait until the opengl thread has compiled the shader */
//    if (gstvrtestsrc->vertex_src == NULL)
//      return FALSE;
//    return gst_gl_context_gen_shader (gstvrtestsrc->context, gstvrtestsrc->vertex_src,
//        gstvrtestsrc->fragment_src, &gstvrtestsrc->shader);
  }
  return TRUE;
}

static GstFlowReturn
gst_vr_test_src_fill (GstPushSrc * psrc, GstBuffer * buffer)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (psrc);
  GstClockTime next_time;
  gint width, height;
  GstVideoFrame out_frame;
  GstGLSyncMeta *sync_meta;
  guint out_tex;

  if (G_UNLIKELY (!src->negotiated || !src->context))
    goto not_negotiated;

  width = GST_VIDEO_INFO_WIDTH (&src->out_info);
  height = GST_VIDEO_INFO_HEIGHT (&src->out_info);

  /* 0 framerate and we are at the second frame, eos */
  if (G_UNLIKELY (GST_VIDEO_INFO_FPS_N (&src->out_info) == 0
          && src->n_frames == 1))
    goto eos;

  if (!gst_video_frame_map (&out_frame, &src->out_info, buffer,
          GST_MAP_WRITE | GST_MAP_GL)) {
    return GST_FLOW_NOT_NEGOTIATED;
  }

  out_tex = *(guint *) out_frame.data[0];

  if (!gst_gl_context_use_fbo_v2 (src->context, width, height, src->fbo,
          src->depthbuffer, out_tex, gst_vr_test_src_callback,
          (gpointer) src)) {
    gst_video_frame_unmap (&out_frame);
    goto gl_error;
  }
  gst_video_frame_unmap (&out_frame);
  if (!src->gl_result)
    goto gl_error;

  sync_meta = gst_buffer_get_gl_sync_meta (buffer);
  if (sync_meta)
    gst_gl_sync_meta_set_sync_point (sync_meta, src->context);

  GST_BUFFER_TIMESTAMP (buffer) = src->timestamp_offset + src->running_time;
  GST_BUFFER_OFFSET (buffer) = src->n_frames;
  src->n_frames++;
  GST_BUFFER_OFFSET_END (buffer) = src->n_frames;
  if (src->out_info.fps_n) {
    next_time = gst_util_uint64_scale_int (src->n_frames * GST_SECOND,
        src->out_info.fps_d, src->out_info.fps_n);
    GST_BUFFER_DURATION (buffer) = next_time - src->running_time;
  } else {
    next_time = src->timestamp_offset;
    /* NONE means forever */
    GST_BUFFER_DURATION (buffer) = GST_CLOCK_TIME_NONE;
  }

  src->running_time = next_time;

  return GST_FLOW_OK;

gl_error:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, NOT_FOUND, ("failed to draw pattern"),
        ("A GL error occured"));
    return GST_FLOW_NOT_NEGOTIATED;
  }
not_negotiated:
  {
    GST_ELEMENT_ERROR (src, CORE, NEGOTIATION, (NULL),
        ("format wasn't negotiated before get function"));
    return GST_FLOW_NOT_NEGOTIATED;
  }
eos:
  {
    GST_DEBUG_OBJECT (src, "eos: 0 framerate, frame %d", (gint) src->n_frames);
    return GST_FLOW_EOS;
  }
}

static gboolean
gst_vr_test_src_start (GstBaseSrc * basesrc)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (basesrc);

  if (!gst_gl_ensure_element_data (src, &src->display, &src->other_context))
    return FALSE;

  gst_gl_display_filter_gl_api (src->display, SUPPORTED_GL_APIS);

  src->running_time = 0;
  src->n_frames = 0;
  src->negotiated = FALSE;

  return TRUE;
}

static gboolean
gst_vr_test_src_stop (GstBaseSrc * basesrc)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (basesrc);

  gst_caps_replace (&src->out_caps, NULL);

  if (src->context) {
    if (src->shader) {
      gst_object_unref (src->shader);
      src->shader = NULL;
    }
    //blocking call, delete the FBO
    gst_gl_context_del_fbo (src->context, src->fbo, src->depthbuffer);
    gst_object_unref (src->context);
    src->context = NULL;
  }

  return TRUE;
}

static gboolean
_find_local_gl_context (GstVRTestSrc * src)
{
  GstQuery *query;
  GstContext *context;
  const GstStructure *s;

  if (src->context)
    return TRUE;

  query = gst_query_new_context ("gst.gl.local_context");
  if (!src->context && gst_gl_run_query (GST_ELEMENT (src), query, GST_PAD_SRC)) {
    gst_query_parse_context (query, &context);
    if (context) {
      s = gst_context_get_structure (context);
      gst_structure_get (s, "context", GST_GL_TYPE_CONTEXT, &src->context,
          NULL);
    }
  }
  if (!src->context
      && gst_gl_run_query (GST_ELEMENT (src), query, GST_PAD_SINK)) {
    gst_query_parse_context (query, &context);
    if (context) {
      s = gst_context_get_structure (context);
      gst_structure_get (s, "context", GST_GL_TYPE_CONTEXT, &src->context,
          NULL);
    }
  }

  GST_DEBUG_OBJECT (src, "found local context %p", src->context);

  gst_query_unref (query);

  if (src->context)
    return TRUE;

  return FALSE;
}

static gboolean
gst_vr_test_src_decide_allocation (GstBaseSrc * basesrc, GstQuery * query)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (basesrc);
  GstBufferPool *pool = NULL;
  GstStructure *config;
  GstCaps *caps;
  guint min, max, size;
  gboolean update_pool;
  GError *error = NULL;
  guint out_width, out_height;

  if (!gst_gl_ensure_element_data (src, &src->display, &src->other_context))
    return FALSE;

  gst_gl_display_filter_gl_api (src->display, SUPPORTED_GL_APIS);

  _find_local_gl_context (src);

  if (!src->context) {
    GST_OBJECT_LOCK (src->display);
    do {
      if (src->context) {
        gst_object_unref (src->context);
        src->context = NULL;
      }
      /* just get a GL context.  we don't care */
      src->context =
          gst_gl_display_get_gl_context_for_thread (src->display, NULL);
      if (!src->context) {
        if (!gst_gl_display_create_context (src->display, src->other_context,
                &src->context, &error)) {
          GST_OBJECT_UNLOCK (src->display);
          goto context_error;
        }
      }
    } while (!gst_gl_display_add_context (src->display, src->context));
    GST_OBJECT_UNLOCK (src->display);
  }

  if ((gst_gl_context_get_gl_api (src->context) & SUPPORTED_GL_APIS) == 0)
    goto unsupported_gl_api;

  out_width = GST_VIDEO_INFO_WIDTH (&src->out_info);
  out_height = GST_VIDEO_INFO_HEIGHT (&src->out_info);

  if (!gst_gl_context_gen_fbo (src->context, out_width, out_height,
          &src->fbo, &src->depthbuffer))
    goto context_error;

  gst_query_parse_allocation (query, &caps, NULL);

  if (gst_query_get_n_allocation_pools (query) > 0) {
    gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

    update_pool = TRUE;
  } else {
    GstVideoInfo vinfo;

    gst_video_info_init (&vinfo);
    gst_video_info_from_caps (&vinfo, caps);
    size = vinfo.size;
    min = max = 0;
    update_pool = FALSE;
  }

  if (!pool || !GST_IS_GL_BUFFER_POOL (pool)) {
    /* can't use this pool */
    if (pool)
      gst_object_unref (pool);
    pool = gst_gl_buffer_pool_new (src->context);
  }
  config = gst_buffer_pool_get_config (pool);

  gst_buffer_pool_config_set_params (config, caps, size, min, max);
  gst_buffer_pool_config_add_option (config, GST_BUFFER_POOL_OPTION_VIDEO_META);
  if (gst_query_find_allocation_meta (query, GST_GL_SYNC_META_API_TYPE, NULL))
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_GL_SYNC_META);
  gst_buffer_pool_config_add_option (config,
      GST_BUFFER_POOL_OPTION_VIDEO_GL_TEXTURE_UPLOAD_META);

  gst_buffer_pool_set_config (pool, config);

  if (update_pool)
    gst_query_set_nth_allocation_pool (query, 0, pool, size, min, max);
  else
    gst_query_add_allocation_pool (query, pool, size, min, max);

  gst_vr_test_src_init_shader (src);

  gst_object_unref (pool);

  return TRUE;

unsupported_gl_api:
  {
    GstGLAPI gl_api = gst_gl_context_get_gl_api (src->context);
    gchar *gl_api_str = gst_gl_api_to_string (gl_api);
    gchar *supported_gl_api_str = gst_gl_api_to_string (SUPPORTED_GL_APIS);
    GST_ELEMENT_ERROR (src, RESOURCE, BUSY,
        ("GL API's not compatible context: %s supported: %s", gl_api_str,
            supported_gl_api_str), (NULL));

    g_free (supported_gl_api_str);
    g_free (gl_api_str);
    return FALSE;
  }
context_error:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, NOT_FOUND, ("%s", error->message),
        (NULL));
    g_clear_error (&error);
    if (src->context)
      gst_object_unref (src->context);
    src->context = NULL;
    return FALSE;
  }
}

//opengl scene
static void
gst_vr_test_src_callback (gpointer stuff)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (stuff);
  const struct SrcFuncs *funcs;

  funcs = src->src_funcs;

  if (!funcs || src->set_pattern != src->active_pattern) {
    if (src->src_impl && funcs)
      funcs->free (src->src_impl);
    src->src_funcs = funcs =
        gst_vr_test_src_get_src_funcs_for_pattern (src->set_pattern);
    if (funcs == NULL) {
      GST_ERROR_OBJECT (src, "Could not find an implementation of the "
          "requested pattern");
      src->gl_result = FALSE;
      return;
    }
    src->src_impl = funcs->create (src);
    if (!(src->gl_result =
            funcs->init (src->src_impl, src->context, &src->out_info))) {
      GST_ERROR_OBJECT (src, "Failed to initialize pattern");
      return;
    }
    src->active_pattern = src->set_pattern;
  }

  src->gl_result = funcs->fill_bound_fbo (src->src_impl);
  if (!src->gl_result)
    GST_ERROR_OBJECT (src, "Failed to render the pattern");
}

static GstStateChangeReturn
gst_vr_test_src_change_state (GstElement * element, GstStateChange transition)
{
  GstVRTestSrc *src = GST_VR_TEST_SRC (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG_OBJECT (src, "changing state: %s => %s",
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_gl_ensure_element_data (element, &src->display,
              &src->other_context))
        return GST_STATE_CHANGE_FAILURE;

      gst_gl_display_filter_gl_api (src->display, SUPPORTED_GL_APIS);
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      if (src->other_context) {
        gst_object_unref (src->other_context);
        src->other_context = NULL;
      }

      if (src->display) {
        gst_object_unref (src->display);
        src->display = NULL;
      }
      break;
    default:
      break;
  }

  return ret;
}
