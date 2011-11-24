require 'xlua'
require 'camera'

camera = image.Camera{}

a = camera:forward()

image.savePNG("test.png", a)
