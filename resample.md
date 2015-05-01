
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help resample

-resample resamples the input using a 3-lobed Lanczos filter. When given three arguments, it produces a new volume of the given width, height, and frames. When given two arguments, it produces a new volume of the given width and height, with the same number of frames.

Usage: ImageStack -loadframes f*.tga -resample 20 50 50 -saveframes f%03d.tga

```