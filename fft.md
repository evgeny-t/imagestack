
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fft

-fft performs a fast dft on the current image, whose values are interpreted as
complex. The input is an image with 2*c channels, where channel 2*i is the real
part of the i'th channel, and channel 2*i+1 is the imaginary part of the i'th
channel. The output image is laid out the same way.

Usage: ImageStack -load a.tmp -fftcomplex -save freq.tmp

```