#!/bin/bash
gst-launch-1.0 v4l2src io-mode=2 device=/dev/video0 ! video/x-raw,width=1920,height=1080 ! waylandsink window-width=1280 window-height=720 &

