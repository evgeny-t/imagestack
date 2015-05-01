
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help paste

-paste places some of the second image in the stack inside the top image, at
the specified location. -paste accepts two or three, six, or nine arguments.
When given two or three arguments, it interprets these as x and y, or x, y,
and t, and pastes the whole of the second image onto that location in the first
image. If six or nine arguments are given, the latter four or six arguments
specify what portion of the second image is copied. The middle two or three
arguments specify the top left, and the last two or three arguments specify
the size of the region to paste.

The format is thus: -paste [desination origin] [source origin] [size]

Usage: ImageStack -load a.jpg -push 820 820 1 3 -paste 10 10 -save border.jpg

```