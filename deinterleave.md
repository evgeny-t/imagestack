
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help deinterleave

-deinterleave collects every nth frame, column, and/or row of the image and
tiles the resulting collections. When given two arguments it operates on
columns and rows. When given three arguments, it operates on all columns, rows,
and frames.

Usage: ImageStack -load lf.exr -deinterleave 16 16 -save lftranspose.exr

```