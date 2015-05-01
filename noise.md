
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help noise

-noise adds uniform noise to the current image, uncorrelated across the
channels, in the range between the two arguments. With one argument, the lower
value is assumed to be zero. With no arguments, the range is assumed to be [0,
1]

Usage: ImageStack -load a.tga -push -noise -add -save anoisy.tga

```