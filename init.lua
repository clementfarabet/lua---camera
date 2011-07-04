
----------------------------------
-- dependencies
----------------------------------
require 'sys'
require 'xlua'

----------------------------------
-- load camera driver based on OS
----------------------------------
if sys.OS == 'linux' then
   if not xlua.require 'video4linux' then
      xlua.error('failed to load video4linux wrapper: verify that you have v4l2 libs')
   end
elseif sys.OS == 'macos' then
   if not xlua.require 'camiface' then
      xlua.error('failed to load camiface wrapper: verify that libcamiface is installed')
   end
else
   xlua.error('not camera driver available for your OS, sorry :-(')
end

----------------------------------
-- package a little demo
----------------------------------
camera = {}
camera.testme = function()
                   local cam = image.Camera{}
                   local w = nil
                   for i = 1,200 do -- ~10 seconds
                      local frame = cam:forward()
                      w = image.display{image=frame, win=w}
                   end
                   cam:stop()
                   return w
                end
