/*
 * GStreamer Plugins VR
 * Copyright (C) 2016 Lubosz Sarnecki <lubosz@collabora.co.uk>
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

#ifndef __GST_FREENECT2_SRC_H__
#define __GST_FREENECT2_SRC_H__

#include <gst/gst.h>
#include <stdio.h>
//#include <OpenNI.h>

#include <string>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
//#include <libfreenect2/threading.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>

#include <gst/base/gstbasesrc.h>
#include <gst/base/gstpushsrc.h>
#include <gst/video/video.h>

G_BEGIN_DECLS
#define GST_TYPE_FREENECT2_SRC \
  (gst_freenect2_src_get_type())
#define GST_FREENECT2_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FREENECT2_SRC,GstFreenect2Src))
#define GST_FREENECT2_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FREENECT2_SRC,GstFreenect2SrcClass))
#define GST_IS_FREENECT2_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FREENECT2_SRC))
#define GST_IS_FREENECT2_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FREENECT2_SRC))
typedef struct _GstFreenect2Src GstFreenect2Src;
typedef struct _GstFreenect2SrcClass GstFreenect2SrcClass;

typedef enum
{
  GST_FREENECT2_SRC_FILE_TRANSFER,
  GST_FREENECT2_SRC_NEXT_PROGRAM_CHAIN,
  GST_FREENECT2_SRC_INVALID_DATA
} GstFreenect2State;

struct _GstFreenect2Src
{
  GstPushSrc element;

  GstFreenect2State state;
  gchar *uri_name;
  gint sourcetype;
  GstVideoInfo info;
  GstCaps *gst_caps;

  /* Timestamp of the first frame */
  GstClockTime oni_start_ts;

  /* Freenect2 variables */

  libfreenect2::Freenect2 *freenect2;
  libfreenect2::Freenect2Device *dev;
  libfreenect2::PacketPipeline *pipeline;

  libfreenect2::SyncMultiFrameListener *listener;
  libfreenect2::FrameMap frames;
  libfreenect2::Frame *undistorted; 
  libfreenect2::Frame *registered;

  libfreenect2::Registration* registration;


  /*
  openni::Device *device;
  openni::VideoStream *depth, *color;
  openni::VideoMode depthVideoMode, colorVideoMode;
  openni::PixelFormat depthpixfmt, colorpixfmt;
  */
  int width, height, fps;
  //openni::VideoFrameRef *depthFrame, *colorFrame;
};

struct _GstFreenect2SrcClass
{
  GstPushSrcClass parent_class;
};

GType gst_freenect2_src_get_type (void);

G_END_DECLS
#endif /* __GST_FREENECT2_SRC_H__ */
