
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help ifft

-ifft performs an inverse dft on the current image, whose values are complex.
The input and output are images with 2*c channels, where channel 2*i is the
real part of the i'th channel, and channel 2*i+1 is the imaginary part of the
i'th channel.

Usage: ImageStack -load a.tga -fftcomplex -save freq.tga

```