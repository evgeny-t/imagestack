
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help patchpca3d

-patchpca3d treats local 3D Gaussian neighbourhoods of pixel values as vectors
and computes a stack of filters that can be used to reduce dimensionality and
decorrelate the color channels. The two arguments are the standard deviation of
the Gaussian, and the desired number of output dimensions. Patches near the
edge of the image are not included in the covariance computation.
Usage: ImageStack -load volume.tmp -patchpca3d 2 8 -save filters.tmp
 -pull 1 -convolve zero inner -save reduced.tmp
```