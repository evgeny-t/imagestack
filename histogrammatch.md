
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help histogrammatch

-histogrammatch alters the histogram of the current image to match that of the
second image, while preserving ordering. Performing any monotonic operation to
an image, and then histogram matching it to its original should revert it to
its original.

Usage: ImageStack -load a.tga -load b.tga -histogrammatch -save ba.tga

```