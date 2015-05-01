
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help opticalflow

-opticalflow computes optical flow between two images based on 
ECCV 2004 paper from Brox et al.(see source code for credits), 
which is performed in multiple scales to handle big displacements.
This operation returns a three-channel image with x and y pixel
offset vectors, and energy cost which can be used as confidence 
measure for the estimation of each vector. 
If you have an initial estimate of flow vectors, you give three
input images and give any value as an argument to indicate you want to
use your initial estimate.

 arguments [useInitialGuess] 
  - useInitialGuess (default: none)

Usage: ImageStack -load target.jpg -load source.jpg -load guess.tmp -opticalflow 1 -save flow.tmp
Usage: ImageStack -load target.jpg -load source.jpg -opticalflow -save flow.tmp

```