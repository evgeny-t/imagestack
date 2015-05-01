
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help warp


-warp treats the top image of the stack as indices (within [0, 1]) into the
second image, and samples the second image accordingly. It takes no arguments.
The number of channels in the top image is the dimensionality of the warp, and
should be three or less.

Usage: ImageStack -load in.jpg -push -evalchannels "X+Y" "Y" -warp -save out.jpg

```