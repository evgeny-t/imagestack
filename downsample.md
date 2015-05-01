
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help downsample

-downsample divides the width, height, and frames of the current image by the
given integer arguments. It averages rectangles to get the new values.

-downsample x y is interpreted as -downsample x y 1
-downsample x is interpreted as -downsample x x 1
-downsample is interpreted as -downsample 2 2 1

Usage: ImageStack -load a.tga -downsample 3 2 -save b.tga

```