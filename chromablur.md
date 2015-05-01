
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help chromablur


-chromablur blurs an image in the chrominance channels only. It is a good way
of getting rid of chroma noise without apparently blurring the image. The two
arguments are the standard deviation in space and color of the bilateral filter.
Usage: ImageStack -load input.jpg -chromablur 2 -save output.png

```