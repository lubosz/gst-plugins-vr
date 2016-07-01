import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk
from gi.repository import Gst


class GstOverlaySink:
    def __init__(self, name, w, h):
        self.element = Gst.ElementFactory.make(name, None)
        self.widget = Gtk.DrawingArea()
        self.widget.set_size_request(w, h)
        self.widget.set_double_buffered(True)

    def xid(self):
        return self.widget.get_window().get_xid()

    def set_handle(self):
        self.element.set_window_handle(self.xid())


class GtkGLSink:
    def __init__(self):
        self.element = Gst.ElementFactory.make("gtkglsink", None)
        self.widget = self.element.get_property("widget")

    def set_handle(self):
        pass