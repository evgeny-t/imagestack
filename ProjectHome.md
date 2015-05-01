## About ImageStack ##

ImageStack is a command-line stack calculator for images, developed in the [Stanford Graphics Lab](http://graphics.stanford.edu/) by [Andrew Adams](http://www.stanford.edu/~abadams) and other members of [Marc Levoy's](http://graphics.stanford.edu/~levoy) group. Many operations were contributed by students in [CS448f](http://www.stanford.edu/class/cs448f).

ImageStack supports a wide variety of image processing operations, including recent techniques from the computational photography literature. Indeed, its key design goal is to make it easy to implement and experiment with new image processing techniques, in an adequately compute- and memory-efficient manner.

It is easily extensible with new operations, and is suitable for use on the command line, in scripts, or by linking to it as a library. ImageStack represents all images as four-dimensional arrays of floating point data, where the four dimensions are space, time, and color channel. This makes it suitable for processing bursts of images, high-dynamic range data, and more general scientific computing. In many respects, ImageStack is like matlab, but more specialized towards images, somewhat leaner, and entirely on the command line.

For a full listing of the operations supported by ImageStack, see the OperationIndex.

## Some Command-line examples: ##

Get a listing of all operations:

ImageStack -[help](help.md)

Get help on an operation (in this case, the scale operation):

ImageStack -[help](help.md) [scale](scale.md)

Brighten an image:

ImageStack -[load](load.md) input.jpg -[scale](scale.md) 1.2 -[save](save.md) output.jpg

Average two images and display the result:

ImageStack -[load](load.md) a.jpg -[load](load.md) b.jpg -[add](add.md) -[scale](scale.md) 0.5 -[display](display.md)

Align and average two images related by a perspective warp:

ImageStack -[load](load.md) a.jpg -[load](load.md) b.jpg -[align](align.md) perspective -[add](add.md) -[scale](scale.md) 0.5 -[display](display.md)

Resize an image to be twice as large. Note that all command line arguments are run through a parser, so you can say things like "width\*2" whenever an operation wants a numeric argument. [resample](resample.md) uses a 3-lobed lanczos filter, you can also do nearest neighbor upsampling and downsampling with [upsample](upsample.md) and [downsample](downsample.md):

ImageStack -[load](load.md) in.jpg -[resample](resample.md) width\*2 height\*2 -[display](display.md)

Load a bunch of images, align them, and then take the median at each pixel (by sorting and then doing a 3D crop):

ImageStack -[loadframes](loadframes.md) `*`.jpg -[alignframes](alignframes.md) perspective -[sort](sort.md) t -[crop](crop.md) 0 0 frames/2 width height 1 -[display](display.md)

Print out some statistics about an image, like mean, variance, etc.

ImageStack -[load](load.md) in.jpg -[statistics](statistics.md)

Divide every value in an image by the standard deviation in the corresponding channel. [eval](eval.md) is a pretty useful operator for evaluating arbitrary expressions at every pixel. [evalchannels](evalchannels.md) is useful variant of it that evaluates a vector of expressions.

ImageStack -[load](load.md) in.jpg -[eval](eval.md) "val/stddev(c)" -[display](display.md)

Repeatedly blur an image, displaying the result each time. Finally, save the result. Adding an extra hyphen acts as a nesting operator for higher-order expressions like [loop](loop.md).

ImageStack -[load](load.md) in.jpg -[loop](loop.md) 100 --[fastblur](fastblur.md) 2 --[display](display.md) -[save](save.md) out.jpg