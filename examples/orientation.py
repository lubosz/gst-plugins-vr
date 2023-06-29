#!/usr/bin/env python3

# GStreamer Plugins VR
# Copyright 2023 Collabora Ltd.
# Author: Lubosz Sarnecki <lubosz.sarnecki@collabora.com>
# SPDX-License-Identifier: LGPL-2.1-or-later

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

import numpy as np
from scipy.spatial.transform import Rotation
import argparse


def on_message(bus, message):
    if message.type == Gst.MessageType.EOS:
        print("End-of-stream reached")
    elif message.type == Gst.MessageType.ERROR:
        err, debug = message.parse_error()
        print(f"Error: {err} {debug}")


class State():
    def __init__(self):
        self.roll = 0  # Rotation around the x-axis
        self.pitch = 0  # Rotation around the y-axis
        self.yaw = 0  # Rotation around the z-axis
        self.vrcompositor = None


def rotate(state):
    # Animate pitch
    state.pitch += 0.01

    r = Rotation.from_euler('xyz', [state.roll, state.pitch, state.yaw])
    quaternion = GLib.Variant('ad', list(r.as_quat()))
    state.vrcompositor.set_property("orientation", quaternion)

    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("video_path", help="Path to equirect video.")
    args = parser.parse_args()

    Gst.init(None)

    pipeline = Gst.parse_launch("filesrc name=src ! decodebin ! glupload ! vrcompositor name=comp ! glimagesink")

    filesrc = pipeline.get_by_name("src")
    filesrc.set_property("location", args.video_path)

    vrcompositor = pipeline.get_by_name("comp")

    s = State()
    s.vrcompositor = vrcompositor

    bus = pipeline.get_bus()
    bus.add_signal_watch()
    bus.connect("message", on_message)

    pipeline.set_state(Gst.State.PLAYING)

    # Animate every 10 ms
    timeout_id = GLib.timeout_add(10, rotate, s)

    main_loop = GLib.MainLoop()
    try:
        main_loop.run()
    except KeyboardInterrupt:
        pass

    pipeline.set_state(Gst.State.NULL)

if __name__ == '__main__':
    main()
