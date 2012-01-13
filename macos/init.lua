
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
   self.tensorsized = {}
   self.buffer = {}
   self.tensortyped = {}
   for i = 1,#self.idx do
      self.tensorsized[i] = torch.FloatTensor(3, height, width)
      self.buffer[i] = torch.FloatTensor()
      self.tensortyped[i] = torch.Tensor(3, height, width)
   end
end

function Camera:forward()
   -- grab all frames
   libcammacos.grabFrames(self.buffer)

   -- process all frames
   for i = 1,#self.idx do
      -- resize frames
      if self.tensorsized[i]:size(2) ~= self.buffer[i]:size(2) or self.tensorsized[i]:size(3) ~= self.buffer[i]:size(3) then
         image.scale(self.buffer[i], self.tensorsized[i])
      else
         self.tensorsized[i] = self.buffer[i]
      end
      -- retype frames
      if self.tensortyped[i]:type() ~= self.tensorsized[i]:type() then
         self.tensortyped[i]:copy(self.tensorsized[i])
      else
         self.tensortyped[i] = self.tensorsized[i]
      end
   end

   -- done
   return self.tensortyped
end

function Camera:stop()
   libcammacos.releaseCameras(self.idx)
end
