
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help convolve

-convolve takes a width, height, and frames and a single-channel 3D kernel
specified across the rows, then down the columns, then over time, and convolves
the current image by that matrix independently in each channel.

With no numeric arguments, -convolve will use the next image on the stack as
the filter.

Boundary conditions can be specified by appending the argument "zero",
"homogeneous", "clamp", or "wrap", which result in the respective assumptions:
the image is zero outside the boundary; the image is weighted with weight one
inside the boundary, and weight zero outside the boundary; the image values
outside the boundary clamp to the nearest defined image value (Neumann); and
the image wraps outside the boundary.

Convolution by multi-channel filters is poorly defined, because it requires a
vector-vector multiplication between filter values and image values. By
specifying a final argument of "inner", "outer", or "elementwise", the
multiplication used is correspondingly the inner product (or matrix product if
the image and kernel have a differing number of frames); the outer product; or
an elementwise product. If the kernel has k channels and the image has m
channels, "inner" produces an image with max(m/k, k/m) channels, "outer"
produces an image with m*k channels, and "elementwise" requires that m==k and
produces an image with the same number of channels. The default method is
"outer".

Taking a horizontal gradient with zero boundary condition: 
 ImageStack -load a.tga -convolve 2 1 1  -1 1 zero -save dx.tga
Convolving by a bank of filters: 
 ImageStack -load bank.tmp -load a.tga -convolve homogeneous outer
```