# linuxdemo

A little excercise for myself, creating an optimized demoscene binary (`20KB`), showing off classic Amiga demoscene-like graphics.

Made with C using SDL2 that implements the classic "Doom" pixel fire algorithm, parrallax text scrolling banner and a moving star trails background.

It creates an internal framebuffer (an array of pixels), updates the fire logic by seeding heat at the bottom and spreading it upward with randomized decay, and renders it to an SDL2 texture.
For the spherical text scroller, we iterate over the string and map each character's index to a circular path using sin(theta) and cos(theta).
It uses Bresenham's Line Algorithm for Star Trails.

![linuxdemo](https://github.com/pizslacker/linuxdemo/blob/main/images/linuxdemo.png)
