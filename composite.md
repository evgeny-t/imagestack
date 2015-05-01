
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help composite


-composite composites the top image in the stack over the next image in
the stack, using the last channel in the top image in the stack as alpha.
If the top image in the stack has only one channel, it interprets this as
a mask, and composites the second image in the stack over the third image
in the stack using that mask.

Usage: ImageStack -load a.jpg -load b.jpg -load mask.png -composite
       ImageStack -load a.jpg -load b.jpg -evalchannels [0] [1] [2] \
       "x>width/2" -composite -display

```