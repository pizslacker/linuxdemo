# linuxdemo

A little excercise for myself, creating an optimized demo binary showing off classic Amiga demoscene-like showcase.

C program using SDL2 that implements the classic "Doom" pixel fire algorithm, parrallax text scrolling banner and a moving starfield background.

It creates an internal framebuffer (an array of pixels), updates the fire logic by seeding heat at the bottom and spreading it upward with randomized decay, and renders it to an SDL2 texture.

![linuxdemo](https://github.com/pizslacker/linuxdemo/blob/main/images/linuxdemo.png)
