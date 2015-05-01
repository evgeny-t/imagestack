
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help reshape


-reshape changes the way the memory of the current image is indexed. The four
integer arguments specify a new width, height, frames, and channels.

Usage: ImageStack -load movie.tmp -reshape width height*frames 1 channels
                  -save filmstrip.tmp
```