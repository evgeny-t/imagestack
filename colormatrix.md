
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help colormatrix


-colormatrix treats each pixel as a vector over its channels and multiplies
the vector by the given matrix. The matrix size and shape is deduced from the
number of arguments. The matrix is specified in column major order.

Converting rgb to grayscale:
  ImageStack -load color.tga -colormatrix 1 1 1 -scale 0.33333 -save gray.tga

Making orange noise:
  ImageStack -push 100 100 1 1 -noise -colormatrix 1 0.3 0 -save noise.tga

Making noise that varies between orange and blue:
  ImageStack -push 100 100 1 2 -noise -colormatrix 1 0.3 0 0 0 1 -save noise.tga

```