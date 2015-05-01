
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help lahbpcg

-lahbpcg takes six images from the stack and treats them as a target output, x
gradient, and y gradient, and then the respective weights for each term. The
weights may be single-channel or have the same number of channels as the target
images. It then attempts to solve for the image which best achieves that target
ouput and those target gradients in the weighted-least-squares sense using a
preconditioned weighted least squares solver. This technique is useful for a
variety of problems with constraints expressed in the gradient domain,
including Poisson solves, making a sparse labelling dense, and other
gradient-domain techniques. The problem formulation is from Pravin Bhat's
"GradientShop", and the preconditioner is the Locally Adapted Hierarchical
Basis Preconditioner described by Richard Szeliski.

This operator takes two arguments. The first specifies the maximum number of
iterations, and the second specifies the error required for convergence

The following example takes a sparse labelling of an image im.jpg, and expands
it to be dense in a manner that respects the boundaries of the image. The
target image is the labelling, with weights indicating where it is defined. The
target gradients are zero, with weights inversely proportional to gradient
strength in the original image.
Usage: ImageStack -load sparse_labels.tmp \
                  -push -dup \
                  -load sparse_weights.tmp \
                  -load im.jpg -gradient x -colorconvert rgb gray \
                  -eval "1/(100*val^2+0.001)" \
                  -load im.jpg -gradient y -colorconvert rgb gray \
                  -eval "1/(100*val^2+0.001)" \
                  -lahbpcg 5 0.001 -save out.png
```