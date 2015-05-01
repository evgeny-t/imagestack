
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help frametiles


-frametiles takes a volume where each frame is a grid and breaks each frame
into multiple frames, one per grid element. The two arguments specify the grid
size. This operation is the inverse of tileframes.

Usage: ImageStack -loadframes sheet*.tif -frametiles 5 5 -saveframes frame%d.tif

```