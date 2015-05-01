
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help equalize

-equalize flattens out the histogram of an image, while preserving ordering
between pixel brightnesses. It does this independently in each channel. When
given no arguments, it produces an image with values between zero and one. With
one argument, it produces values between zero and that argument. With two
arguments, it produces values between the two arguments. The brightest pixel(s)
will always map to the upper bound and the dimmest to the lower bound.

Usage: ImageStack -load a.tga -equalize 0.2 0.8 -save out.tga

```