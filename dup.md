
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help dup

-dup duplicates an image and pushes it on the stack. Given no argument it
duplicates the top image in the stack. Given a numeric argument it duplicates
that image from down the stack. Given a string argument it duplicates an image
that was stashed with -stash using that name

Usage: ImageStack -load a.tga -dup -scale 0.5 -save a_small.tga
                  -pop -scale 2 -save a_big.tga

```