# neotrellis-grid-paletted

![palette-selector](https://user-images.githubusercontent.com/2180300/127749178-a6e435ef-fbd9-45fa-9426-13e7f2d1fdea.jpg)

Modified teensy code originally from https://github.com/okyeron that allows selection of various gradient color palettes.  Boots up with 24 available palettes over three pages - push a button within a row to select.  Lower-left corner button changes palette page (currently 3 available).  These color palettes make it easier to distinguish functional blocks, see a grid in daylight, and hopefully some may even help people with color-blindness.

Leftmost column selects overall brightness - watch the Fates red power LED go out if you draw too much current, then either back down, or count on a patch not lighting up as many LEDs as the palette selector.  I found that even my original green-board teletype has enough current to drive these palettes.

Lower right hand corner three button press gets you back to selection now.  

To generate the palettes, I first used an eyedropper program to get RGB values from these palettes:
https://matplotlib.org/tutorials/colors/colormaps.html

Then I got lazy and wrote a macro in Igor pro to output 16 more color palettes.  Feel free to change / add your own.

Original code and installation instructions here:
https://github.com/okyeron/neotrellis-monome/tree/master/neotrellis_monome_teensy
