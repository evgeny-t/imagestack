
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help save


-save stores the image at the top of the stack to a file. The stack is not
altered. The following file formats are supported:


.tmp files. This format is used to save temporary image data, and to
interoperate with other programs that can load or save raw binary data. The
format supports any number of frames and channels. A .tmp file starts with a
header that containining five 32-bit integer values which represents frames,
width, height, channels and type. Image data follows.
types:
 0: 32 bit floats (the default format, which matches the internal format)
 1: 64 bit doubles
 2: 8 bit unsigned integers
 3: 8 bit signed integers
 4: 16 bit unsigned integers
 5: 16 bit signed integers
 6: 32 bit unsigned integers
 7: 32 bit signed integers
 8: 64 bit unsigned integers
 9: 64 bit signed integers

When saving, an optional second argument specifies the format. This may be any
of int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64,
or correspondingly char, unsigned char, short, unsigned short, int, unsigned
int, float, or double. The default is float32.

.hdr files. These always have three channels and one frame. They store data
in a 4 bytes per pixel red green blue exponent (rgbe) format.

.jpg (or .jpeg) files. When saving, an optional second arguments specifies
the quality. This defaults to 90. A jpeg image always has a single frame,
and may have either one or three channels.


.png files. These have a bit depth of 8, and may have 1-4 channels. They may
only have 1 frame.

.ppm files, of either 8 or 16 bit depth. When saving, an optional second
argument, which defaults to 8, specifies the bit depth. ppm files always
have three channels and one frame.

.tga files. These can have 1, 3, or 4 channels, are run-length encoded, and
are low dynamic range.

.wav sound files. They are represented as one or two channel images with
height and width of 1, but many frames.

.tiff (or .tif or .meg) files. When saving, an optional second argument
specifies the format. This may be any of int8, uint8, int16, uint16, int32,
uint32, float16, float32, float64, or correspondingly char, unsigned char,
short, unsigned short, int, unsigned int, half, float, or double. The default
is uint16.

.flo files. This format is used for optical flow evaluation. It stores 2-band
float image for horizontal and vertical flow components.

.csv files. These contain comma-separated floating point values in text. Each
scanline of the image corresponds to a line in the file. x and c are thus
conflated, as are y and t. When loading csv files, ImageStack assumes 1 channel
and 1 frame.

.pba files. This format is a human-readable space-separated 2D array of
numbers. It is used by petabricks. Width and channels become columns of the
file, and height and frames becomes rows.

Usage: ImageStack -load in.ppm -save out.jpg 98
       ImageStack -load in.ppm -save out.jpg
       ImageStack -load in.ppm -save out.ppm 16

```