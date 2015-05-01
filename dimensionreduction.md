
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help dimensionreduction

-dimensionreduction takes a dimensionality and projects all points on the image
onto a linear subspace of best fit with that number of dimensions. It is useful
if you know an image should be low dimensional (eg a sunset is mostly shades or
red), and components orthogonal to that dimension are unwanted (eg chromatic
abberation).

Usage: ImageStack -load sunset.jpg -dimensionreduction 2 -save fixed.jpg

```