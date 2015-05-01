
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help locallaplacian

-locallaplacian modifies contrast at various scales. It is similar to the
clarity slider in Photoshop. This operation is an implementation of Fast and
Robust Pyramid-based Image Processing by Aubry et al, which is an acceleration
of Local Laplacian Filters by Paris et al.

The first argument specifies how much of an effect to apply. 0 gives no effect,
1 produces a moderate amount of contrast enhancement, and -1 produces a
piecewise flattening. The second argument specifies how the effect should
change with respect to scale. 0 applies the same effect at all scales, -1
applies the effect only at coarse scales, and 1 applies the effect only at fine
scales. Values larger than one actually reverse the effect at fine or coarse
scales. In fact -locallaplacian 1 2 is an effective tone-mapper because it
amplifies contrast at fine scales and reduces it at coarse scales.

Usage: ImageStack -load input.jpg -locallaplacian 1 0 -save boosted.jpg
```