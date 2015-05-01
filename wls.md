
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help wls

-wls filters the image with the wls-filter described in the paper
Edge-Preserving Decompositions for Multi-Scale Tone and Detail Manipulation by
Farbman et al. The first parameter (alpha) controls the sensitivity to edges,
and the second one (lambda) controls the amount of smoothing.

Usage: ImageStack -load in.jpg -wls 1.2 0.25 -save blurry.jpg
```