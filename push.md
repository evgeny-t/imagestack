
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help push

-push adds a new zeroed image to the top of the stack. With no arguments it
matches the dimensions of the current image. With 4 arguments (width, height,
frames, and channels) it creates an image of that size. Given three arguments
frames defaults to 1, and the arguments are taken as width, height, and
channels.

Usage: ImageStack -load a.tga -push -add -scale 0.5 -multiply -save out.tga
       ImageStack -push 1024 1024 1 3 -offset 0.5 -save gray.tga

```