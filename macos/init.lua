
----------------------------------
-- dependencies
----------------------------------
require 'torch'
require 'xlua'
require 'image'
require 'libcammacos'

----------------------------------
-- a camera class
----------------------------------
local Camera = torch.class('image.Camera')

function Camera:__init(...)
   -- parse args
   local args, idx, width, height = xlua.unpack(
      {...},
      'image.Camera', nil,
      {arg='idx', type='number | table', help='camera index, or tables of indices', default=0},
      {arg='width', type='number', help='frame width', default=640},
      {arg='height', type='number', help='frame height', default=480}
   )

   -- init vars
   if type(idx) == 'number' then
      self.idx = {idx}
   else
      self.idx = idx
   end

   -- init capture
   local nbcams = libcammacos.initCameras(self.idx, width, height)

   -- cleanup
   if nbcams < #self.idx then
      self.idx[nbcams+1] = nil
   end

   -- buffers
   self.tensorsized = torch.FloatTensor(3, height, width)
   self.buffer = torch.FloatTensor()
   self.tensortyped = torch.Tensor(3, height, width)
end

function Camera:forward()
   libcammacos.grabFrame(self.idx, self.buffer)
   if self.tensorsized:size(2) ~= self.buffer:size(2) or self.tensorsized:size(3) ~= self.buffer:size(3) then
      image.scale(self.tensorsized, self.buffer)
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
   libcammacos.releaseCameras(self.idx)
end
