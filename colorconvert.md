
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help colorconvert


-colorconvert converts from one colorspace to another. It is called with two
arguments representing these colorspaces.

Allowable colorspaces are rgb, yuv, hsv, xyz, lab and y (luminance alone). grayscale,
gray, and luminance are synonyms for y, and hsb and hsl are synonyms for hsv.

Usage: ImageStack -load a.tga -colorconvert rgb hsv -scale 0.1 1 1
                  -colorconvert hsv rgb -save out.tga

```