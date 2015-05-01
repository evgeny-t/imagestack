
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help loadframes


-loadframes accepts a sequence of images and loads them as the frames of a
single stack entry. See the help for -load for details on file formats.

-loadframes cannot be used on raw float files. To achieve the same effect, cat
the files together and load them as a single multi-frame image.

Usage: ImageStack -loadframes foo*.jpg bar*.png

```