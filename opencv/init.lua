
----------------------------------
-- dependencies
----------------------------------
require 'torch'
require 'xlua'
require 'image'
require 'libopencv'

----------------------------------
-- a camera class
----------------------------------
local Camera = torch.class('image.Camera')

function Camera:__init(...)
   -- parse args
   local args, idx = xlua.unpack(
      {...},
      'ImageSource', help_desc,
      {arg='idx', type='number', help='camera index', default=0}
   )
   -- init vars
   self.height = 480
   self.width = 640
   self.camidx = (idx or 0);
   self.tensor = torch.DoubleTensor(3,self.height,self.width)
   
   -- init capture
   libopencv.initCam(idx);
end

function Camera:forward()
   libopencv.grabFrame(self.tensor)
   -- image.savePNG("forward.png",self.tensor)
   image.scale(self.tensor,self.width,self.height)
   return self.tensor
end

function Camera:stop()
  libopencv.releaseCam()
  print('stopping camera')
end
