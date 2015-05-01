
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help gradmag

-gradmag computes the square gradient magnitude at each pixel in x and
y. Temporal gradients are ignored. The gradient is estimated using
backward differences, and the image is assumed to be zero outside its
bounds.

Usage: ImageStack -load input.jpg -gradmag -save out.jpg
```