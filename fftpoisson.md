
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fftpoisson

-fftpoisson computes an image from a gradient field in the same way as -poisson. It interprets the top image on the stack as the y gradient, and the next image as the x gradient. If a single argument is given, it uses that as a weight, and interprets the third image on the stack as a rough target output. The output of this operation will adhere to the target proportionally to the given weight.

Usage: ImageStack -load gx.tmp -load gy.tmp -fftpoisson -display
```