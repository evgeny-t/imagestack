
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help createtmp

-createtmp creates a zero filled floating point .tmp file of the specified
dimensions. It can be used to create tmp files larger than can fit in memory.
The five arguments are the filename, width, height, frames and channels. If
only four arguments are specified, frames is assumed to be one.

The following example creates a giant volume, and fills some of it with noise:
ImageStack -createtmp volume.tmp 1024 1024 1024 1 \
           -push 256 256 256 1 -noise \
           -saveblock volume.tmp 512 512 512 0 

```