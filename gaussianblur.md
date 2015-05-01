
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help gaussianblur

-gaussianblur takes a floating point width, height, and frames, and performs a
gaussian blur with those standard deviations. The blur is performed out to
three standard deviations. If given only two arguments, it performs a blur in x
and y only. If given one argument, it performs the blur in x and y with filter
width the same as height.

Usage: ImageStack -load in.jpg -gaussianblur 5 -save blurry.jpg

```