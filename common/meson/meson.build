gst_version = '@0@.@1@.@2@'.format(gst_major, gst_minor, gst_micro)
apiversion = '1.0'
soversion = 0

prefix = get_option('prefix')

cc = meson.get_compiler('c')
cdata = configuration_data()
cdata.set('GST_VERSION_MAJOR', gst_major)
cdata.set('GST_VERSION_MINOR', gst_minor)
cdata.set('GST_VERSION_MICRO', gst_micro)
cdata.set('GST_VERSION_NANO', gst_nano)
cdata.set('GST_API_VERSION', '"@0@"'.format(apiversion))
cdata.set('GST_DATADIR', '"@0@/@1@"'.format(prefix, get_option('datadir')))
cdata.set('LOCALEDIR', '"@0@/@1@"'.format(prefix, get_option('localedir')))
cdata.set('LIBDIR', '"@0@/@1@"'.format(prefix, get_option('libdir')))
cdata.set('GST_API_VERSION', '"' + apiversion + '"')
cdata.set('GETTEXT_PACKAGE', '"gstreamer' + apiversion + '"')
cdata.set('GST_LEVEL_DEFAULT', 'GST_LEVEL_NONE')
cdata.set('GST_LICENSE', '"LGPL"')
cdata.set('GST_PACKAGE_ORIGIN', '"c"')
cdata.set('GST_PACKAGE_NAME', '"GStreamer Plugins VR source release"')
cdata.set('PACKAGE', '"gst-plugins-vr"')
cdata.set('PACKAGE_NAME', '"GStreamer Plugins VR"')
cdata.set('PACKAGE_STRING', '"GStreamer Plugins VR @0@"'.format(gst_version))
cdata.set('PACKAGE_TARNAME', '"gstreamer-plugins-vr"')
cdata.set('PACKAGE_BUGREPORT', '"https://github.com/lubosz/gst-plugins-vr"')
cdata.set('PACKAGE_URL', 'https://github.com/lubosz/gst-plugins-vr""')
cdata.set('PACKAGE_VERSION', '"@0@"'.format(gst_version))
cdata.set('PLUGINDIR', '"@0@/lib/gstreamer-@1@"'.format(get_option('prefix'), apiversion))
cdata.set('VERSION', '"@0@"'.format(gst_version))

if cc.has_function('getrusage', prefix : '#include<sys/resource.h>')
  cdata.set('HAVE_GETRUSAGE', 1)
endif

configure_file(input : 'config.h.meson',
  output : 'config.h',
  configuration : cdata)
