
DEPENDENCIES:
MacOS: Install OpenCV 2.X: http://opencv.willowgarage.com/
Linux: None

INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install camera

STATE:
MacOS: working on all MacOS builds, using OpenCV (wrapper from Jordan Bates)
Linux: working all right, using raw video4linux2 (wrapper from Clement Farabet)

USE:
$ lua
> require 'camera'
> camera.testme()   -- a simple grabber+display
> cam = image.Camera()  -- create the camera grabber
> frame = cam:forward()  -- return the next frame available
> cam:stop() -- release the camera
> image.display(frame)  -- display frame
