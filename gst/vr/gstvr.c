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

#define GST_USE_UNSTABLE_API

#include "config.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gst/gst.h>
#include "gstvrcompositor.h"
#include "gsthmdwarp.h"
#include "gstvrtestsrc.h"
#include "gstpointcloudbuilder.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "vrcompositor", GST_RANK_NONE, gst_vr_compositor_get_type()))
    return FALSE;
 
  if (!gst_element_register (plugin, "vrtestsrc", GST_RANK_NONE, gst_vr_test_src_get_type()))
    return FALSE;

  if (!gst_element_register (plugin, "hmdwarp", GST_RANK_NONE, gst_hmd_warp_get_type()))
    return FALSE;

  if (!gst_element_register (plugin, "pointcloudbuilder", GST_RANK_NONE, gst_point_cloud_builder_get_type()))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vr,
    "GStreamer VR Plugins",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
