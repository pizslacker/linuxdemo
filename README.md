# linuxdemo

A little excercise for myself, creating an optimized demoscene binary (`24KB`), showing off classic Amiga demoscene-like graphics, **On Linux**.

Made with C using SDL2 that implements the classic "Doom" pixel fire algorithm, parrallax text scrolling banner and a moving star trails background.

It creates an internal framebuffer (an array of pixels), updates the fire logic by seeding heat at the bottom and spreading it upward with randomized decay, and renders it to an SDL2 texture.
For the spherical text scroller, we iterate over the string and map each character's index to a circular path using sin(theta) and cos(theta).
It uses Bresenham's Line Algorithm for Star Trails.

It also emulates CRT scanlines for oldschool feels' :P Right before blasting the buffer to the GPU, we loop over every even row in the frameBuffer and bit-shift/divide the RGB channels by half, creating dark interlaced lines.

![linuxdemo](https://github.com/pizslacker/linuxdemo/blob/main/images/linuxdemo.png)
