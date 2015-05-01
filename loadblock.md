
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help loadblock

-loadblock loads a rectangular portion of a .tmp file. It is roughly equivalent
to a load followed by a crop, except that the file need not fit in memory. The
nine arguments are filename, x, y, t, and c offsets within the file, then
width, height, frames, and channels. If seven arguments are given, all channels
are loaded. If five arguments are given, all frames are used and the arguments
specify x and y. If three arguments are given, they specify frames and all x,
y, and channels are loaded. Loading out of bounds from the tmp file is
permitted. Undefined areas will be zero-filled.

This example multiplies a 512x512x128x3 volume by two, without ever loading it
all into memory:
ImageStack -loadblock foo.tmp 0 0 0 0 512 512 64 3 \
           -scale 2 -saveblock foo.tmp 0 0 0 0
ImageStack -loadblock foo.tmp 0 0 0 64 512 512 64 3 \
           -scale 2 -saveblock foo.tmp 0 0 64 0

```