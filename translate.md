
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help translate


-translate moves the image data, leaving black borders. It takes two or three
arguments. Two arguments are interpreted as a shift in x and a shift in y.
Three arguments indicates a shift in x, y, and t. Negative values shift to the
top left and positive ones to the bottom right. Fractional shifts are
permitted; Lanczos sampling is used in this case.

Usage: ImageStack -load in.jpg -translate -10 -10 -translate 20 20
                  -translate -10 -10 -save in_border.jpg

```