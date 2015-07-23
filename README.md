Newman by Brian Jackson
=======================

Contents
--------

1. Acknowledgements
2. Installation
3. Usage: fractal viewer
4. Usage: palette editor
5. Contact information

1. Acknowledgements
-------------------

This program uses both the perturbation theory and series approximation methods pioneered by K. I.
Martin and implemented in his SuperFractalThing software. It also uses a variation on the "multiwave"
palette generation method invented by Pauldelbrot.

2. Installation
---------------

This program depends on [Magick++](http://www.imagemagick.org/Magick++/), [SDL2](http://libsdl.org), [FreeType](http://freetype.org), and
[libbyteimage](http://github.com/axnjaxn/libbyteimage) (which must be compiled with support for the other libraries).

If you have a make-compatible build system installed with support for C++11, you can execute `make` to
build the software. Currently, it looks for its font, FreeSans (from GNU FreeFont) in the res
subdirectory. Yes, eventually I'll implement a real "install" target. No, it's not ready now.

You can run the program by navigating to the newman directory and executing `./newman`

3. Usage: fractal viewer
------------------------

Left click and drag to zoom in - holding shift while dragging will zoom out.

Right click and drag to shift.

Pressing escape will terminate any kind of render activity.

Other keys:

Backspace - Reset view window and iteration count

Return - Force rerender at current settings

Up arrow - Increase iteration count by 256

Down arrow - Decrease iteration count by 256

F2 - Save view window and iteration settings

F3 - Load view window and iteration settings

F5 - Force display refresh

F11 - Screenshot (saved in working directory)

1 - Disable multisampling

2 - 2x multisampling

3 - 3x multisampling

4 - 4x multisampling

B - Start a beauty render

D - Toggle line-by-line preview

I - Set iteration count

P - Switch to palette editor

S - Toggle smoothing

Z - Create zoom video centered on current location

4. Usage: palette editor
------------------------

The palette editor represents three types of "cycles" that, together, generate the palette of colors
used to render the fractal. Changing the cycles re-indexes the palette, but doesn't require a full
re-render of the preview shown in the fractal viewer.

The top rows of colors represent the hue cycles. Each row is one cycle, with the number representing
the number of iterations in one full cycle. Press the left minus or plus icons to delete the cycle or
add a following cycle, respectively. The minus and plus icons on the right will remove or add one hue
in the cycle, respectively. The number underneath the hue cycles represents the period of a cycle that
rotates between each row of hues when generating colors. You can click the colors to adjust hue (using
the slider at the bottom of the window) or any of the numbers, to adjust their values.

The row of green squares below the hue cycles represents the saturation bias. You can adjust them by
clicking any of the icons and adjusting the slider, or clicking the number to adjust the period.

Finally, the black-to-white squares under the saturation cycle represent luminance waves. These are
sinusoidal functions that brighten or darken the colors. You can adjust the intensity and period in
the same way as the hue and saturation values, but luminance adds asymptotically (using a logistic
curve) so that it's impossible to clip past white or black.

You can use the following keys to interact with the palette editor:

F2 - Save the current palette

F3 - Load a new palette

P - Close the palette editor and return to the fractal viewer

5. Contact information
----------------------

This software was developed by Brian Jackson (axnjaxn AT axnjaxn DOT com), and is currently hosted on
GitHub. Feel free to email Dr. Jackson if you have any questions or comments.