# meson.build
project('gst-ddktmix', 'c',
  version : '1.1.0',
  meson_version : '>= 0.50',
  default_options : [ 'warning_level=1',
                     'buildtype=debugoptimized' ])

gst_version = '1.0'
version_arr = meson.project_version().split('.')
gst_version_major = version_arr[0].to_int()
gst_version_minor = version_arr[1].to_int()
gst_version_micro = version_arr[2].to_int()

gstreamer_dep = dependency('gstreamer-1.0', version : '>=1.0')
gstreamer_base_dep = dependency('gstreamer-base-1.0')
gstreamer_video_dep = dependency('gstreamer-video-1.0')

plugin_c_args = ['-DHAVE_CONFIG_H']

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

shared_library('gstddktmix',
  ['gstddktmix.c'],
  c_args : plugin_c_args,
  dependencies : [gstreamer_dep, gstreamer_base_dep, gstreamer_video_dep],
  install : true,
  install_dir : plugins_install_dir,
)
