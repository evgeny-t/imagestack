
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help nlmeans

-nlmeans denoises an image using non-local means, by performing a PCA reduction
on Gaussian weighted patches and then doing a joint-bilateral filter of the
image with respect to those PCA-reduced patches. The four arguments required
are the standard deviation of the Gaussian patches used, the number of
dimensions to reduce the patches to, the spatial standard deviation of the
filter, and the patch-space standard deviation of the filter. Tolga Tasdizen
demonstrates in "Principal Components for Non-Local Means Image Denoising" that
6 dimensions work best most of the time. You can optionally add a fifth
argument that specifies which method to use for the joint bilateral filter (see
-gausstransform).

Usage: ImageStack -load noisy.jpg -nlmeans 1.0 6 50 0.02
```