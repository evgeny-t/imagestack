
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help bidirectionalsimilarity

-bidirectionalsimilarity reconstructs the top image on the stack using patches
from the second image on the stack, by enforcing coherence (every patch in the
output must look like a patch from the input) and completeness (every patch
from the input must be represented somewhere in the output). The first argument
is a number between zero and one, which trades off between favoring coherence
only (at zero), and completeness only (at one). It defaults to 0.5. The second
arguments specifies the number of iterations that should be performed, and
defaults to five. Bidirectional similarity uses patchmatch as the underlying
nearest-neighbour-field algorithm, and the third argument specifies how many
iterations of patchmatch should be performed each time it is run. This also
defaults to five.

This is an implementation of the paper "Summarizing visual data using
bidirectional similarity" by Simakov et al. from CVPR 2008.

Usage: ImageStack -load source.jpg -load target.jpg -bidirectional 0.5 -display
```