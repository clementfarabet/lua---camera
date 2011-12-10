
----------------------------------
-- dependencies
----------------------------------
require 'torch'
require 'xlua'
require 'image'
require 'libv4l'

----------------------------------
-- a camera class
----------------------------------
local Camera = torch.class('image.Camera')

function Camera:__init(...)
   -- parse args
   local args, idx, width, height, nbuffers, fps = xlua.unpack(
      {...},
      'ImageSource', help_desc,
      {arg='idx', type='number', help='camera index', default=0},
      {arg='width', type='number', help='width', default=640},
      {arg='height', type='number', help='height', default=480},
      {arg='buffers', type='number', help='number of buffers (v4l2 only)', default=1},
      {arg='fps', type='number', help='optional frame rate (v4l2 only)', default=30}
   )
   -- init vars
   self.camidx = idx
   self.width = width
   self.height = height
   self.nbuffers = nbuffers
   self.fps = fps
   self.tensor = torch.DoubleTensor(3,height,width)
   self.tensortyped = torch.Tensor(self.tensor:size())
   libv4l.init(self.camidx, self.width, self.height, self.fps, self.nbuffers)
end

function Camera:forward(tensor)
   libv4l.grabFrame(self.camidx, self.tensor)
   self.tensortyped:copy(self.tensor)
   if tensor then
      image.scale(self.tensortyped, tensor)
      return tensor
   end
   return self.tensortyped
end

function Camera:stop()
   print('stopping camera')
end
