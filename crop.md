
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help crop

-crop takes either zero, two, four, or six arguments. The first half of the
arguments are either minimum t, minimum x and y, or all three in the order x,
y, t. The second half of the arguments are correspondingly either number of
frames, width and height, or all three in the order width, height, frames. You
may crop outside the bounds of the original image. Values there are assumed to
be black. If no argument are given, ImageStack guesses how much to crop by
trimming rows and columns that are all the same color as the top left pixel.

Usage: ImageStack -loadframes f*.tga -crop 10 1 -save frame10.tga
       ImageStack -load a.tga -crop 100 100 200 200 -save cropped.tga
       ImageStack -loadframes f*.tga -crop 100 100 10 200 200 1
                  -save frame10cropped.tga

```