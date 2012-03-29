
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
      {arg='fps', type='number', help='optional frame rate (v4l2 only)', default=1}
   )
   -- init vars
   self.camidx = idx
   self.width = width
   self.height = height
   self.nbuffers = nbuffers
   self.fps = fps
   self.tensor = torch.FloatTensor(3,height,width)
   self.tensortyped = torch.Tensor(self.tensor:size())
   libv4l.init(self.camidx, self.width, self.height, self.fps, self.nbuffers)
end

function Camera:adjustManualFocus(f)
   libv4l.adjustManualFocus(self.camidx,f)
end

function Camera:setManualFocus()
   libv4l.setFocusType(self.camidx,0)
end

function Camera:setAutoFocus()
   libv4l.setFocusType(self.camidx,1)
end

function Camera:setMacroFocus()
   libv4l.setFocusType(self.camidx,2)
end

function Camera:setContinuousFocus()
   libv4l.setFocusType(self.camidx,3)
end

function Camera:forward(tensor)
   libv4l.grabFrame(self.camidx, self.tensor)
   if tensor then
      if (self.tensor:type() ~= tensor:type()) then
	 self.tensortyped:copy(self.tensor)
	 image.scale(self.tensortyped, tensor)
	 return tensor
      end
      image.scale(self.tensor, tensor)
      return tensor
   end
   if (self.tensor:type() ~=  self.tensortyped:type()) then
      self.tensortyped:copy(self.tensor)
      return self.tensortyped
   end
   return self.tensor
end

function Camera:stop()
   print('stopping camera')
end
