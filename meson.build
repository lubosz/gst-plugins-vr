project('gst-plugins-vr', 'c', 'cpp', meson_version : '>= 0.28.0')

gst_major = 0
gst_minor = 1
gst_micro = 0
gst_nano = 0
subdir('common/meson')

# Commented for release versions
gst_version = '@0@.@1@'.format(gst_version, gst_nano)

GST_DEP_VERSION = '>= @0@.@1@'.format(gst_major, gst_minor)

gobject_dep = dependency('gobject-2.0')
graphene_dep = dependency('graphene-1.0', fallback : ['graphene', 'graphene_dep'])
openhmd_dep = dependency('openhmd', required : false)
freenect2_dep = dependency('freenect2', required : false)
glib_dep = dependency('glib-2.0')
gio_dep = dependency('gio-2.0')
gst_dep = dependency('gstreamer-' + apiversion, version: GST_DEP_VERSION)
gst_gl_dep = dependency('gstreamer-gl-' + apiversion, version: GST_DEP_VERSION)
gst_video_dep = dependency('gstreamer-video-' + apiversion, version: GST_DEP_VERSION)

gst3dincludes = include_directories('gst-libs/')

# The Gst3D library
install_headers(
  'gst-libs/gst/3d/gst3dmesh.h',
  'gst-libs/gst/3d/gst3dcamera.h',
  'gst-libs/gst/3d/gst3dhmd.h',
  'gst-libs/gst/3d/gst3drenderer.h',
  'gst-libs/gst/3d/gst3dshader.h',
  subdir : 'gstreamer-' + apiversion + '/gst/3d')

gst_3d_lib_src_hmd = []
vr_plugin_src_hmd = []

if openhmd_dep.found()
  gst_3d_lib_src_hmd = ['gst-libs/gst/3d/gst3dhmd.c', 'gst-libs/gst/3d/gst3dcamera_hmd.c', 'gst-libs/gst/3d/gst3drenderer.c']
  vr_plugin_src_hmd = ['gst/vr/gsthmdwarp.c']
  add_global_arguments('-DHAVE_OPENHMD=1', language : 'c')
endif

gst_3d_lib = shared_library('gst3d-' + apiversion,
  'gst-libs/gst/3d/gst3dmesh.c',
  'gst-libs/gst/3d/gst3dcamera.c',
  'gst-libs/gst/3d/gst3dcamera_arcball.c',
  'gst-libs/gst/3d/gst3dcamera_wasd.c',
  'gst-libs/gst/3d/gst3dshader.c',
  'gst-libs/gst/3d/gst3dnode.c',
  'gst-libs/gst/3d/gst3dscene.c',
  'gst-libs/gst/3d/gst3dmath.c',
  gst_3d_lib_src_hmd,
  install: true,
  dependencies: [glib_dep, gobject_dep, gst_dep, gst_gl_dep, gst_video_dep, graphene_dep, openhmd_dep, gio_dep],
  c_args: ['-Wno-pedantic'],
  cpp_args: ['-fpermissive'],
  link_args: ['-lm'],
  soversion : '0')

'''
gnome = import ('gnome')
girtargets = gnome.generate_gir(gst_vr_lib,
  sources : ['gst-libs/gst/vr/gstvrgeometry.h',
             'gst-libs/gst/vr/gstvrgeometry.c'],
  nsversion : apiversion,
  namespace : 'GstVR',
  identifier_prefix : 'Gst',
  symbol_prefix : 'gst_',
  includes : ['GObject-2.0',
              'Gst-' + apiversion,
              'GstPbutils-' + apiversion],
  dependencies: [gst_dep],
  install : true,
  extra_args : ['--add-init-section=extern gboolean gst_init(gint *argc, gchar **argv); gst_init(NULL,NULL);']
)
'''

pkg_mod = import('pkgconfig')


pkg_mod.generate(libraries : gst_3d_lib,
      subdirs : ['gstreamer-' + apiversion],
      version : gst_version,
      name : 'libgst3d-' + apiversion,
      filebase :'gst3d-' + apiversion,
      description : 'A high level API for 3D in GStreamer.')

gst_c_args = ['-DHAVE_CONFIG_H',
  '-D_GNU_SOURCE',
  '-DGST_EXPORTS',
  '-DGST_DISABLE_DEPRECATED',
]

configure_file(input : 'common/meson/config.h.meson',
  output : 'config.h',
  configuration : cdata)


# glib-compile-resources
r = run_command('glib-compile-resources', '--target=gpu/shaders.c', '--sourcedir=gpu/', '--generate-source', 'gpu/shaders.gresource.xml')
if r.returncode() != 0
  message('Could not compile glib resources:')
  message(r.stderr().strip())
endif

vr_plugin_src = ['gst/vr/gstvrcompositor.c',
  'gst/vr/gstvrtestsrc.c',
  'gst/vr/vrtestsrc.c',
  'gst/vr/gsthmdwarp.c',
  'gst/vr/gstpointcloudbuilder.c',
  'gst/vr/gstvr.c',
  'gpu/shaders.c',
  vr_plugin_src_hmd]

gst_vr_plugins = shared_library('gstvrplugins',
  vr_plugin_src,
  install : true,
  dependencies : [glib_dep, gobject_dep, gst_dep, gst_gl_dep, gst_video_dep, graphene_dep, openhmd_dep, gio_dep],
  c_args : gst_c_args,
  install_dir : '@0@/gstreamer-1.0'.format(get_option('libdir')),
  link_with: [gst_3d_lib], 
  include_directories : gst3dincludes
)

if freenect2_dep.found()
  gst_freenect2 = shared_library('gstfreenect2',
    'gst/freenect2/gstfreenect2src.cpp',
    'gst/freenect2/gstfreenect2.cpp',
    install : true,
    dependencies : [glib_dep, gobject_dep, gst_dep, gst_gl_dep, gst_video_dep, graphene_dep, freenect2_dep, openhmd_dep, gio_dep],
    c_args : gst_c_args,
    install_dir : '@0@/gstreamer-1.0'.format(get_option('libdir')),
    link_with: [gst_3d_lib]
  )
endif

executable('gst-play-vr-' + apiversion,
  'tools/gst-play-vr.c',
  install : true,
  dependencies : [glib_dep, gobject_dep, gst_dep],
  link_with: [gst_3d_lib]
)

# tests

executable('camera', 'tests/3d/camera.c',
  install : false,
  dependencies : [glib_dep, gobject_dep, gst_dep, gst_gl_dep, gst_video_dep],
  link_with: [gst_3d_lib]
)

executable('mesh', 'tests/3d/mesh.c', 'gpu/shaders.c',
  install : false,
  dependencies : [glib_dep, gobject_dep, gst_dep, gst_gl_dep, gst_video_dep, graphene_dep, gio_dep],
  link_with: [gst_3d_lib]
)

# install sphvr
#install_data('sphvr/sphvr', install_dir : 'bin/')
#site_packages_dir = run_command('./scripts/print_sitepackages_dir.py').stdout().strip()
#install_subdir('sphvr', install_dir : site_packages_dir)
