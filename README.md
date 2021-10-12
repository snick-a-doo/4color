# 4Color
Four colors suffice to produce a map with adjacent regions in different colors, as the famous theorem says. This program is about finding maps that *require* four colors.

It's not hard to come up with maps that require four colors. Here are a few.

![Maps that require four colors](examples/4-color-disks.png)

A bigger challenge is to find a 4-color map that consists of four copies of the same figure translated, rotated, and possibly reflected. In particular, I'm interested in polyomino solutions, using shapes made of square tiles. Here's an example

![A 4-color polyomino map](examples/figure-1.png)

Sure, there's a 5th figure left uncolored in the interior, and a 6th figure that makes up the exterior. But for me, the fun is finding maps where there's a repeated figure that must be shown in four colors because each copy shares an edge with the other three copies.

# Usage
    4color

Clicking on the grid toggles tiles of the focused figure, initially the red figure. The tab key changes the focus. The focused figure is indicated by the color of the grid lines. Currently all manipulation other than adding and removing tiles is done with keys.

Cursor keys
: Move the focused figure.

Page down (up)
: Rotate the focused figure 90° (counter) clockwise.

Shift
: Flip left-right.

Tab
: Focus another figure.

C
: Erase all tiles.

W
: Save a PNG image of the figures to 4color.png. Grid lines and status aren't drawn. The background is transparent.

A status area at the bottom of the window shows if figures

* are contiguous - all tiles joined by edges
* are completely visible - not overlapping
* require four colors

The number of tiles in each figure is also shown.

# TODO
* Figures sometimes shift when toggling.
* Make the controls discoverable.
* External config files for keymap and colors?
* Triangular grid mode.
* Crop exported maps.
* Read in PNG files if they meet certain criteria.
