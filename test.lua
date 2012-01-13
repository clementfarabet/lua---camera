require 'image'
require 'cammacos'

camera = image.Camera{idx={0,1,2,3,4}, width=160, height=120}

for i = 1,500 do
   sys.tic()
   a = camera:forward()
   sys.toc(true)
   d = image.display{image=a, win=d, zoom=2}
end

camera:stop()
