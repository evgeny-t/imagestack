
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help demosaic


-demosaic demosaics a raw bayer mosaiced image camera. It should be a one
channel image. The algorithm used is adaptive color plane interpolation (ACPI).
Demosaic optionally takes two or three arguments. Two arguments specify an offset
of the standard bayer pattern in x and y. The presence of a third argument
indicates that auto-white-balancing should be performed.

Usage: ImageStack -load photo.dng -demosaic -save out.png
       ImageStack -load raw.yuv -demosaic 0 1 awb -save out.png
```