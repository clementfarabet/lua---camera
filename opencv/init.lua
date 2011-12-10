
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
   local args, idx, width, height = xlua.unpack(
      {...},
      'image.Camera', nil,
      {arg='idx', type='number', help='camera index', default=0},
      {arg='width', type='number', help='frame width', default=640},
      {arg='height', type='number', help='frame height', default=480}
   )

   -- init vars
   self.tensorsized = torch.FloatTensor(3, height, width)
   self.buffer = torch.FloatTensor()
   self.tensortyped = torch.Tensor(3, height, width)

   -- init capture
   self.idx = libcamopencv.initCam(idx, width, height)
end

function Camera:forward()
   libcamopencv.grabFrame(self.idx, self.buffer)
   if self.tensorsized:size(2) ~= self.buffer:size(2) or self.tensorsized:size(3) ~= self.buffer:size(3) then
      image.scale(self.buffer, self.tensorsized)
   else
      self.tensorsized = self.buffer
   end
   if self.tensortyped:type() ~= self.tensorsized:type() then
      self.tensortyped:copy(self.tensorsized)
   else
      self.tensortyped = self.tensorsized
   end
   return self.tensortyped
end

function Camera:stop()
  libcamopencv.releaseCam(self.idx)
  print('stopping camera')
end
