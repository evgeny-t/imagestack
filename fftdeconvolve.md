
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fftdeconvolve


-fftdeconvolve uses Fourier-space math to undo a convolution using the gaussian
image prior described in Levin et al. 2007. The convolution specified must be
2D. The arguments are the weight argument (the weight assigned to the gradients
being minimized), then the filter width, filter height, and filter frames, then
the filter to be deconvolved by in row major form. With only the first
argument, fftdeconvolve will use the next image on the stack as the filter. It
must be single channel.

Usage: ImageStack -load filter.png -load in.jpg -fftdeconvolve 0.01 -save
dcv.jpg

```