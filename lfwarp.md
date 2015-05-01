
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help lfwarp


-lfwarp treats the top image of the stack as indices (within [0, 1]) into the
lightfield represented by the second image, and samples quadrilinearly into it.
The two arguments it takes are the width and height of each lenslet.
The number of channels in the top image has to be 4, with the channels being
the s,t,u and v coordinates in that order.
An extra argument of 'quick' at the end switches nearest neighbor resampling on
Usage: ImageStack -load lf.jpg -load lfmap.png -lfwarp 8 8 -save out.jpg

```