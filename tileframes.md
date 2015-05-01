
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help tileframes


-tileframes takes a volume and lays down groups of frames in a grid, dividing
the number of frames by the product of the arguments. It takes two arguments,
the number of old frames across each new frame, and the number of frames down.
each new frame. The first batch of frames will appear as the first row of the.
first frame of the new volume.

Usage: ImageStack -loadframes frame*.tif -tileframes 5 5 -saveframes sheet%d.tif

```