
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help poisson

-poisson assumes the stack contains gradients images in x and y, and attempts
to find the image which fits those gradients best in a least squares sense. It
uses a preconditioned conjugate gradient descent method. It takes one argument,
which is required RMS error of the result. This defaults to 0.01 if not given.

Usage: ImageStack -load dx.tmp -load dy.tmp 
                  -poisson 0.0001 -save out.tga

```