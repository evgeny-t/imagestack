
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help bilateralsharpen


-bilateralsharpen sharpens using a bilateral filter to avoid ringing. The
three arguments are the spatial and color standard deviations, and the sharpness.
A sharpness of zero has no effect; a sharpness of 1 is significant.

Usage: ImageStack -load input.jpg -bilateralsharpen 1.0 0.2 1 -save sharp.jpg

```