require 'xlua'
xrequire('camera',true)

camera = image.Camera{}

a = camera:forward()
image.savePNG("test2.png",a)
