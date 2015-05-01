
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help lffocalstack


-lffocalstack turns a 4d light field into a 3d focal stack. The five arguments
are the lenslet width, height, the minimum alpha, the maximum alpha, and the
step size between adjacent depths (alpha is slope in line space).

Usage: ImageStack -load lf.exr -lffocalstack 16 16 -1 1 0.1 -display

```