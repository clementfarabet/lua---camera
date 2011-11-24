
----------------------------------
-- dependencies
----------------------------------
require 'torch'
require 'xlua'
require 'image'
require 'libcamopencv'

----------------------------------
-- a camera class
----------------------------------
local Camera = torch.class('image.Camera')

function Camera:__init(...)
   -- parse args
   local args, idx = xlua.unpack(
      {...},
      'image.Camera', nil,
      {arg='idx', type='number', help='camera index', default=0}
   )
   -- init vars
   self.height = 480
   self.width = 640
   self.camidx = (idx or -1);
   self.tensor = torch.DoubleTensor(3,self.height,self.width)
   self.tensortyped = torch.Tensor(self.tensor:size())

   -- init capture
   libcamopencv.initCam(idx);
end

function Camera:forward()
   libcamopencv.grabFrame(self.tensor)
   self.tensortyped:copy(self.tensor)
   return self.tensortyped
end

function Camera:stop()
  libcamopencv.releaseCam()
  print('stopping camera')
end
