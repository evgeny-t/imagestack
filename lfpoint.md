
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help lfpoint


-lfpoint colors a single 3d point white in the given light field. The five
arguments are the light field u, v, resolution, and then the x, y, and z
coordinates of the point. x and y should be in the range [0, 1], while z
is disparity. z = 0 will be at the focal plane.

Usage: ImageStack -load lf.exr -lfpoint 16 16 0.5 0.5 0.1 -save newlf.exr

```