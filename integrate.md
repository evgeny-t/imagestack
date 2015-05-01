
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help integrate


-integrate computes partial sums along the given dimension. It is the
of the -gradient operator. Multiply dimensions can be given as arguments,
for example -integrate x y will produce a summed area table of an image.
Allowed dimensions are x, y, or t.

Warning: Don't expect to integrate more than twice and be able to get back
the image by differentiating. Numerical errors will dominate.

Usage: ImageStack -load a.tga -gradient x y -integrate y x -save a.tga

```