
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fastblur

-fastblur takes a floating point frames, width, and height, and performs a fast
approximate gaussian blur with those standard deviations using the IIR method
of van Vliet et al. If given only two arguments, it performs a blur in x and y
only. If given one argument, it performs the blur in x and y with filter width
the same as height.

Usage: ImageStack -load in.jpg -fastblur 5 -save blurry.jpg

```