import gi
gi.require_version('Gdk', '3.0')
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
gi.require_version('GstVideo', '1.0')
gi.require_version('GstGL', '1.0')
gi.require_version('Graphene', '1.0')

from gi.repository import GLib
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import Gst, GstGL, Graphene

# from vrsink.scene import *


class GstOverlaySink(Gtk.DrawingArea):
    def __init__(self, name, w, h):
        Gtk.DrawingArea.__init__(self)
        from gi.repository import GdkX11, GstVideo

        self.sink = Gst.ElementFactory.make(name, None)
        self.set_size_request(w, h)
        self.set_double_buffered(True)

    def xid(self):
        return self.get_window().get_xid()

    def set_handle(self):
        self.sink.set_window_handle(self.xid())


class CairoGLSink(GstOverlaySink):
    def __init__(self, w, h):
        GstOverlaySink.__init__(self, "glimagesink", w, h)

        self.gl_init = False

        # self.scene = GL3Demo()

        # self.sink.connect("client-draw", self.scene.draw)
        # self.sink.connect("client-reshape", self.scene.reshape)

        # self.sink.handle_events(False)

        self.canvas_width, self.canvas_height = w, h
        self.aspect = w/h

        self.transformation_element = None
        self.pipeline = None
        self.pipeline_position = 0

    def on_button_press(self, sink, event):
        # self.scene.on_press(event)

        if self.pipeline.get_state(Gst.CLOCK_TIME_NONE)[1] == Gst.State.PAUSED:
            GLib.timeout_add(90, self.flush_seek)
