How to build ?
```
meson build --prefix=/usr
```

How to use ?
```
gst-launch-1.0 filesrc location=1.h265 ! h265parse ! avdec_h265 ! videoconvert ! autovideosink
change to
gst-launch-1.0 filesrc location=1.h265 ! h265parse ! avdec_h265 ! videoconvert ! ddktmix ! videoconvert ! autovideosink
```

like tmix in ffmpeg, I use 9 frames all weights 1.
