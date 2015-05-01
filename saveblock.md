
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help saveblock

-saveblock overwrites a rectangular subblock of a .tmp file with the top of the
stack. It is logically similar to a load, paste, save combination, but never
loads the full tmp file. The five arguments are the filename, followed by the
offset at which to paste the volume in x, y, t, and c. When given four
arguments c is assumed to be zero. When given three arguments t is also set to
zero. With two arguments, x, y, and c are set to zero.

This example multiplies a 128x512x512x3 volume by two, without ever loading it
all into memory:
ImageStack -loadblock foo.tmp 0 0 0 0 64 512 512 3 \
           -scale 2 -saveblock foo.tmp 0 0 0 0
ImageStack -loadblock foo.tmp 64 0 0 0 64 512 512 3 \
           -scale 2 -saveblock foo.tmp 0 0 0 0

```