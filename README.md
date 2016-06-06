# GStreamer VR Plugins

This repository contains GStreamer plugins for watching spherical video in VR and a Python GTK+ player SPHVR.

## Disclaimer

Gst VR Plugins are in a very early development stage, you will get motion sick :)

## Dependencies

### VR Plugins

* GStreamer
* GStreamer GL Plugins
* Meson
* OpenHMD
* libfreenect2
* graphene

### SPHVR

* Python 3
* GTK+ 3.X

## Installation

```
./configure
make
```

## Usage

### View spherical video on a DK2

```
gst-launch-1.0 filesrc location=~/video.webm ! decodebin ! glupload ! glcolorconvert ! videorate ! vrcompositor ! video/x-raw\(memory:GLMemory\), width=1920, height=1080, framerate=75/1 ! hmdwarp ! glimagesink
```

### Open 2 Windows with Tee

```
GST_GL_XINITTHREADS=1 \ gst-launch-1.0 filesrc location=~/video.webm ! decodebin ! videoscale ! glupload ! glcolorconvert ! videorate ! vrcompositor ! video/x-raw\(memory:GLMemory\), width=1920, height=1080, framerate=75/1 ! hmdwarp ! tee name=t ! queue ! glimagesink t. ! queue ! glimagesink
```

### Display point cloud from Kinect v2

```
gst-launch-1.0 freenect2src sourcetype=0 ! glupload ! glcolorconvert ! pointcloudbuilder ! video/x-raw\(memory:GLMemory\), width=1920, height=1080 ! glimagesink
```

## License

GPLv2
