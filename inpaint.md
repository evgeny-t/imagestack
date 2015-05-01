
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help inpaint


-inpaint takes the image on the top of the stack, and a one channel mask of the
same size second on the stack, and diffuses areas of the image where the mask is
high into areas of the image where the mask is low. Image pixels with mask of 1
are unchanged.

Usage: ImageStack -push 1 640 480 1 -eval "(X > 0.5)*(X < 0.6)" -load in.jpg
                  -inpaint -save out.jpg

```