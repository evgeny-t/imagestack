
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help multiply

-multiply multiplies the top image in the stack by the second image in the
stack. If one or both images are single-channel, then this performs a
scalar-vector multiplication. If both images have multiple channels, there are
three different vector-vector multiplications possible, selectable with the
sole argument. Say the first image has k channels and the second image has j
channels. "elementwise" performs element-wise multiplication. If k != j then
the lesser is repeated as necessary. The output has max(k, j) channels. "inner"
performs an inner, or matrix, product. It treats the image with more channels
as containing matrices in row-major order, and the image with fewer channels as
a column vector, and performs a matrix-vector multiplication at every pixel.
The output has max(j/k, k/j) channels. "outer" performs an outer product at
each pixel. The output image has j*k channels. If no argument is given, the
multiplication will be "elementwise".

Usage: ImageStack -load a.tga -load b.tga -multiply -save out.tga.
```