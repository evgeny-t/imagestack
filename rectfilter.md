
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help rectfilter

-rectfilter performs a iterated rectangular filter on the image. The four
arguments are the filter width, height, frames, and the number of iterations.
If three arguments are given, they are interpreted as frames, width, and
height, and the number of iterations is assumed to be one. If two arguments are
given they are taken as width and height, and frames is assumed to be one. If
one argument is given it is taken as both width and height, with frames and
iterations again assumed to be one.

Usage: ImageStack -load in.jpg -rectfilter 1 10 10 -save out.jpg

```