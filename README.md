# __This repository is a clone of [code.google.com/p/imagestack](https://code.google.com/p/imagestack)__

# imagestack
_Automatically exported from code.google.com/p/imagestack_

## About ImageStack
ImageStack is a command-line stack calculator for images, developed in the Stanford Graphics Lab by Andrew Adams and other members of Marc Levoy's group. Many operations were contributed by students in CS448f.

ImageStack supports a wide variety of image processing operations, including recent techniques from the computational photography literature. Indeed, its key design goal is to make it easy to implement and experiment with new image processing techniques, in an adequately compute- and memory-efficient manner.

It is easily extensible with new operations, and is suitable for use on the command line, in scripts, or by linking to it as a library. ImageStack represents all images as four-dimensional arrays of floating point data, where the four dimensions are space, time, and color channel. This makes it suitable for processing bursts of images, high-dynamic range data, and more general scientific computing. In many respects, ImageStack is like matlab, but more specialized towards images, somewhat leaner, and entirely on the command line.

For a full listing of the operations supported by ImageStack, see the OperationIndex.

## Some Command-line examples:
Get a listing of all operations:

```
ImageStack -help
```

Get help on an operation (in this case, the scale operation):

```
ImageStack -help scale
```

Brighten an image:

```
ImageStack -load input.jpg -scale 1.2 -save output.jpg
```

Average two images and display the result:

```
ImageStack -load a.jpg -load b.jpg -add -scale 0.5 -display
```

Align and average two images related by a perspective warp:

```
ImageStack -load a.jpg -load b.jpg -align perspective -add -scale 0.5 -display
```

Resize an image to be twice as large. Note that all command line arguments are run through a parser, so you can say things like "width*2" whenever an operation wants a numeric argument. resample uses a 3-lobed lanczos filter, you can also do nearest neighbor upsampling and downsampling with upsample and downsample:

```
ImageStack -load in.jpg -resample width*2 height*2 -display
```

Load a bunch of images, align them, and then take the median at each pixel (by sorting and then doing a 3D crop):

```
ImageStack -loadframes *.jpg -alignframes perspective -sort t -crop 0 0 frames/2 width height 1 -display
```

Print out some statistics about an image, like mean, variance, etc.

```
ImageStack -load in.jpg -statistics
```

Divide every value in an image by the standard deviation in the corresponding channel. eval is a pretty useful operator for evaluating arbitrary expressions at every pixel. evalchannels is useful variant of it that evaluates a vector of expressions.

```
ImageStack -load in.jpg -eval "val/stddev(c)" -display
```

Repeatedly blur an image, displaying the result each time. Finally, save the result. Adding an extra hyphen acts as a nesting operator for higher-order expressions like loop.

```
ImageStack -load in.jpg -loop 100 --fastblur 2 --display -save out.jpg
```
