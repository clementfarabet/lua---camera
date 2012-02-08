require 'xlua'
require 'sys'
require 'camera'
width = 320
height = 240
fps = 30
dir = 'test_vid_hex_nowide'

sys.execute(string.format('mkdir -p %s',dir))

camera1 = image.Camera{idx=1,width=width,height=height,fps=fps}
camera2 = image.Camera{idx=2,width=width,height=height,fps=fps}
camera3 = image.Camera{idx=3,width=width,height=height,fps=fps}
camera4 = image.Camera{idx=4,width=width,height=height,fps=fps}
camera5 = image.Camera{idx=5,width=width,height=height,fps=fps}

a1 = camera1:forward()
a2 = camera2:forward()
a3 = camera3:forward()
a4 = camera4:forward()
a5 = camera5:forward()
win = image.display{win=win,image={a1,a2,a3,a4,a5}}
f = 1

frame = torch.Tensor(3,240,1600)

while true do
   sys.tic()
   a1 = camera1:forward()
   a2 = camera2:forward()
   a3 = camera3:forward()
   a4 = camera4:forward()
   a5 = camera5:forward()
   frame:narrow(3,1,320):copy(a5)
   frame:narrow(3,320,320):copy(a4)
   frame:narrow(3,320*2,320):copy(a3)
   frame:narrow(3,320*3,320):copy(a2)
   frame:narrow(3,320*4,320):copy(a1)
   image.display{win=win,image=frame}
   image.savePNG(string.format("%s/frame_%05d.png",dir,f),frame)
   f = f + 1
   print("FPS: ".. 1/sys.toc()) 
end