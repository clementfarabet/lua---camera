
INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install camera

TODO:
All fixed and debugged on MacOSX. 
Needs to be tested on Linux.

USE:
$ lua
> require 'camera'
> camera.testme()   -- a simple grabber+display
> cam = image.Camera()  -- create the camera grabber
> frame = cam:forward()  -- return the next frame available
> cam:stop() -- release the camera
> image.display(frame)  -- display frame
