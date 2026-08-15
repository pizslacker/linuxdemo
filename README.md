# linuxdemo

A little excercise for myself creating a demoscene'ish program, showing off classic Amiga demoscene-like graphics, **On Linux**.

Made with C using SDL2 that implements the classic "Doom" pixel fire algorithm, parrallax text scrolling banner and a moving star trails background.

#### What:
- optimized demoscene binary (`24KB`, 4KB more than `LICENSE` file).
- 20KB soundbyte loop (courtesy of [**k!M**](https://soundcloud.com/kim-olsen-357297567)), making the total size 44KB!

#### How:
- It creates an internal framebuffer (an array of pixels), updates the fire logic by seeding heat at the bottom and spreading it upward with randomized decay, and renders it to an SDL2 texture.
- For the spherical text scroller, it iterates over the string and map each character's index to a circular path using sin(theta) and cos(theta).
- It uses Bresenham's Line Algorithm for parrallax star trails.
- It also emulates CRT scanlines for oldschool feels' :P Right before blasting the buffer to the GPU, we loop over every even row in the frameBuffer and bit-shift/divide the RGB channels by half, creating dark interlaced lines.

![linuxdemo](https://github.com/pizslacker/linuxdemo/blob/main/images/linuxdemo.png)
