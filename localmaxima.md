
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help localmaxima

-localmaxima finds local maxima in the image and outputs their locations to a
text file. Each line in the text file consists of four comma-delimited floating
point values, corresponding to the t, x and y coordinates, and the strength of
the local maxima (its value minus the maximum neighbor). -localmaxima will only
operate on the first channel of an image. There are three arguments. The first
is some string containing the characters x, y, and t. It specifies the
dimensions over which a pixel must be greater than its neighbors. The second is
the minimum value by which a pixel must exceed its neighbors to count as a
local maximum. The third is the minimum distance which must separate adjacent
local maxima.

Usage: ImageStack -load stack.tmp -localmaxima txy 0.01 5 output.txt
```