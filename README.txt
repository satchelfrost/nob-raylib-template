video link: https://youtu.be/X6edbokkRrY
student: Reese Gallagher

What was implemented at a high level:

* Drawing the different shapes (i.e. circle, ellipse, square, rectangle, and triangle)
* Rescaling and translation
* Copy (CRTL-C), Cut (CRTL-X), and Paste (CRTL-V)
* New canvas (CRTL-N)
* Saving (CRTL-S), which saves data for the shape program, and a png of the canvas for reference
* Loading/Open, the program is simply run from the command line, passing the save data as the first argument
* Deleting objects (DELETE key, or BACKSPACE key)
* Color wheel, instead of simply 5 colors, there are theoretically 16 million (2\^{}24)

UI Design choices:

1) After drawing the shape, instead of going into select mode immediately, the user continues in draw mode.
   This was based on personal preference.
2) Seamless mode switching. For example, getting into select mode, does not require pressing a button first,
   but instead the user intuitively clicks one of the shapes that they have drawn (i.e. the bounding box of the shape).
3) Minimalism. The first iteration of the program did have buttons for switching between modes, saving, and deleting,
   but ultimately it became clear that many of these were unnecessary. For example, switching between modes was a matter
   of clicking the shape you want to affect, or clicking the canvas, or the color wheel. While technically, these were
   different high-level states (i.e. modes), the user doesn't need to be aware of this. I would argue that copy/cut, paste,
   and save, are so ingrained in the cultural zeitgeist, that it's hardly necessary to explain this. The ctrl-n may be slightly
   cryptic, and perhaps a help dialogue could have been useful.
4) Context aware scroll-wheel. When choosing between shapes, rather than clicking a button, you simply use the scroll
   wheel to hop between different shapes. However, when you are in select mode, the shape movement widget appears, and
   if multiple shapes were clicked, then the scroll wheel can be used to highlight which shape will be moved. Finally,
   when utilizing the color wheel (Hue [0, 360], Saturation [0, 1], and Value [0, 1]), the ``Value'' can be adjusting
   by moving the scroll wheel.
5) There were several prevention mechanisms for scaling the shape too small:
       - A limit so that the scale was no smaller than the widget itself
       - When holding down a scale widget, only that one widget could be affected. Something that could happen if the scale becomes small enough
       - Having a boolean that stays true until the user releases the mouse. This prevents the widget from being detached because
         the collision of the mouse to the widget may no longer be true. Therefore the boolean ensures that it ``sticks'' to the
         mouse until the mouse is released.

Rendering/Technical design choice:

1) Rendering of the shapes was done via a render texture mode. In other words the shape was rendered directly to a texture,
and this texture could then be translated and scaled accordingly. This was done so that shapes like the circle, and 
square, could be distorted after the fact. There was an undesired consequence of rendering at such a low resolution,
namely that aliasing (or sometimes ``jaggies'') became quite evident.

What was not implemented, and other flaws:

1) While saving and loading was implemented, the most glaring feature lacking is the now commonly used ``open file dialogue''.
   The trade off was made for simplicity to simply save the current state upon using the keyboard shortcut CRTL-S (save).
   This was simple and to the point, however some users may find the command line daunting, and this probably isn't the best
   approach for a wide range of users. Additionally, for simplicity the file name saved was simply the date. To mitigate confusion,
   a png screenshot of the canvas was saved with the same name, but this design is likely not as good as being able to specify the name of the file to
   save in an open file dialogue.
2) Despite the scaling prevention mechanisms on the widgets, the shape may still be scaled so small that it's difficult
   to select either scaling or translation. Additionally, if the color was black, the widget icon becomes unreadable. It was
   also possible via copy-paste for a translation widget to be offscreen. These issues could be addressed by having these widgets
   be a little more dynamic in nature.


