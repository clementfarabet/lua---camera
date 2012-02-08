require 'xlua'
require 'camera'
width = 1920
height = 1080
fps = 30 
dir = 'test_vid_two'

sys.execute(string.format('mkdir -p %s',dir))

camera1 = image.Camera{idx=1,width=width,height=height,fps=fps}
camera2 = image.Camera{idx=2,width=width,height=height,fps=fps}

a1 = camera1:forward()
a2 = camera2:forward()
win = image.display{win=win,image={a1,a2}}

frame = torch.Tensor(3,height,2*width)
f = 1 

while true do
   sys.tic()
   a1 = camera1:forward()
   a2 = camera2:forward()

   frame:narrow(3,1,width):copy(a1)
   frame:narrow(3,width,width):copy(a2)

   image.display{win=win,image=frame}
   -- image.savePNG(string.format("%s/frame_%05d.png",dir,f),frame)
   f = f + 1
   print("FPS: ".. 1/sys.toc()) 
end