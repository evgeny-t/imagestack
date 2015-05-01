
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help complexdivide

-complexdivide divides the top image in the stack by the second image in the
stack, using 2 "complex" images as its input - a "complex" image is one where
channel 2*n is the real part of the nth channel and channel 2*n + 1 is the
imaginary part of the nth channel. Using zero arguments results in a straight
division (a + bi) / (c + di). Using one argument results in a conjugate
division (a - bi) / (c + di).

Usage: ImageStack -load a.tga -load b.tga -complexdivide -save out.tga.
```