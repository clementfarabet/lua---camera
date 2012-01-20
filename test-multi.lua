require 'xlua'
require 'camera'
width = 320
height = 240

camera1 = image.Camera{idx=1,width=width,height=height}
camera2 = image.Camera{idx=2,width=width,height=height}
camera3 = image.Camera{idx=3,width=width,height=height}
camera4 = image.Camera{idx=4,width=width,height=height}

a1 = camera1:forward()
a2 = camera2:forward()
a3 = camera3:forward()
a4 = camera4:forward()
win = image.display{win=win,image={a1,a2,a3,a4}}

while true do
   sys.tic()
   a1 = camera1:forward()
   a2 = camera2:forward()
   a3 = camera3:forward()
   a4 = camera4:forward()
   image.display{win=win,image={a1,a2,a3,a4}}
   print("FPS: ".. 1/sys.toc()) 
end