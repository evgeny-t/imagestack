
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help jointbilateral

-jointbilateral blurs the top image in the second without crossing boundaries
in the second image in the stack. It takes up to five arguments: the color
standard deviation of the filter, the standard deviations in width, height, and
frames, and the method to use (see -gausstransform for a description of the
methods). If the method is omitted it automatically chooses an appropriate one.
Temporal standard deviation defaults to zero, and standard deviation in height
defaults to the same as the standard deviation in width

Usage: ImageStack -load ref.jpg -load im.jpg -jointbilateral 0.1 4
```