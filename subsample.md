
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help subsample


-subsample subsamples the current image. Given two integer arguments, a and b,
it selects one out of every a frames starting from frame b. Given four arguments,
a, b, c, d, it selects one pixel out of every axb sized box, starting from pixel
(c, d). Given six arguments, a, b, c, d, e, f, it selects one pixel from every
axbxc volume, in the order width, height, frames starting at pixel (d, e, f).

Usage: ImageStack -load in.jpg -subsample 2 2 0 0 -save smaller.jpg

```