
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help assemblehdr

-assemblehdr takes a volume in which each frame is a linear luminance image
taken at a different exposure, and compiles them all into a single HDR image,
gracefully handling oversaturated regions.

If exposure values are known, they can be given in increasing frame order.
Otherwise, assemblehdr attempts to discover the exposure ratios itself, which
may fail if there are very few pixels that are properly imaged in multiple
frames. For best results, load the frames in either increasing or decreasing
exposure order.

Usage: ImageStack -loadframes input*.jpg -gamma 0.45 -assemblehdr -save out.exr
   or  ImageStack -loadframes input*.jpg -gamma 0.45 -assemblehdr 1.0 0.5 0.1
                  -save output.exr

```