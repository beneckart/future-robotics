#!/usr/bin/env python

"""A demo client for Open Pixel Control
http://github.com/zestyping/openpixelcontrol

Sends red, green, and blue to the first 3 LEDs.

To run:
First start the gl simulator using, for example, the included "wall" layout

    make
    bin/gl_server layouts/wall.json

Then run this script in another shell to send colors to the simulator

    python_clients/example.py

"""

import time
import random
import opc

ADDRESS = '10.0.0.14:7890'

# Create a client object
client = opc.Client(ADDRESS)

# Test if it can connect
if client.can_connect():
    print 'connected to %s' % ADDRESS
else:
    # We could exit here, but instead let's just print a warning
    # and then keep trying to send pixels in case the server
    # appears later
    print 'WARNING: could not connect to %s' % ADDRESS

'''
# Send pixels forever
my_pixels = [(255, 0, 0), (0, 255, 0), (0, 0, 255),(128,128,128),(255,255,0),(0,255,255),(255,0,255),(0,0,0),(0,128,128),(128,0,128)]*50
while True:
    #my_pixels = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
    random.shuffle(my_pixels)
    
    if client.put_pixels(my_pixels, channel=0):
        print 'sent'
    else:
        print 'not connected'
    #time.sleep(0.3)'''

from PIL import Image, ImageSequence, ImageOps
import sys, os
filename = sys.argv[1]
im = Image.open(filename)
#rgb_im = im.convert('RGB')

#for pixel in rgb_im.getdata():
#    print pixel
rows = 20
cols = 25

pixelmap = []

for i in range(rows):
    for j in range(cols):
        fwd = i % 2 == 0
        if fwd:
            ypos = j
            xpos = i
        else:
            ypos = (cols-j-1)
            xpos = i
        pixelmap.append(ypos+xpos*25)
        
#print pixelmap

#original_duration = im.info['duration']

#padding = (0,0,0,0)

#frames = [ImageOps.expand(frame.copy().convert('RGB'), padding) for frame in ImageSequence.Iterator(im)]#[:2]

frames = [frame.copy().convert('RGB') for frame in ImageSequence.Iterator(im)]#[:2]

#frames = frames[1:]
#frames = [frames[0]]

frames = [fr.resize((25,20)) for fr in frames]

#frames = [frames[0].rotate(i) for i in range(0,360,3)]

'''frames_new = []
for fr in frames:
    #fr.thumbnail((20,20))
    old_size = (20, 20)
    new_size = (25, 20)
    new_im = Image.new("RGB", new_size)   ## luckily, this is already black!
    new_im.paste(fr, ((new_size[0]-old_size[0])//2,
                      (new_size[1]-old_size[1])//2))   
    frames_new.append(new_im)
frames = frames_new#[1:]'''

frames = [fr.getdata() for fr in frames]

import numpy as np
for i in range(len(frames)):
    new_im = frames[i].copy()
    #print len(new_im), np.amax(pixelmap)
    #raw_input()
    frames[i] = [new_im[pixelmap[j]] for j in range(len(new_im))]      

rgb = []
for fr in frames:
    rgb.append([(g,r,b) for (r,g,b) in fr][::-1])
   
i = 0
st = time.time()
while True:
    
    #print dir(frames[i%len(frames)])
    #print [x for x in frames[i%len(frames)].getdata()]
    #raw_input()
    #bytes = ''.join(x if j%4!=3 else '' for j, x in enumerate(frames[i%len(frames)].tobytes()) )

    #if client.put_bytes(frames[i%len(frames)].tobytes()[::-1], channel=0):
    #if client.put_pixels([x for x in frames[i%len(frames)].getdata()], channel=0):
    if client.put_pixels(rgb[i%len(rgb)], channel=0):
        pass #print 'sent'
    else:
        print 'not connected'
    if time.time() - st > 0.1:
        i+=1
        st = time.time()
    time.sleep(0.017)
    #time.sleep(0.01-(time.time()-st))