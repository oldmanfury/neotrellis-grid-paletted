# neotrellis-grid-paletted
Teensy code from okyeron modified with gradient color palettes.  Boots up with all 8 available palettes showing - push a button on one to select.  

Leftmost column selects overall brightness - watch the Fates LED go out if you draw too much current, then either back down, or count on a patch not lighting up as many LEDs as the palette selector.

Lower right hand corner three button press gets you back to selection now.  May do weird things if Fates is spewing data while selecting - I recommend pausing the patch if possible.

I used an eyedropper program to get RGB values from these palettes:
https://matplotlib.org/tutorials/colors/colormaps.html

Feel free to change / add your own.

Original code here:
https://github.com/okyeron/neotrellis-monome/tree/master/neotrellis_monome_teensy
