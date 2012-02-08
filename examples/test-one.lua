require 'torch'
require 'sys'
require 'camera'
width  = 1920
height = 1080
fps = 30
dir = "demo_test"

sys.execute(string.format('mkdir -p %s',dir))

camera1 = image.Camera{idx=1,width=width,height=height,fps=fps}

a1 = camera1:forward()
win = image.display{win=win,image={a1}}
f = 1

while true do
   sys.tic()
   a1 = camera1:forward()
   image.display{win=win,image={a1}}
   -- image.savePNG(string.format("%s/frame_1_%05d.png",dir,f),a1)
   f = f + 1
   print("FPS: ".. 1/sys.toc()) 
end