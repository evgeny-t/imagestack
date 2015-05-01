
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help lanczosblur

-lanczosblur convolves the current image by a three lobed lanczos filter. A
lanczos filter is a kind of windowed sinc. The three arguments are filter
width, height, and frames. If two arguments are given, frames is assumed to be
one. If one argument is given, it is interpreted as both width and height.

Usage: ImageStack -load big.jpg -lanczosblur 2 -subsample 2 2 0 0 -save
small.jpg

```