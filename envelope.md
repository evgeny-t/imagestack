
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help envelope

-envelope computes a lower or upper envelope of the input, which is smooth, and
less than (or greater than) the input. The first argument should be "lower" or
"upper". The second argument is the desired smoothness, which should be greater
than zero and strictly less than one. The last argument is the degree of edge
preserving. If zero, the output will be smooth everywhere. Larger values
produce output that is permitted to have edges where the input does, in a
manner similar to a bilateral filter.

Usage: ImageStack -load a.jpg -envelope upper 0.5 1 -display

To locally maximize contrast:
ImageStack -load a.jpg -dup -scale 1.1 -envelope lower 0.9 1 -pull 1
           -subtract -envelope upper 0.9 1 -offset 1 -pull 1 -pull 2
           -add -divide -display
```