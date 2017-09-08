# Talisman

## brain enclosure

I had trouble using the usb port window effectively since the usb port wasn't affixed to the side of the enclosure in any way. I would therefore have to take out the device to charge it, exposing it to dust. This also made using the screws for the lid too cumbersome to use. I therefore just wrapped the lips on with electrical tape. Some possible solutions:
1) Make the port hole much bigger so the fit of the device inside its enclosure isn't so important
2) Find a way to affix the usb port to the enclosure so that it can't be pushed around when trying to connect it to a cable
3) Use a small strain-relieved usb cable dongle that hangs off the enclosure at all times

It might be nice to have some sort of structure on it for the purpose of attaching a caribiner or chain/thread/strap. Most people either used zip ties or just placed the brain into a pouch on their belt or a breast pocket. 

It might be nice to try to miniaturize the brain to fit inside the talisman itself

## talisman enclosure

We could benefit from longer dust-proof cabling so that the brain could be further from the talisman

Our use of both 16-led and 12-led rings made it so the middle ring was always partially obscured by the upper (smallest) ring. We might want to consider designing our own custom IC for the talisman LEDs so that we can control our LED placement better and also better line up the leds to our translucent printed map. 

## GPS Module

I used the U-Blox neo6mv2 gps module, on a board with a built-in ceramic antenna usb-to-serial chip. The supplied software for setting firmware (u-center) is awful and, for some reason, my firmware settings would disappear after a few days and go back to the default. In the future, we should probably write an initialization routine in our arduino sketch that checks the firmware settings, and if not set correctly, resets them to our custom settings. To do this, we can either study the datasheet or we can snag the serial data coming from u-center and put it directly in the arduino code. I couldn't find any pre-made arduino packages for setting u-blox firmware, but it might be worth another googling. One note though is that if we wish to go the route of setting configuration in code, we will need to connect the recv line on the gps module, which will cost us another I/O pin.

## software

#### party mode

it might be nice to have a way for user customization (let long button presses speed up and slow down animations, let double taps freeze current animation)

it might be cool to embed information about location and/or nearby friends into the party mode (faster animation = more friends nearby, brighter animation = at the man, etc)

#### diagnostic mode

battery life was really valuable here, as was the readout of nearby active talisman color codes / device IDs. The # of active satellites was slightly less useful (at burning man, this was typically extremely high), and the animation that data was being sent was actually not very useful (I have no way of knowing in software that a message is actually being sent, only that I am pushing bits out the appropriate serial line). 

it would be great if diagnostic mode could more easily inform the user of their own device ID (counting the LEDs is hard when you don't know where the "zero" LED is)

#### lights-off / battery-saver mode

I need to do some actually power consumption tests to figure out how much juice the LEDs use relative to the overall power consumption. It seems that battery-saver mode actually didn't keep the device running that much longer, which points to the GPS, LoRa, and WEMOS as the main consumers of power. In the future, it might be useful to have a battery saver mode that not only turns off the leds, but perhaps modulates other functions that consume power (slow down the GPS update rate, slow down the LoRa transmission frequency, etc). 

#### mini-marauder's map mode

I only used the inner and outermost rings and skipped the middle ring. Utilizing the middle ring would give us 50% more resolution in terms of distance from the man, which seems like a big benefit, but I had reasons for not using it.... I originally didn't use the middle ring because, being partially obscured by the upper ring, it was slightly hard to see which LED was lit, which I thought might result in user confusion. Furthermore, being 16 led's, each led represented a 45 minute slice, making the mental math required to localize oneself a bit harder (the outer ring was 24 leds and thus provided nice 30 minnute increments and the inner ring was 12 leds, providing nice 1 hour increments). Finally, I was just being lazy and coded up the marauder's map mode in about 10 minutes and the reverse addressing on the middle ring was something I didn't want to think about at the time. With just two distances available, I broke it up as follows:
  1) inner ring: man to esplanade (inner playa)
  2) outer ring: A to L (city and deep playa)

One side effect of this split is that really, the only time one could locate someone with significant accuracy was the exact moment when their dot jumped from the outer to inner ring since this would localize them to the exact boundary between A and esplanade. Otherwise, the distance from the man to esplanade and the distance from A to L was too coarse to be all that useful (without a "zoom in" mode, that is). 

The angular distance, however, was much better resolution (12 and 24 angular divisions as opposed to just 2 radial distances). However, knowing someone is "between 1:15 and 1:45 and in deep playa" is still of limited usefulness as this still covers a good bit of ground. 

it was pointed out that a "zoom-in" mode would be incredibly useful when you land near or on top of someone's dot. The proposed solution would put your own dot at the center of the map and then show the nearby friends' dots relative to you. This won't work without a static frame of reference, so it was also proposed to put always put a special "man dot" on the map as well. Thus, given the "man dot", if you orient yourself such that the man is directly in front of you, and then turn the talisman such that the "man dot" is directly in front of your dot, you now can read off the proper direction to travel to reach your friend. The man dot could always be placed on the middle ring, leaving the inner and outer rings to be used to find friends (outer ring could be >50m from you and inner ring could be <50m from you, for example.) 

Being on someone's dot: cycling through colors when there are many talisman on the same dot can make for difficulties distinguishing colors. We might want to explore different types of visualizations for showing when multiple talismans occupy the same space (or perhaps the zoom-in hot/cold mode will work to disambiguate this visual ambiguity. 
