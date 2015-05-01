
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help percentilefilter

-percentilefilter selects a given statistical percentile over a circular support
around each pixel. The two arguments are the support radius, and the percentile.
A percentile argument of 0.5 gives a median filter, whereas 0 or 1 give min or
max filters.

Usage: ImageStack -load input.jpg -percentilefilter 10 0.25 -save dark.jpg

```