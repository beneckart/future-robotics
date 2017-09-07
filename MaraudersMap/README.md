
# Marauder's Map

## Solar Panel & battery

Map was installed under the shade structure, so the solar panel needed to be manually moved off of it every morning for best charging. Solution: place map outside shade structure?

Seemed to charge 40Ah battery and then some each day. This battery (search "coolis solar battery" on amazon) was perfect for our needs, with three 12V/5A power rails, two usb out (5V/2A), and a 110W inverter. Note that inverter cannot be used while solar charging. Battery seemed to be able to last all night.

New task for campers? wipe down the panel if too dusty each morning
        
## LEDs

Used 8 strands of 50 WS2811 12V LEDs for the actual map and 1 strand of 5V WS2801b LEDs for the edge of the plexiglass to light the etchings. Note that even when WS2811 LEDs are 12V compatible, the control lines are still 5V. Additionally, the 12V line can dip below 12V without loss of functionality, only a drop in brightness. 

We used a rough printout to place the leds that showed strands by color, but it did not include an explicit ordering-- we need to do better here because it is not always clear how to order the leds, especially for the center leds where leds "leap frog" each other down the 12/3/6/9pm roads to the man. Furthermore, since the leds in the center were so packed, the holes ran together, making some sort of adhesive required to keep them in place. Unfortunately, we had trouble gluing these center leds into place and so left them mostly unconnected. 

Debugging: it was determined that some of the leds were placed incorrectly. To prevent this, it might be helpful to write on the plywood board at least the start and stop locations, or, connect neighboring leds with a drawn line (with arrows showing directionality) using a marker. Lastly, there was some confusion about whether we should wire the LEDs into the plywood "mirrored left-right" or not. Either way, we should make this explicit in our wiring diagram and marked lined on the plywood.   

Some of the leds were "stuck on" and so we had to remove them. We should consider using any leftover leds at the end of the strand as "replacements" and hard-code their placement. We might even consider unpacking the problematically dense leds in the middle of the map to use for this purpose if we have too many dead/stuck leds. 

## Plexiglass Map & Stand

We had just enough room to light the map on three edges, but all four edges would have been better

The bottom left edge, due to the led strand being fed out of the back, would pull the strip away from the edge of the plexiglass, resulting in the lower left quadrant of the map being less well-lit than the other quadrants. In general, we need a better way to affix the strip properly to the edge of the plexiglass so that the map can be consistently well-lit across all quadrants

Because the back of the map wasn't sealed, wind blew in a bunch of playa dust and it caked onto the plexiglass, nearly ruining its visibility. We found this very hard to clean, but perhaps compressed air cannisters (not whipits lol) would be helpful. Also, we should probably seal the chamber between the leds and the plexiglass from playa dust accumulation if possible so this isn't a concern. 

Stand needs to be tied down-- a wind storm blew it over before we tied it down. 

If the map isn't at eye level, the plexiglass road etchings will not visually line up with the led's-- I found myself needing to crouch to figure out where the dots on the map were actually located with respect to the etchings. The map would only need to be moved up maybe 2 feet to be at eye level. 

## Power & Data wiring

8 LED strands (50 LEDs each) were fed into one of three 12V lines on the solar battery using 3-pin jst to dc connectors and then down to one of three 12V power rails via dc splitters (two 3-to-1 splitters and one 2-to-1 splitter). I used the "end" of each strand (the side with data out) to connect power, while the side with "data in" was connected directly to the fadecandy. Jst-to-dc connectors were pretty janky, using screwed terminal blocks to make the connections-- these would often come loose and require re-screwing. In the future, we might want to actually solder our own converters and use heat shrink to cover the solder joints.

The ws2801b strip lighting the plexiglass etchings was powered by the 5V usb line on the solar battery using a janky jst to micro-usb connector and a usb cord. The data-in side was connected to a simple lithium battery powered microcontroller running code to power on all the lights. Once the ws2801b were given the command to light up, I could disconnect the microcontroller. Note that the jst connector I used had its power line cut, but if I preserved this power line, I could feed it directly to the microcontroller's 5v VIN and not need a separate lithium battery to power it

It might be nice to do some actual power consumption tests here. For example, I'd be curious to know how long we would be able to run the map off of a fully charged 40Ah battery.    
        
## Electronics

Software stack: fadecandy led controller + computer with fadecandy server and processing 

Hardware: used an old macbook (not ideal) via the inverter on the solar battery. Too busy before burn to fully set up embedded GPU box (Tegra X2). Had a working raspberry pi with processing+fadecandy server, but haven't yet tested marauder's map processing code-- it might not be able to run visualizations at full framerates (30 fps). Ideally, we will use the Tegra X2 next year, which we will be able to run 24/7 (it uses about 3-5 Watts and can be run from a 12V line). 

## LoRa

Used a 1W LoRa transceiver with a long antenna (not sure the dB rating on it, it was provided by the manufacturer of the LoRa boards)

It might make sense to try to put the antenna as high as possible in order to increase the range at which it can receive LoRa data packets. This will be especially true if we ever move to a star topology and let the base station handle message passing / forwarding to other nodes. 
        
## Software

Processing provides a nice environment to be able to develop visualizations without actually needing access to the LED map (the pixel map is abstracted away by the software). I would love to field new visualizations from talented people (Emily? Luca? Josh?) that we can cycle through (using something like an rf remote or a physical button/dial). I will post the current code on a repo so that people can make their own visualizations. I will also provide the data logs from last year and a script to play them back in order to create a realistic simulation of incoming map data.

Processing is built on top of a custom JRE and requires a framebuffer for its visualization engine. This is a problem when no monitor is connected to the computer (as would be in the case of a raspberry pi or tegra x2)-- thus, to run processing "headless" one must create a virtual framebuffer. I found that I could easily run processing headless using xvfb (x virtual framebuffer) and xvfb-run on the raspberry pi, but had difficulty setting processing and java up on the tegra. Next year we need to make sure that the Tegra platform can support processing and virtual frame buffers before arriving on playa :P  

