
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help gradient


-gradient takes the backward differences in the dimension specified by the
argument. Values outside the image are assumed to be zero, so the first row,
or column, or frame, will not change, effectively storing the initial value
to make later integration easy. Multiple arguments can be given to differentiate
with respect to multiple dimensions in order (although the order does not matter).

Warning: Don't expect to differentiate more than twice and be able to get back
the image by integrating. Numerical errors will dominate.

Usage: ImageStack -load a.tga -gradient x y -save out.tga

```