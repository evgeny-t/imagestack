
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help savearray

-savearray saves raw arrays of various data types. It takes 2 arguments. The first
is the filename to be loaded, the second is the data type, which must be one of:
int8, uint8, int16, uint16, int32, uint32, float32, float64, or equivalently:
char, unsigned char, short, unsigned short, int, unsigned int, float, double.

Bear in mind that ImageStack stores values internally as 32 bit floats, so
saving in double format does not give you higher fidelity.

Integer type formats are not scaled from the range zero to one. This must be
done manually using the -scale operation.

Usage: ImageStack -load in.jpg -savearray out.float float32

```