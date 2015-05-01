
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help patchmatch

-patchmatch computes approximate nearest neighbor field from the top
image on the stack to the second image on the stack, using the
algorithm from the PatchMatch SIGGRAPH 2009 paper. This operation
requires two input images which may have multiple frames.
It returns an image with four channels. First three channels 
correspond to x, y, t coordinate of closest patch and 
fourth channels contains the sum of squared differences 
between patches. 

 arguments [numIter] [patchSize]
  - numIter : number of iterations performed. (default: 5)
  - patchSize : size of patch. (default: 7, 7x7 square patch)
 You can omit some arguments from right to use default values.

Usage: ImageStack -load target.jpg -load source.jpg -patchmatch -save match.tmp

```