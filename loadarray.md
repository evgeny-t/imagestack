
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help loadarray

-loadarray loads raw arrays of various data types. It takes 6 arguments. The
first is the filename to be loaded, the second is the data type, which must be
one of: int8, uint8, int16, uint16, int32, uint32, float32, float64, or
equivalently: char, unsigned char, short, unsigned short, int, unsigned int,
float, double. The last four arguments specify the dimensions in the order
width, height, frames, and channels. Bear in mind that ImageStack stores values
internally as 32 bits floats, so information will be lost when double arrays
are loaded. Integer formats are not scaled to lie between zero and one, this
must be done manually with the -scale operation.

Usage: ImageStack -loadarray foo.bar uint8 640 480 1 3

```