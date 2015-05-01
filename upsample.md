
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help upsample

-upsample multiplies the width, height, and frames of the current image by the
given integer arguments. It uses nearest neighbor interpolation. For a slower,
high-quality resampling method, use -resample instead.

-upsample x y is interpreted as -upsample x y 1
-upsample x is interpreted as -upsample x x 1
-upsample is interpreted as -upsample 2 2 1

Usage: ImageStack -load a.tga -upsample 3 2 -save b.tga

```