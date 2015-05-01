
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help fprintf

-fprintf evaluates and prints its arguments, appending them to the file
specified by the first argument, using the second argument as a format string.
The remaining arguments are all evaluated as floats, so use %%d, %%i, and other
non-float formats with caution.

Usage: ImageStack -load foo.jpg -fprintf results.txt "Mean  =  %%f" "mean()"
```