
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
   self.tensor = torch.Tensor(3,480,640)
   sys.sleep(2)
end

function Camera:forward(tensor)
   libcamiface.getSharedFrame(self.sharedMemFile, self.tensor, false)
   if tensor then
      image.scale(self.tensor,tensor)
   else
      tensor = self.tensor
   end
   return tensor
end

function Camera:stop()
   print('stopping camera')
   sys.execute('rm -f shared-mem')
end
