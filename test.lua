require 'image'
require 'cammacos'

camera = image.Camera{idx={0,1}, width=320, height=240}
for i = 1,500 do
   sys.tic()
   a = camera:forward()
   sys.toc(true)
   d = image.display{image=a, win=d}
end

camera:stop()
