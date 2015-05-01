
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help gausstransform

-gausstransform evaluates a weighted sum of Gaussians at a discrete set of
positions. The positions are given by the top image on the stack, the locations
of the Gaussians are given by the second image, and the weights of the
Gaussians are given by the third. There are several ways to accelerate a Gauss
transform, therefore -gausstransform takes one argument to indicate the method
to use, and then one argument per channel in the second image in the stack to
indicate the standard deviation of the desired Gaussian in that dimension.
Methods available are: exact (slow!); grid (the bilateral grid of Paris et
al.); permutohedral (the permutohedral lattice of Adams et al.); and gkdtree
(the gaussian kdtree of Adams et al.). If only one argument is given, the
standard deviations used are all one. If two arguments are given, the standard
deviation is the same in each dimension.

Usage: ImageStack -load pics/dog1.jpg -evalchannels [0] [1] [2] 1 \
                  -dup -evalchannels x y [0] [1] [2] -dup \
                  -gausstransform permutohedral 4 4 0.1 0.1 0.1 \
                  -evalchannels [0]/[3] [1]/[3] [2]/[3] \
                  -save bilateral_filtered_dog.jpg
```