
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
   self.camidx = (idx or 0);
   self.tensor = torch.DoubleTensor(3,480,640)
   self.tensortyped = torch.Tensor(self.tensor:size())

   -- init capture
   libopencv.initCam(idx);
end

function Camera:forward()
   libopencv.grabFrame(self.tensor)
   self.tensortyped:copy(self.tensor)
   if tensor then
      image.scale(self.tensortyped, tensor)
      return tensor
   end
   return self.tensortyped
end

function Camera:stop()
  libopencv.releaseCam()
  print('stopping camera')
end
