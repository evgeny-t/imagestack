
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help histogram

-histogram computes a per-channel histogram of the current image. The first
optional argument specifies the number of buckets in the histogram. If this is
not given it defaults to 256. The second and third arguments indicate the range
of data to expect. These default to 0 and 1.Usage: ImageStack -load a.tga
-histogram -normalize -plot 256 256 3 -display
```