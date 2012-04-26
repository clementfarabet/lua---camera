#!/usr/bin/env torch
require 'torch'
require 'sys'
require 'camera'
require 'xlua'

op = xlua.OptionParser('%prog [options]')
op:option{'-dev','--device', action='store',dest='devvideo',
help='number of the /dev/video* device from which you want to capture',
	   default=0}
op:option{'-w','--width', action='store', dest='width',
	   help='width of frames you want to capture', default=640}
op:option{'-h','--height', action='store', dest='height',
	   help='height of frames you want to capture', default=480}
op:option{'-r','--fps', action='store', dest='fps',
   help='fps at which you want to capture', default=15}

opt, args = op:parse()
op:summarize()

dev    = tonumber(opt.devvideo)
width  = tonumber(opt.width)
height = tonumber(opt.height)
fps    = tonumber(opt.fps)

camera1 = image.Camera{idx=dev,width=width,height=height,fps=fps}

a1 = camera1:forward()
f = 1

while true do
   sys.tic()
   a1 = camera1:forward()
   f = f + 1
   print("FPS: ".. 1/sys.toc())
end
