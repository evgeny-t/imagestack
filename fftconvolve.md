
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fftconvolve

-fftconvolve performs convolution in Fourier space. It is much faster than
-convolve for large kernels. The two arguments are the boundary condition
(zero, clamp, wrap, homogeneous) and the vector-vector multiplication used
(inner, outer, elementwise). The defaults are wrap and outer respectively. See
-convolve for a description of each option.

Usage: ImageStack -load filter.tmp -load im.jpg -fftconvolve zero inner
```