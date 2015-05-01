
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help deconvolve

-deconvolution will deconvolve an image with the kernel in the stack.
 This operation takes the name of the deconvolution method as a single
 argument, plus any optional arguments that the method may require.
 Currently supported are "cho" (Cho and Lee, 2009), "shan" 
 (Shan et al, 2008), and "levin" - the simpler method with a Gaussian prior on
gradients describe in (Levin 2007). The method "levin" takes an additional
argument to specify the weight given to the prior.

Usage: ImageStack -load blurred -load kernel -deconvolve cho
```