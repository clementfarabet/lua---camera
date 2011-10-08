
----------------------------------
-- dependencies
----------------------------------
require 'torch'
require 'image'
require 'sys'
require 'libcamiface'

----------------------------------
-- a camera class
----------------------------------
local Camera = torch.class('image.Camera')

function Camera:__init()
   self.threadID = libcamiface.forkProcess('frame_grabber-quicktime')
   self.sharedMemFile = 'shared-mem'
   self.tensor = torch.DoubleTensor(3,480,640)
   self.tensortyped = torch.Tensor(self.tensor:size())
   sys.sleep(2)
end

function Camera:forward()
   libcamiface.getSharedFrame(self.sharedMemFile, self.tensor, false)
   self.tensortyped:copy(self.tensor)
   if tensor then
      image.scale(self.tensortyped, tensor)
      return tensor
   end
   return self.tensortyped
end

function Camera:stop()
   libcamiface.getSharedFrame(self.sharedMemFile, self.tensor, true)
end
