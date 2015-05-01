
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help sort

-sort sorts the data along the given dimension for every value of the other
dimensions. For example, the following command computes the median frame of a
video.

ImageStack -loadframes frame*.jpg -sort t -crop frames/2 1 -save median.jpg

```