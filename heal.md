
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help heal

-heal takes an image and a mask, and reconstructs the portion of the image where the mask is high using patches from the rest of the image. It uses the patchmatch algorithm for acceleration. The arguments include the number of iterations to run per scale, and the number of iterations of patchmatch to run. Both default to five.

Usage: ImageStack -load mask.png -load image.jpg -heal -display
```