
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help align

-align warps the top image on the stack to match the second image on the
stack. align takes one argument, which must be "translate", "similarity", 
"affine", "perspective", or "rigid" and constrains the warp to be of that
type.

Usage: ImageStack -load a.jpg -load b.jpg -align similarity \
                  -add -save ab.jpg

```